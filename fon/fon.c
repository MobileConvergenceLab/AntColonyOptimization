/* library for FON Client */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <assert.h>

#include "array.h"
#include "hexdump.h"
#include "fon.h"
#include "fon_dbg.h"

static bool
__sync_connect(int *out_fd, int in_port) {
    struct sockaddr_in  addr        = {};
    socklen_t           len         = sizeof(struct sockaddr_in);

    *out_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*out_fd < 0) {
        perror("socket()");
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port=htons(in_port);
    inet_pton(AF_INET,"127.0.0.1",&(addr.sin_addr));

    if(connect(*out_fd, (struct sockaddr*)&addr, len) == -1) {
        perror("connect()");
        return false;
    }

    return true;
}

static bool
__async_open(int *out_fd, int *out_port) {
    struct sockaddr_in  addr        = {};
    socklen_t           len         = sizeof(struct sockaddr_in);

    *out_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*out_fd < 0) {
        perror("socket()");
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port=htons(0);
    if(inet_pton(AF_INET,"127.0.0.1",&(addr.sin_addr)) != 1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    if(bind(*out_fd, (struct sockaddr*)&addr, len) == -1){
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    if(getsockname(*out_fd, (struct sockaddr*)&addr, &len)) {
        perror("getsockname()");
        exit(EXIT_FAILURE);
    }
    *out_port = ntohs(addr.sin_port);

    return true;
}


static bool
__reg(int sync_sock, fon_type_t func_type, int async_port)
{
    msg_rsp_reg_t     rsp;
    msg_req_reg_t     req;

    req.hdr.func_type   = func_type;
    req.hdr.msg_type    = MSG_TYPE_REG;
    req.hdr.tot_len     = sizeof(msg_req_reg_t);
    req.async_port      = async_port;

    msg_req_send(sync_sock, &req.hdr);
    msg_rsp_recv(sync_sock, &rsp.hdr, sizeof(msg_rsp_reg_t));

    return MSG_RESULT_BOOL(rsp.hdr.result);
}

static void
__init_client(FonClient    *client,
              fon_type_t   in_type,
              int          in_port)
{
    //TODO: type 범위 체크!

    if(in_port == 0)
    {
        in_port = FON_IPC_LISTEN_PORT;
    }

    client->func_type       = in_type;
    client->sync_port       = in_port;
}


FonClient*
__fon_client_malloc0_try()
{
    FonClient *client = malloc(sizeof(FonClient));
    if(client == NULL)
    {
        exit(-1);
    }
    
    client->sync_sock       = -1;
    client->sync_port       = -1;
    client->async_sock      = -1;
    client->async_port      = -1;
    client->func_type       = -1;

    return client;
}

static bool    
__connect_client(FonClient    *client)
{
    int                 sync_sock       = -1;
    int                 async_sock      = -1;
    int                 async_port      = -1;
    bool                ret;
    __attribute__((unused))
    char                *err_str        = NULL;

    if(!__sync_connect(&sync_sock, client->sync_port)) {
        err_str = "__sync_connect(), Please Check fon-daemon server";
        ret = false;
        goto ERROR;
    }

    if(!__async_open(&async_sock, &async_port)) {
        err_str = "__async_open(), Please Check fon-daemon server";
        ret = false;
        goto ERROR;
    }

    if(!__reg(sync_sock, client->func_type, async_port)) {
        err_str = "__reg()";
        ret = false;
        goto ERROR;
    }

    client->sync_sock       = sync_sock;
    client->async_sock      = async_sock;
    client->async_port      = async_port;

    ret = true;
    goto RETURN;

ERROR:
//    //dbg("Error(%s)", err_str);
    close(sync_sock);
    close(async_sock);

RETURN:
    return ret;
}


/*==============================================================================
 *
 *==============================================================================*/
FonClient*
fon_client_new(fon_type_t in_type ,int in_port)
{
    //dbg("Called");
    FonClient* client = __fon_client_malloc0_try();

    __init_client(client, in_type, in_port);

    if(!__connect_client(client)) {
        goto ERROR;
    }

    goto RETURN;

ERROR:
    //dbg("Error");
    free(client);
    client = NULL;
RETURN:
    //dbg("Done");
    return client;
}

bool
fon_sendto(FonClient *client, packet_hdr_t *hdr)
{
    //dbg("Called");
    if(hdr->type != client->func_type) {
        printf("Failed\n");
        return false;
    }

    msg_req_buff_t              req;
    msg_rsp_sendto_t            rsp;
    int                         pkt_tot_len = sizeof(packet_hdr_t) + hdr->paylen;

    req.hdr.func_type   = client->func_type;
    req.hdr.msg_type    = MSG_TYPE_SENDTO;
    req.hdr.tot_len     = sizeof(msg_req_sendto_t) + pkt_tot_len;

    // Assume that packet_buff_t's size is enough.
    pkt_cpy(&req.sendto.pkt,
            sizeof(msg_req_buff_t) - sizeof(msg_req_hdr_t),
            hdr);

    msg_req_send(client->sync_sock, &req.hdr);
    msg_rsp_recv(client->sync_sock, &rsp.hdr, sizeof(msg_rsp_sendto_t));

    //dbg("Done");
    return MSG_RESULT_BOOL(rsp.hdr.result);
}

int
fon_recvfrom(FonClient      *client,
             packet_hdr_t   *hdr,
             int            buflen)
{
    //dbg("Called");
    int                 recvbyte;

    recvbyte = recvfrom(client->async_sock,
                        hdr,
                        // see fon_core.c:delivery_to_client()
                        sizeof(packet_buff_t),
                        MSG_WAITALL,
                        0,
                        0);

    assert(hdr->type == client->func_type);

    //dbg("Done");
    return recvbyte;
}


bool
fon_table_add(FonClient *client, fib_tuple_t   *tuple) {
    msg_rsp_buff_t  rsp;
    msg_req_buff_t  req;

    req.hdr.func_type   = client->func_type;
    req.hdr.msg_type    = MSG_TYPE_TABLE_ADD;
    req.hdr.tot_len     = sizeof(msg_req_table_add_t);
    req.table_add.tuple = *tuple;

    msg_req_send(client->sync_sock, &req.hdr);
    msg_rsp_recv(client->sync_sock, &rsp.hdr, sizeof(msg_rsp_buff_t));

    return MSG_RESULT_BOOL(rsp.hdr.result);
}

bool
fon_table_del(FonClient *client, fon_id_t id) {
    msg_rsp_buff_t      rsp;
    msg_req_buff_t      req;

    req.hdr.func_type   = client->func_type;
    req.hdr.msg_type    = MSG_TYPE_TABLE_DEL;
    req.hdr.tot_len     = sizeof(msg_req_table_del_t);
    req.table_del.id    = id;

    msg_req_send(client->sync_sock, &req.hdr);
    msg_rsp_recv(client->sync_sock, &rsp.hdr, sizeof(msg_rsp_buff_t));

    return MSG_RESULT_BOOL(rsp.hdr.result);
}

bool
fon_table_get(FonClient *client, Array **tuple_array)
{
    msg_req_buff_t  req     = {{0,}};
    msg_rsp_buff_t  rsp     = {{0,}};
    // number of tuples
    int             len     = -1;
    // Array of tuples
    Array           *arr    = NULL;

    req.hdr.func_type   = client->func_type;
    req.hdr.msg_type    = MSG_TYPE_TABLE_GET;
    req.hdr.tot_len     = sizeof(msg_req_table_get_t);

    msg_req_send(client->sync_sock, &req.hdr);
    msg_rsp_recv(client->sync_sock, &rsp.hdr, sizeof(msg_rsp_buff_t));

    len = MSG_RSP_GET_TUPLE_LEN(&rsp);

    if(MSG_RESULT_BOOL(rsp.hdr.result)) {
        arr = array_new(len, sizeof(fib_tuple_t  ), true);
        array_append_vals(arr, rsp.table_get.tuple, len);
        *tuple_array = arr;
    }

    return MSG_RESULT_BOOL(rsp.hdr.result);
}

bool
fon_host_get(FonClient *client, fon_id_t *out_id)
{
    msg_rsp_buff_t  rsp = {{0,}};
    msg_req_buff_t  req = {{0,}};

    req.hdr.func_type   = client->func_type;
    req.hdr.msg_type    = MSG_TYPE_HOST_GET;
    req.hdr.tot_len     = sizeof(msg_req_host_get_t);

    msg_req_send(client->sync_sock,
                 &req.hdr);

    msg_rsp_recv(client->sync_sock,
                 &rsp.hdr,
                 sizeof(msg_rsp_buff_t));

    if(MSG_RESULT_BOOL(rsp.hdr.result)) {
        *out_id = rsp.host_get.host_id;
    }

    return MSG_RESULT_BOOL(rsp.hdr.result);
}

fon_type_t
fon_get_type(FonClient *client)
{
    return client->func_type;
}

