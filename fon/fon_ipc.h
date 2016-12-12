#ifndef FON_IPC_H
#define FON_IPC_H

#include "fon_types.h"
#include "fon_ipc_defs.h"
#include "fon_ipc_msgs.h"

#ifdef __cplusplus
extern "C" {
#endif

void msg_req_print      (msg_req_hdr_t  *in);
void msg_req_cpy        (msg_req_hdr_t  *dest,
                         int            buflen,
                         msg_req_hdr_t  *src);
bool msg_req_send       (int            fd,
                         msg_req_hdr_t  *in);
bool msg_req_recv       (int            fd,
                         msg_req_hdr_t  *out,
                         int            buflen);
void msg_rsp_print      (msg_rsp_hdr_t  *in);
void msg_rsp_cpy        (msg_rsp_hdr_t  *dest,
                         int            buflen,
                         msg_rsp_hdr_t  *src);
void msg_rsp_fill       (msg_rsp_hdr_t  *hdr,
                         int buflen,
                         msg_result_t result,
                         const char *error_str,
                         void *data,
                         int data_len);
bool msg_rsp_send       (int            fd,
                         msg_rsp_hdr_t  *in);
bool msg_rsp_recv       (int            fd,
                         msg_rsp_hdr_t  *out,
                         int            buflen);

#ifdef __cplusplus
}
#endif

#endif // FON_IPC_H
