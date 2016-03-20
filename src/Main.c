/**
 * FON Daemon
 * 구현은 크게
 *  - IPC synchronous message 처리
 *  - IPC asynchronous message 처리(미구현)
 *  - Ether link
 *  - bluetooth link(미구현)
 * 로 이루어져 있음.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>
#include <glib-unix.h>

#include "fon/msg_if.h"
#include "fon/fon-utils.h"
#include "fon/msg_if.h"
#include "fon/msg_defs.h"
#include "fon/common-defs.h"
#include "Main.h"

#define LISTEN_MAX              (10)

static gboolean req_handler_reg(FonCoreObject *obj, int fd, const msg_req *req);
static gboolean req_handler_dereg(FonCoreObject *obj, int fd, const msg_req *req);
static gboolean req_handler_sendto(FonCoreObject *obj, int fd, const msg_req *req);
static gboolean req_handler_table_add(FonCoreObject *obj, int fd, const msg_req *req);
static gboolean req_handler_table_del(FonCoreObject *obj, int fd, const msg_req *req);
static gboolean req_handler_table_get(FonCoreObject *obj, int fd, const msg_req *req);
static gboolean req_handler_host_get(FonCoreObject *obj, int fd, const msg_req *req);

typedef gboolean (*req_handler_t)(FonCoreObject *obj, int fd, const msg_req *req);

static req_handler_t req_handlers[] = {
    (req_handler_t)req_handler_reg,
    (req_handler_t)req_handler_dereg,
    (req_handler_t)req_handler_sendto,
    (req_handler_t)req_handler_table_add,
    (req_handler_t)req_handler_table_del,
    (req_handler_t)req_handler_table_get,
    (req_handler_t)req_handler_host_get,
};

static gboolean req_handler_reg(FonCoreObject *obj, int fd, const msg_req *req) {
    ////dbg("Called");
    
    ClientInfo*         info = g_new(ClientInfo, 1);
    struct sockaddr_in* addr = &info->deliver_addr;

    info->sync = fd;

    addr->sin_family = AF_INET;
    addr->sin_port=htons(req->reg.deliver_port);
    if(inet_pton(AF_INET,"127.0.0.1",&(addr->sin_addr)) != 1)
    {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    cmanager_insert(obj->cmanager, req->hdr.func_type, info);

    msg_send_rsp_full(fd, TRUE, "Success", NULL, 0);

    ////dbg("Done");

    return TRUE;
}

static gboolean req_handler_dereg(FonCoreObject *obj, int fd, const msg_req *req) {
    ////dbg("Called");
    cmanager_remove(obj->cmanager, req->hdr.func_type);
    ////dbg("Done");
    return TRUE;
}

static gboolean req_handler_sendto(FonCoreObject *obj, int fd, const msg_req *req) {
    ////dbg("Called");

    static char err_str_fail[]    = "Fail: There is no did in Forward Table";
    static char err_str_succ[]    = "Succes";

    packet          pkt         = req->sendto.pkt;
    int             neigh_id    = -1;
    table_tuple     *tuple      = NULL;
    NearNode        *node       = NULL;
    gboolean        result      = FALSE;
    char            *err_str    = NULL;

    /* 어디로 포워딩 해야하는지 알아낸다. */
    table_get(obj->table, pkt.hdr.pkt_did, &tuple);
    if(tuple == NULL) {
        result  = FALSE;
        err_str = err_str_fail;
        goto RETURN;
    }
    neigh_id = tuple->neigh_id;

    node = node_pool_lookup(obj->pool, neigh_id);
    g_assert(node != NULL);

    /* 실제로 패킷을 보내는 가상함수를 호출한다. */
    node->op_sendto(node, &pkt);
    result  = TRUE;
    err_str = err_str_succ;

RETURN:
    ////dbg("Done");
    g_free(tuple);
    msg_send_rsp_full(fd, result, err_str, NULL, 0);
    return TRUE;
}

static gboolean req_handler_table_add(FonCoreObject *obj, int fd, const msg_req *req) {
    ////dbg("Called");
    table_tuple     *tuple      = NULL;
    gboolean        result      = FALSE;
    char            *err_str    = NULL;

    tuple   = g_new0(table_tuple, 1);
    *tuple  = req->table_add.tuple;

    table_add(obj->table, tuple);
    result = TRUE;
    err_str = strdup("Success");

    ////dbg("Done");
    msg_send_rsp_full(fd, result, err_str, NULL, 0);
    free(err_str);
    return TRUE;
}

