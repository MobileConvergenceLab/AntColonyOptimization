#ifndef MSG_DEFS_H
#define MSG_DEFS_H

#include "msg_if.h"
#include "packet_if.h"
#include "table_if.h"

enum MSG_TYPE {
    /* syncronus messages */
    MSG_TYPE_REG        = 0,        /* 채널 등록 */
    MSG_TYPE_DEREG,
    MSG_TYPE_SENDTO,
    MSG_TYPE_TABLE_ADD,
    MSG_TYPE_TABLE_DEL,
    MSG_TYPE_TABLE_GET,
    MSG_TYPE_HOST_GET,
    MSG_TYPE_MAX
};

const static char MSG_TYPE_STR[][64] = {
    "MSG_TYPE_REG",
    "MSG_TYPE_DEREG",
    "MSG_TYPE_SENDTO",
    "MSG_TYPE_TABLE_ADD",
    "MSG_TYPE_TABLE_DEL",
    "MSG_TYPE_TABLE_GET",
    "MSG_TYPE_HOST_GET",
    "MSG_TYPE_MAX"
};

/*==============================================================================
 * request messages
 *==============================================================================*/
struct _msg_req_reg {
    struct _msg_req_hdr hdr;
    int                 deliver_port; // See cmanager.h
};

struct _msg_req_sendto {
    struct _msg_req_hdr hdr;
    packet              pkt;
};

struct _msg_req_table_add {
    struct _msg_req_hdr hdr;
    table_tuple         tuple;
};

struct _msg_req_table_del {
    struct _msg_req_hdr hdr;
    int                 id;
};

typedef struct _msg_req_reg         msg_req_reg;
typedef struct _msg_req_hdr         msg_req_dereg;
typedef struct _msg_req_sendto      msg_req_sendto;
typedef struct _msg_req_table_add   msg_req_table_add;
typedef struct _msg_req_table_del   msg_req_table_del;
typedef struct _msg_req_hdr         msg_req_table_get;
typedef struct _msg_req_hdr         msg_req_host_get;

typedef union _msg_req {
    msg_req_hdr         hdr;
    msg_req_reg         reg;
    msg_req_dereg       dereg;
    msg_req_sendto      sendto;
    msg_req_table_add   table_add;
    msg_req_table_del   table_del;
    msg_req_table_get   table_get;
    msg_req_host_get    host_get;
} msg_req;

/*==============================================================================
 * response messages
 *==============================================================================*/
struct _msg_rsp_table_get {
    struct _msg_rsp_hdr hdr;
    table_tuple         tuple[];
};

struct _msg_rsp_host_get {
    struct _msg_rsp_hdr hdr;
    int                 host_id;
};

/* return number of tuples */
#define MSG_RSP_GET_TUPLE_LEN(msg)        \
                    (((msg)->hdr.tot_len-sizeof(msg_rsp_table_get)) / sizeof(table_tuple))

typedef struct _msg_rsp_hdr         msg_rsp_reg;
typedef struct _msg_rsp_hdr         msg_rsp_dereg;
typedef struct _msg_rsp_hdr         msg_rsp_sendto;
typedef struct _msg_rsp_hdr         msg_rsp_table_add;
typedef struct _msg_rsp_hdr         msg_rsp_table_del;
typedef struct _msg_rsp_table_get   msg_rsp_table_get;
typedef struct _msg_rsp_host_get    msg_rsp_host_get;

typedef union _msg_rsp {
    msg_rsp_hdr         hdr;
    msg_rsp_reg         reg;
    msg_rsp_dereg       dereg;
    msg_rsp_sendto      sendto;
    msg_rsp_table_add   table_add;
    msg_rsp_table_del   table_del;
    msg_rsp_table_get   table_get;
    msg_rsp_host_get    host_get;

    char padd[1024*4];  /* big enough? */
} msg_rsp;

#endif /* MSG_DEFS_H */
