/**
 * 1) 수신 이벤트 핸들러 정의 및 수신 이벤트 등록
 * 2) 송신 이벤트 핸들러 정의
 */

/**
 * written by Sim Young-Bo
 * 인터페이스가 동적으로 변화하지 않는다고 가정.(plug and play 지원 안함)
 *
 * 패킷 수신시 fon_recvfrom() 호출
 * ad 송/수신 및 노드 추가/ 테이블 추가
 */

/* system dependent library */
#include <net/if_arp.h>
#include <net/ethernet.h>    /* the L2 protocols */
#include <netinet/if_ether.h>    /* for struct sockaddr_ll */
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>

/* POSIX library */
#include <pthread.h>
#include <unistd.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>        // For socket(), struct sockaddr, struct sockaddr_storage.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <glib.h>
#include <glib-unix.h>
#include "fon/fon-utils.h"
#include "packet.h"
#include "node.h"
#include "Main.h"       /* global data and global functions */

/** 
 * ethertype of FON. local byte-order. use ntohs()/htons()
 * according to IEEE Spec, 0101-01FF are reservced for Experimental.
 * 왜인지는 모르겠는데 0101-01FF 사이의 범위로 지정하니 동작하지 않음.
 * 그래서 동작가능한 임의의 다른 값으로 설정하였음 
 */
#define ETH_P_FON       (0xFF03)

#define BUFF_SZ         (0x0800)

/* the number of maximum physical interfaces */
#define IF_LIST_MAX         (24)
#define AD_PERIOD_IN_SEC    (1)

/*==============================================================================
 *
 *=============================================================================*/
/* inherite from NearNode */
typedef struct _NearNodeEther {
    NearNode            parent;
    ether_link          *r_link;
    struct sockaddr_ll  m_addr;
} NearNodeEther;
#define near_node_ether_new()   g_new0(NearNodeEther, 1)

/* local interface address list */
typedef struct _if_addr_list {
    int n;
    struct sockaddr_ll if_addr[IF_LIST_MAX]; /* interface socket address */
} if_addr_list;


struct _ether_link {
    FonCoreObject   *r_obj;
    int             *r_host_id;
    NodePool        *r_pool;
    int             m_fd;
    if_addr_list    m_if_addr_list;

    //TODO
    /*
    GMainLoop       *m_loop;
    GThread         *m_thread;
    */
};

/*==============================================================================
 *
 *=============================================================================*/
//static gboolean     is_loopback(if_addr_list *list, struct sockaddr_ll *sa_ll);
static int          ether_node_op_sendto(NearNodeEther *node, packet *pkt);
static void         sall_set_boradcast(struct sockaddr_ll *sa_ll);

static gboolean     init_ether_link_fd(ether_link* link);
static gboolean     init_ether_link_if_list(if_addr_list *list);

static gboolean     attach_sendto_advertising_source(ether_link* link);
static gboolean     callback_advertising_source(ether_link *link);

static gboolean     attach_read_source(ether_link* link);
static gboolean     callback_read_source(gint fd, GIOCondition condition, gpointer unused);
static void         callback_read_source_handler_ad_type(ether_link *link, packet *pkt, struct sockaddr_ll *addr);
static void         callback_read_source_handler_default(ether_link *link, packet* pkt);

/*==============================================================================
 *
 *=============================================================================*/
/*static gboolean is_loopback(if_addr_list *list, struct sockaddr_ll *sa_ll) {*/
/*    int         i           = 0;*/
/*    socklen_t   len         = sa_ll->sll_halen;*/

/*    for(i=0; i< list->n; i++) {*/
/*        if(!memcmp(list->if_addr[i].sll_addr, sa_ll->sll_addr , len))*/
/*            return TRUE;*/
/*    }*/

/*    return FALSE;*/
/*}*/