static gboolean req_handler_table_del(FonCoreObject *obj, int fd, const msg_req *req) {
    ////dbg("Called");
    /* always success */
    table_del(obj->table, req->table_del.id);
    msg_send_rsp_full(fd, TRUE, "Success", NULL, 0);
    ////dbg("Done");
    return TRUE;
}

static gboolean req_handler_table_get(FonCoreObject *obj, int fd, const msg_req *req) {
    ////dbg("Called");

    size_t              len     = -1;

    GArray *arr = table_get_all(obj->table);

    len = sizeof(table_tuple) * (arr->len);

    msg_send_rsp_full(fd, TRUE, "Success",
                        &TUPLE_GARRAY_INDEX(arr, 0),
                        len);

    TUPLE_GARRAY_UNREF(arr);

    ////dbg("Done");
    return TRUE;
}

static gboolean req_handler_host_get(FonCoreObject *obj, int fd, const msg_req *req) {
    ////dbg("Called");

    size_t  len     = sizeof(msg_rsp_host_get) -sizeof(msg_rsp_hdr);
    int     host    = obj->host_id;
    msg_send_rsp_full(fd, TRUE, "Success",
                        &host,
                        len);

    ////dbg("Done");
    return TRUE;
}

/*GUnixFDSourceFunc */
static gboolean
client_callback(gint fd,
                GIOCondition condition,
                FonCoreObject *obj) {
    msg_req         req;

    if(!msg_recv_req(fd, &req)) {
        ////dbg("Session may be closed");
        // TODO
        g_source_destroy(g_main_current_source());
        cmanager_remove(obj->cmanager, req.hdr.func_type);
        close(fd);

        goto RETURN;
    }

    if(req.hdr.msg_type < 0 || req.hdr.msg_type >= MSG_TYPE_MAX) {
        /* Critical error(in client or library side). */
        g_assert_not_reached();
        exit(EXIT_FAILURE);
    }
    req_handlers[req.hdr.msg_type](obj, fd, &req);

RETURN:
    return TRUE;
}

/*GUnixFDSourceFunc */
static gboolean
accept_callback(gint fd,
                GIOCondition condition,
                FonCoreObject *obj) {

    ////dbg("Called");

    struct sockaddr_in  addr_from;
    socklen_t           len = sizeof(struct sockaddr_in);
    int                 client;
    int                 src_id;
    GSource             *client_source;

    client = accept(fd, (struct sockaddr*)&addr_from, &len);



    client_source = g_unix_fd_source_new(client, G_IO_IN);
    g_source_set_callback(  client_source,
                            (GSourceFunc)client_callback,
                            obj,
                            NULL);

    src_id = g_source_attach(client_source, obj->context);
    if(src_id < 0) {
        exit(EXIT_FAILURE);
    }
    /* from now, listen_source belong to context in loop */
    g_source_unref(client_source);

    ////dbg("Done");
    return TRUE;
}

static int
init_listen_sock(int port) {
    ////dbg("Called");
    int                 listen_sock = -1;
    struct sockaddr_in  addr        = {};
    socklen_t           len         = sizeof(struct sockaddr_in);
    int                 bf          = 1;

    listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &bf, (int)sizeof(bf)))
    {
        perror("setsockopt()");
        goto RETURN;
    }

    if(listen_sock == -1) {
        perror("socket()");
        goto RELEASE_SOCK;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port=htons(port);
    if(bind(listen_sock, (struct sockaddr*)&addr, len) == -1) {
        perror("bind()");
        goto RELEASE_SOCK;
    }

    if(listen(listen_sock, LISTEN_MAX) == -1) {
        perror("listen()");
        goto RELEASE_SOCK;
    }

    goto RETURN;

RELEASE_SOCK:
    close(listen_sock);
    listen_sock = -1;
RETURN:
    ////dbg("Done");
    return listen_sock;
}

