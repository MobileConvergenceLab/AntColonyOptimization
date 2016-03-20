/* library for FON Client */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#include <glib.h>
#include <glib-unix.h>

#include "fon.h"
#include "msg_if.h"
#include "msg_defs.h"
#include "fon-utils.h"
#include "common-defs.h"

#define LISTEN_MAX      (10)

typedef struct _CallbackFuncs {
    FonCallbackRecv     callback_recv;
} CallbackFuncs;

typedef struct _CallbackArgs {
    gpointer            recv_arg;
} CallbackArgs;


/**
 *  @context: asynchronous message를 처리하기 위한 context
 *  @sync_sock: daemon server에 대한 소켓
 *  @sync_port:
 *  @fucn_type:
 *  @callback: asynchronous message를 처리하는
 *           사용자 정의 함수포인터를 담고 있는 구조체
 */
typedef struct _FonLib {
    GMainContext        *context;
    gboolean            initiated;
    int                 sync_sock;
    int                 sync_port;
    int                 deliver_sock;
    int                 deliver_port;
    int                 fucn_type;
    CallbackFuncs       callbacks;
    CallbackArgs        args;
    
} FonLib;

static FonLib __fon_lib = {
    .context        = NULL,
    .initiated      = FALSE,
    .sync_sock      = -1,
    .sync_port      = -1,
    .deliver_sock   = -1,
    .deliver_port   = -1,
    .fucn_type      = -1,
    .callbacks      = {0,},
};

static gboolean __init_callbacks(CallbackFuncs *funcs) {
    memset(funcs, 0, sizeof(CallbackFuncs));

    return TRUE;
}

static gboolean __connect(int *out_fd, int in_port) {
    struct sockaddr_in  addr        = {};
    socklen_t           len         = sizeof(struct sockaddr_in);

    *out_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*out_fd < 0) {
        perror("socket()");
        return FALSE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port=htons(in_port);
    inet_pton(AF_INET,"127.0.0.1",&(addr.sin_addr));

    if(connect(*out_fd, (struct sockaddr*)&addr, len) == -1) {
        perror("connect()");
        return FALSE;
    }

    return TRUE;
}

static gboolean __deliver_open(int *out_fd, int *out_port) {
    struct sockaddr_in  addr        = {};
    socklen_t           len         = sizeof(struct sockaddr_in);

    *out_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*out_fd < 0) {
        perror("socket()");
        return FALSE;
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

    return TRUE;
}


static gboolean __reg(int sync_sock, int func_type, int deliver_port)
{
    msg_rsp     rsp;
    msg_req     req;

    req.hdr.func_type   = func_type;
    req.hdr.msg_type    = MSG_TYPE_REG;
    req.hdr.tot_len     = sizeof(msg_req_reg);
    req.reg.deliver_port = deliver_port;

    msg_send_req(sync_sock, &req);
    msg_recv_rsp(sync_sock, &rsp);

    return rsp.hdr.result;
}

static gboolean __async_msg_handler(gint            fd,
                                    GIOCondition    condition,
                                    gpointer        user_data) {

    int                 recvbyte;
    packet              pkt;
    struct sockaddr_in  addr;
    socklen_t           len = sizeof(struct sockaddr_in);

    memset(&pkt, 0, sizeof(packet));
    recvbyte = recvfrom(__fon_lib.deliver_sock,
                        &pkt,
                        sizeof(packet),
                        MSG_WAITALL,
                        (struct sockaddr*)&addr,
                        &len);

    if(recvbyte != sizeof(packet)) {
        /* 데몬과의 통신이 끊어짐 */
        return FALSE;
    }

    g_assert(pkt.hdr.pkt_type == __fon_lib.fucn_type);

    if(__fon_lib.callbacks.callback_recv != NULL) {
        __fon_lib.callbacks.callback_recv(&pkt, __fon_lib.args.recv_arg);
    }

    return TRUE;
}

static gboolean __attach_async_msg_handler_source() {
    GSource         *in_source  = NULL;
    int             src_id;

    in_source = g_unix_fd_source_new(__fon_lib.deliver_sock, G_IO_IN);
    if(in_source == NULL) {
        perror("g_unix_fd_source_new()");
        exit(EXIT_FAILURE);
    }

    g_source_set_callback (in_source,
                     (GSourceFunc) __async_msg_handler,
                     NULL,
                     NULL);
    src_id = g_source_attach(in_source, __fon_lib.context);
    g_source_unref(in_source);
    if(src_id < 0) {
        return FALSE;
    }
    return TRUE;
}