static gboolean init_ether_link_fd(ether_link* link) {
    int broadcastEnable = 1;
    int err             = -1;

    link->m_fd = socket(AF_PACKET, SOCK_DGRAM ,htons(ETH_P_FON));
    if(link->m_fd < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    err = setsockopt(link->m_fd,
                SOL_SOCKET,
                SO_BROADCAST,
                &broadcastEnable,
                sizeof(broadcastEnable));

    if(err < 0) {
        perror("setsockopt()");
        exit(EXIT_FAILURE);
        //close(link->m_fd);
    }

    return TRUE;
}

static gboolean init_ether_link_if_list(if_addr_list *list) {
    struct ifaddrs          *ifaddr;
    struct ifaddrs          *ifa;
    struct sockaddr_ll      *sa_ll;
    int                     family;
    int                     n;  /* loop var */

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        //exit(EXIT_FAILURE);
        return FALSE;
    }
    /* Walk through linked list, maintaining head pointer so we
       can free list later */

    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        family = ifa->ifa_addr->sa_family;

        /* Display interface name and family (including symbolic
           form of the latter for the common families) */

        if(family != AF_PACKET) {
            continue;
        }

        sa_ll = (struct sockaddr_ll *)ifa->ifa_addr;
        
        if(sa_ll->sll_hatype == ARPHRD_LOOPBACK) {
            continue;
        }

        memcpy(&list->if_addr[list->n],
                sa_ll,
                sizeof(struct sockaddr_ll));
        list->n++;

        if(list->n == IF_LIST_MAX) {
            break;
        }
    }

    freeifaddrs(ifaddr);

    return TRUE;
}

/* node_op_sendto */
static int ether_node_op_sendto(NearNodeEther *node, packet *pkt)
{
    char        buf[BUFF_SZ];
    int         buflen = BUFF_SZ;
    socklen_t   addrlen = sizeof(struct sockaddr_ll);

    pkt_pack(pkt, buf, &buflen);
    if(buflen < 0) {
        return -1;
    }

    g_assert(buflen < 1400);

    return sendto(  node->r_link->m_fd,
                    buf, buflen, 0,
                    (struct sockaddr*)&node->m_addr, addrlen);
}

/**
 * 1) Recv and unpack(de-mashalling)
 * 2) Loopback packet check
 * 3) call handler dependent on pkt_type. See [fon/packet_if.h]
 */
/* GUnixFDSourceFunc is a GSourceFunc*/
static gboolean
callback_read_source(gint         fd,
           GIOCondition condition,
           gpointer     user_data)
{
    ether_link          *link = (ether_link*)user_data;
    uint8_t             buf[BUFF_SZ];
    int                 recvbyte;
    packet              *pkt;
    struct sockaddr_ll  addr;
    socklen_t           addrlen = sizeof(struct sockaddr_ll);

    g_assert(condition == G_IO_IN);
    g_assert(fd == link->m_fd);

    recvbyte = recvfrom(link->m_fd,
                buf,
                BUFF_SZ,
                0, /* flags */
                (struct sockaddr*)(&addr),
                &addrlen);

    if(recvbyte < 0) {
        return FALSE;
    }

    pkt = g_new0(packet, 1);
    pkt_unpack(pkt, buf, recvbyte);

    /* loopback check */
    if(pkt->hdr.pkt_sid == *link->r_host_id) {
        g_free(pkt);
        //g_assert(is_loopback(&link->m_if_addr_list, &addr));
        return TRUE;
    }

    switch(pkt->hdr.pkt_type) {
    case PACKET_TYPE_ADV:
        callback_read_source_handler_ad_type(link, pkt, &addr);
        break;
    default:
        callback_read_source_handler_default(link, pkt);
        break;
    }

    return TRUE;
}

