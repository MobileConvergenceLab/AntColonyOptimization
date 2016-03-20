#ifndef CMANAGER_IF_H
#define CMANAGER_IF_H

#include <glib.h>
#include <string.h>
#include <stddef.h>

struct _msg_req_hdr {
    int         func_type;
    int         msg_type;
    int         tot_len;
    char        data[];
};

struct _msg_rsp_hdr {
    #define MSG_RSP_ERRSTR_LEN      (64)
    gboolean    result;
    int         tot_len;
    char        error_str[MSG_RSP_ERRSTR_LEN];     /* 널문자 포함 */
    char        data[];
};

typedef struct _msg_req_hdr         msg_req_hdr;
typedef struct _msg_rsp_hdr         msg_rsp_hdr;
typedef union  _msg_req             msg_req;
typedef union  _msg_rsp             msg_rsp;
typedef union  _msg_asyn            msg_asyn;

void msg_req_hdr_print(msg_req_hdr *hdr);
void msg_rsp_hdr_print(msg_rsp_hdr *hdr);

/* client side */
gboolean
msg_send_req(           int                 fd,
                        msg_req             *in_req);

gboolean
msg_recv_rsp(           int                 fd,
                        msg_rsp             *out_rsp);

/* serve side */
gboolean
msg_recv_req(           int                 fd,
                        msg_req             *out_req);
gboolean
msg_send_rsp_full(      int                 fd,
                        gboolean            result,
                        const char          *error_str,
                        void                *rv,
                        int                 rv_len);

#endif /* CMANAGER_IF_H */