static gboolean __init_obj(FonLib *obj, GMainContext *context,
                            int in_type ,int in_port)
{
    int                 sync_sock       = -1;
    int                 sync_port       = -1;
    int                 deliver_sock    = -1;
    int                 deliver_port    = -1;
    int                 fucn_type       = -1;       /* FON Func type */
    CallbackFuncs       callbacks;

    if(in_port != 0) {
        sync_port = in_port;
    }
    else {
        sync_port = FON_CORE_LISTEN_PORT;
    }

    //TODO: type 범위 체크!
    fucn_type = in_type;

    if(!__connect(&sync_sock, sync_port)) {
        return FALSE;
    }

    if(!__deliver_open(&deliver_sock, &deliver_port)) {
        return FALSE;
    }

    if(!__reg(sync_sock, fucn_type, deliver_port)) {
        return FALSE;
    }

    if(!__init_callbacks(&callbacks)) {
        return FALSE;
    }

    obj->context   = context;
    g_main_context_ref(obj->context);

    obj->sync_sock      = sync_sock;
    obj->sync_port      = sync_port;
    obj->deliver_sock   = deliver_sock;
    obj->deliver_port   = deliver_port;
    obj->fucn_type      = fucn_type;
    obj->callbacks      = callbacks;
    obj->initiated      = TRUE;

    return TRUE;
}


/*==============================================================================
 *
 *==============================================================================*/
gboolean fon_init(GMainContext *context, int in_type ,int in_port) {

    if(__fon_lib.initiated) {
        return TRUE;
    }

    if(!__init_obj(&__fon_lib, context, in_type, in_port)) {
        return FALSE;
    }

    if(!__attach_async_msg_handler_source()){
        return FALSE;
    }

    return TRUE;
}

gboolean fon_sendto(packet *pkt) {

    if(pkt->hdr.pkt_type != __fon_lib.fucn_type) {
        return FALSE;
    }

    msg_rsp             rsp;
    msg_req             req;

    req.hdr.func_type   = __fon_lib.fucn_type;
    req.hdr.msg_type    = MSG_TYPE_SENDTO;
    req.hdr.tot_len     = sizeof(msg_req_sendto);
    req.sendto.pkt      = *pkt;

    msg_send_req(__fon_lib.sync_sock, &req);
    msg_recv_rsp(__fon_lib.sync_sock, &rsp);

    return rsp.hdr.result;
}

gboolean fon_table_add(table_tuple *tuple) {
    msg_rsp             rsp;
    msg_req             req;

    req.hdr.func_type   = __fon_lib.fucn_type;
    req.hdr.msg_type    = MSG_TYPE_TABLE_ADD;
    req.hdr.tot_len     = sizeof(msg_req_table_add);
    req.table_add.tuple = *tuple;

    msg_send_req(__fon_lib.sync_sock, &req);
    msg_recv_rsp(__fon_lib.sync_sock, &rsp);

    return rsp.hdr.result;
}

gboolean fon_table_del(int id) {
    msg_rsp             rsp;
    msg_req             req;

    req.hdr.func_type   = __fon_lib.fucn_type;
    req.hdr.msg_type    = MSG_TYPE_TABLE_DEL;
    req.hdr.tot_len     = sizeof(msg_req_table_del);
    req.table_del.id    = id;

    msg_send_req(__fon_lib.sync_sock, &req);
    msg_recv_rsp(__fon_lib.sync_sock, &rsp);

    return rsp.hdr.result;
}

gboolean fon_table_get(GArray **tuple_array)
{
    msg_rsp         rsp;
    msg_req         req;
    GArray     *arr;

    req.hdr.func_type   = __fon_lib.fucn_type;
    req.hdr.msg_type    = MSG_TYPE_TABLE_GET;
    req.hdr.tot_len     = sizeof(msg_req_table_get);

    msg_send_req(__fon_lib.sync_sock, &req);
    msg_recv_rsp(__fon_lib.sync_sock, &rsp);

    if(rsp.hdr.result) {
        arr = TUPLE_GARRAY_NEW();
        g_array_append_vals(arr, rsp.table_get.tuple, MSG_RSP_GET_TUPLE_LEN(&rsp));
        *tuple_array = arr;
    }

    return rsp.hdr.result;
}

gboolean fon_host_get(int *out_id)
{
    msg_rsp         rsp;
    msg_req         req;

    req.hdr.func_type   = __fon_lib.fucn_type;
    req.hdr.msg_type    = MSG_TYPE_HOST_GET;
    req.hdr.tot_len     = sizeof(msg_req_host_get);

    msg_send_req(__fon_lib.sync_sock, &req);
    msg_recv_rsp(__fon_lib.sync_sock, &rsp);

    if(rsp.hdr.result) {
        *out_id = rsp.host_get.host_id;
    }

    return rsp.hdr.result;
}

int fon_get_type()
{
    return __fon_lib.fucn_type;
}

void fon_set_callback_recv(FonCallbackRecv callback_recv, gpointer user_data)
{
    __fon_lib.callbacks.callback_recv = callback_recv;
    __fon_lib.args.recv_arg = user_data;

    return;
}