static int
init_deliver_sock(int port) {
    ////dbg("Called");
    int                 deliver_sock = -1;
    struct sockaddr_in  addr        = {};
    socklen_t           len         = sizeof(struct sockaddr_in);
    int                 bf          = 1;

    deliver_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(deliver_sock == -1) {
        perror("socket()");
        goto RETURN;
    }

    if(setsockopt(deliver_sock, SOL_SOCKET, SO_REUSEADDR, &bf, (int)sizeof(bf)))
    {
        perror("setsockopt()");
        goto RELEASE_SOCK;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port=htons(port);
    if(bind(deliver_sock, (struct sockaddr*)&addr, len) == -1) {
        perror("bind()");
        goto RELEASE_SOCK;
    }

    goto RETURN;

RELEASE_SOCK:
    close(deliver_sock);
    deliver_sock = -1;
RETURN:
    ////dbg("Done");
    return deliver_sock;
}

static void fon_core_object_init(FonCoreObject* obj, int host_id, int port, int port2) {
    ////dbg("Called");
    g_assert(obj != NULL);

    GMainLoop           *loop;
    GMainContext        *context;
    int                 listen_sock;
    int                 deliver_sock;
    GSource             *listen_source;
    guint               src_id;

    context = g_main_context_new();
    loop    = g_main_loop_new(context, FALSE);
    /* from now, context belong to loop */
    g_main_context_unref(context);

    listen_sock = init_listen_sock(port);
    if(listen_sock < 0) {
        perror("init_listen_sock()");
    }

    deliver_sock = init_deliver_sock(port2);
    if(deliver_sock < 0) {
        perror("init_deliver_sock()");
    }

    listen_source = g_unix_fd_source_new(listen_sock, G_IO_IN);
    g_source_set_callback(  listen_source,
                            (GSourceFunc)accept_callback,
                            obj,
                            NULL);

    src_id = g_source_attach(listen_source, context);
    if(src_id < 0) {
        exit(EXIT_FAILURE);
    }
    /* from now, listen_source belong to context in loop */
    g_source_unref(listen_source);

    obj->listen_sock  = listen_sock;
    obj->deliver_sock = deliver_sock;
    obj->loop         = loop;
    obj->context      = context;
    obj->host_id      = host_id;
    obj->table        = table_create();
    obj->pool         = node_pool_create(obj->table);
    obj->cmanager     = cmanager_create();
    obj->ether_link   = ether_link_create(obj);

    g_assert(obj->table != NULL);
    g_assert(obj->pool != NULL);
    g_assert(obj->cmanager != NULL);
    g_assert(obj->ether_link != NULL);

    ////dbg("Done");
    return;
}

/* parse arguments and set variables */
static void opt(    int         argc,
                    char        **argv,
                    int         *arg_host_id,
                    int         *arg_port,
                    int         *arg_port2) {

    /* default value */
    int host_id = 0;
    int port    = FON_CORE_LISTEN_PORT;
    int port2   = FON_CORE_DELIVER_PORT;

    switch(argc) {
    case 4:
        port2 = atoi(argv[3]);
    case 3:
        port = atoi(argv[2]);
    case 2:
        host_id = atoi(argv[1]);
    case 1:
        *arg_port2      = port2;
        *arg_port       = port;
        *arg_host_id    = host_id;
        break;
    default:
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    FonCoreObject       obj;
    int                 arg_host_id;
    int                 arg_port;
    int                 arg_port2;

    opt(argc, argv, &arg_host_id, &arg_port, &arg_port2);

    fon_core_object_init(   &obj,
                            /* host id*/
                            arg_host_id,
                            /* listen port number */
                            arg_port,
                            arg_port2);

#ifndef NO_DAEMON
    if (daemon(0, 0) != 0) {
        g_error(PROGNAME ": Failed to daemonize.\n");
    }
#else
    g_print(PROGNAME
          ": Not daemonizing (built with NO_DAEMON-build define)\n");
#endif

    g_main_loop_run(obj.loop);

    return 0;
}

void delivery_to_client_function(FonCoreObject* obj,  packet *pkt)
{
    ////dbg("Called");

    g_assert(pkt != NULL);
    g_assert(obj != NULL);

    int         sendbyte    = -1;
    ClientInfo* info        = NULL;

    info = cmanager_lookup(obj->cmanager, pkt->hdr.pkt_type);
    if(info == NULL) {
        // Drop packet
        ////dbg("Not registerd pkt_type(client or FON Function). Drop packet");
        goto RETURN;
    }

    sendbyte = sendto(obj->deliver_sock, pkt, sizeof(packet), 0,
                      (struct sockaddr*)&info->deliver_addr,
                      sizeof(info->deliver_addr));
    if(sendbyte == -1) {
        // TODO
        perror("sendto()");
        exit(EXIT_FAILURE);
    }

RETURN:
    ////dbg("Done");
    return;
}