/* GSourceFunc compatible(casting possible) */
static gboolean callback_advertising_source(ether_link *link)
{
    int                 i;
    int                 sendbyte;
    struct sockaddr_ll  sa_ll;
    socklen_t           addrlen;
    packet              pkt;
    uint8_t             buf[BUFF_SZ];
    int                 buflen;
    int                 host_id = *link->r_host_id;

    sall_set_boradcast(&sa_ll);
    buflen = BUFF_SZ;
    addrlen = sizeof(struct sockaddr_ll);

    pkt_hdr_set(&pkt,
                host_id,                /* sid */
                PACKET_ID_ANY,          /* did */
                0,                      /* len */
                PACKET_TYPE_ADV);        /* type */

    pkt_pack(&pkt, buf, &buflen);
    g_assert(buflen != 0);

    for(i=0; i< link->m_if_addr_list.n; i++)
    {
        sa_ll.sll_ifindex = link->m_if_addr_list.if_addr[i].sll_ifindex;
        sendbyte = sendto(link->m_fd,
                       buf,
                       buflen,
                       0, /* flags */
                       (struct sockaddr*)&sa_ll,
                       addrlen);
        if(sendbyte < 0) {
        }
    }
    return TRUE;
}

static gboolean
attach_read_source(ether_link *link) {
    GSource         *in_source  = NULL;
    GMainContext    *context    = g_main_loop_get_context(link->r_obj->loop);
    int             src_id;

    in_source = g_unix_fd_source_new(link->m_fd, G_IO_IN);
    if(in_source == NULL) {
        return FALSE;
    }

    g_source_set_callback (in_source,
                     (GSourceFunc) callback_read_source,
                     link,
                     NULL);
    src_id = g_source_attach(in_source, context);
    g_source_unref(in_source);
    g_assert(src_id > 0);

    return TRUE;
}

static gboolean
attach_sendto_advertising_source(ether_link *link)
{
    GSource         *out_source = NULL;
    GMainContext    *context    = g_main_loop_get_context(link->r_obj->loop);
    int             src_id;

    out_source = g_timeout_source_new_seconds (AD_PERIOD_IN_SEC);
    g_assert(out_source != NULL);
    if(out_source == NULL)
        return FALSE;

    g_source_set_callback(out_source, (GSourceFunc)callback_advertising_source, link, NULL);
    src_id = g_source_attach(out_source, context);
    g_source_unref(out_source);
    g_assert(src_id > 0);

    return TRUE;
}

static void callback_read_source_handler_ad_type(ether_link *link, packet *pkt, struct sockaddr_ll *addr) {
    int             id;
    NearNodeEther   *node;

    /* 노드 추가 */
    id = pkt->hdr.pkt_sid;
    node = near_node_ether_new();
    node->parent.id         = id;
    node->parent.op_sendto  = (node_op_sendto)ether_node_op_sendto;
    node->r_link            = link;
    node->m_addr            = *addr;

    node_pool_add(link->r_pool, (NearNode*)node);

    g_free(pkt);

    return;
}

static void callback_read_source_handler_default(ether_link *link, packet* pkt) {
    delivery_to_client_function(link->r_obj, pkt);
    g_free(pkt);
}

static void sall_set_boradcast(struct sockaddr_ll *sa_ll)
{
    sa_ll->sll_family    = AF_PACKET;
    sa_ll->sll_protocol  = htons(ETH_P_FON);
    sa_ll->sll_ifindex   = 0;
    sa_ll->sll_hatype    = ARPHRD_ETHER;
    sa_ll->sll_pkttype   = PACKET_BROADCAST;
    sa_ll->sll_halen     = ETH_ALEN;
    memset(sa_ll->sll_addr, 0xFF, 8);

    return;
}


/*==============================================================================
 *
 *=============================================================================*/
ether_link* ether_link_create(FonCoreObject *obj) {

    ether_link *link;
    link = (ether_link *)g_try_malloc0(sizeof(ether_link));

    link->r_obj     = obj;
    link->r_host_id = &obj->host_id;
    link->r_pool    = obj->pool;

    if(!init_ether_link_fd(link)) {
        perror("init_ether_link_fd()");
        g_assert(0);
        return FALSE;
    }
    
    if(!init_ether_link_if_list(&link->m_if_addr_list)) {
        perror("init_ether_link_if_list()");
        g_assert(0);
        return FALSE;
    }
    
    if(!attach_read_source(link)) {
        g_assert(0);
        return FALSE;
    }
    
    if(!attach_sendto_advertising_source(link)) {
        g_assert(0);
        return FALSE;
    }

    return link;
}

