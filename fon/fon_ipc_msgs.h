#ifndef FON_IPC_MSGS_H
#define FON_IPC_MSGS_H

#include <string.h>
#include <stddef.h>

#include "fon_packet.h"
#include "fon_fib_if.h"

typedef int msg_type_t;
enum MSG_TYPE {
    /* syncronus messages */
    MSG_TYPE_REG        = 0,        /* 채널 등록 */
    MSG_TYPE_DEREG      = 1,
    MSG_TYPE_SENDTO     = 2,
    MSG_TYPE_TABLE_ADD  = 3,
    MSG_TYPE_TABLE_DEL  = 4,
    MSG_TYPE_TABLE_GET  = 5,
    MSG_TYPE_HOST_GET   = 6,
    MSG_TYPE_MAX
};

static const char MSG_TYPE_STR[][64] = {
    "MSG_TYPE_REG",
    "MSG_TYPE_DEREG",
    "MSG_TYPE_SENDTO",
    "MSG_TYPE_TABLE_ADD",
    "MSG_TYPE_TABLE_DEL",
    "MSG_TYPE_TABLE_GET",
    "MSG_TYPE_HOST_GET",
    "MSG_TYPE_MAX"
};

typedef int msg_result_t;

enum MSG_RESULT {
    MSG_RESULT_FAILURE  = 0,
    MSG_RESULT_SUCCESS  = 1,
};

static const char MSG_RESULT_STR[][64] = {
    "MSG_RESULT_FAILURE",
    "MSG_RESULT_SUCCESS",
};

#define MSG_RESULT_BOOL(MSG_RESULT)     (MSG_RESULT)

/*==============================================================================
 * request messages
 *==============================================================================*/
typedef struct _msg_req_hdr_t {
    fon_type_t  func_type;
    msg_type_t  msg_type;
    int         tot_len;
    char        data[];
} msg_req_hdr_t;


typedef struct _msg_req_reg_t {
    msg_req_hdr_t       hdr;
    int                 async_port; // See cmanager.h
} msg_req_reg_t;

typedef struct _msg_req_dereg_t {
    msg_req_hdr_t       hdr;
} msg_req_dereg_t;

typedef struct _msg_req_sendto_t {
    msg_req_hdr_t       hdr;
    packet_hdr_t        pkt;
} msg_req_sendto_t;

typedef struct _msg_req_table_add_t {
    msg_req_hdr_t       hdr;
    fib_tuple_t         tuple;
} msg_req_table_add_t;

typedef struct _msg_req_table_del_t {
    msg_req_hdr_t       hdr;
    fon_id_t            id;
} msg_req_table_del_t;

typedef struct _msg_req_table_get_t {
    msg_req_hdr_t       hdr;
} msg_req_table_get_t;

typedef struct _msg_req_host_get_t {
    msg_req_hdr_t       hdr;
} msg_req_host_get_t;

typedef union _msg_req_buff_t {
    msg_req_hdr_t       hdr;
    msg_req_reg_t       reg;
    msg_req_dereg_t     dereg;
    msg_req_sendto_t    sendto;
    msg_req_table_add_t table_add;
    msg_req_table_del_t table_del;
    msg_req_table_get_t table_get;
    msg_req_host_get_t  host_get;

    // payload
    char                data[0x0800];
} msg_req_buff_t;

/*==============================================================================
 * response messages
 *==============================================================================*/
typedef struct _msg_rsp_hdr_t {
    #define MSG_RSP_ERRSTR_LEN      (64)
    msg_type_t  result;
    int         tot_len;
    char        error_str[MSG_RSP_ERRSTR_LEN];     /* 널문자 포함 */
    char        data[];
} msg_rsp_hdr_t;

typedef struct _msg_rsp_reg_t {
    msg_rsp_hdr_t       hdr;
} msg_rsp_reg_t;

typedef struct _msg_rsp_dereg_t {
    msg_rsp_hdr_t       hdr;
} msg_rsp_dereg_t;

typedef struct _msg_rsp_sendto_t {
    msg_rsp_hdr_t       hdr;
} msg_rsp_sendto_t;

typedef struct _msg_rsp_table_add_t {
    msg_rsp_hdr_t       hdr;
} msg_rsp_table_add_t;

typedef struct _msg_rsp_table_del_t {
    msg_rsp_hdr_t       hdr;
} msg_rsp_table_del_t;

typedef struct _msg_rsp_table_get_t {
    msg_rsp_hdr_t       hdr;
    fib_tuple_t         tuple[];

/* return number of tuples */
#define MSG_RSP_GET_TUPLE_LEN(msg)        \
                    (((msg)->hdr.tot_len-sizeof(msg_rsp_table_get_t)) / sizeof(fib_tuple_t  ))
} msg_rsp_table_get_t;

typedef struct _msg_rsp_host_get_t {
    msg_rsp_hdr_t       hdr;
    fon_id_t            host_id;
} msg_rsp_host_get_t;

typedef union _msg_rsp_buff_t {
    msg_rsp_hdr_t       hdr;
    msg_rsp_reg_t       reg;
    msg_rsp_dereg_t     dereg;
    msg_rsp_sendto_t    sendto;
    msg_rsp_table_add_t table_add;
    msg_rsp_table_del_t table_del;
    msg_rsp_table_get_t table_get;
    msg_rsp_host_get_t  host_get;

    // payload
    char                data[0x0800];
} msg_rsp_buff_t;

#endif // FON_IPC_MSGS_H

