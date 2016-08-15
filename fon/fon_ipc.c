#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <assert.h>
#include <stdio.h>

#include "fon_dbg.h"
#include "hexdump.h"
#include "fon_ipc.h"

#define BIG_BUF_LEN     (0x0800)    /* assume: big enough */

#define MSG_HEXDUMP     (0)

void msg_req_print(msg_req_hdr_t *hdr)
{
    printf("msg_type:   %s(%d)\n", MSG_TYPE_STR[hdr->msg_type], hdr->msg_type);
    printf("tot_len:   %d\n", hdr->tot_len);
    hexDump("data", hdr->data, hdr->tot_len);
} // msg_req_print()

void
msg_req_cpy(msg_req_hdr_t  *dest,
            int            buflen,
            msg_req_hdr_t  *src)
{
    assert(buflen >= src->tot_len);
    memcpy(dest, src, src->tot_len);
} // msg_req_cpy()

bool
msg_req_send(int fd,
             msg_req_hdr_t *hdr)
{
    int         sendbyte;

    sendbyte = send(fd, hdr, hdr->tot_len, 0);

#if MSG_HEXDUMP
    hexDump("Dump(req<send>)", hdr, hdr->tot_len);
#endif
    return (sendbyte != -1);
} // msg_req_send()


bool
msg_req_recv(int fd,
             msg_req_hdr_t  *hdr,
             int            buflen)
{
    msg_req_buff_t  buff;
    int             tot_len;
    int             recvbyte;
    bool            ret;

    recvbyte = recv(fd, &buff, sizeof(msg_req_hdr_t), MSG_WAITALL|MSG_PEEK);
    if(recvbyte != sizeof(msg_req_hdr_t)) {
        ret = false;
        goto RETURN;
    }

    tot_len = buff.hdr.tot_len;
    recvbyte = recv(fd, &buff, tot_len, MSG_WAITALL);
    if(recvbyte != tot_len) {
        ret = false;
        goto RETURN;
    }

    if(buflen >= tot_len)
    {
        memcpy(hdr, &buff.hdr, tot_len);
        ret = true;
        goto RETURN;
    }
    else
    {
        ret = false;
        goto RETURN;
    }

RETURN:
#if MSG_HEXDUMP
    hexDump("Dump(req<recv>)", hdr, hdr->tot_len);
#endif
    return ret;
} // msg_req_recv()

void msg_rsp_print(msg_rsp_hdr_t *hdr)
{
    printf("result   : %s(%d)\n", MSG_RESULT_STR[hdr->result], hdr->result);
    printf("tot_len  : %d\n", hdr->tot_len);
    printf("error_str: %s\n", hdr->error_str);
    hexDump("data", hdr->data, hdr->tot_len);
} // msg_rsp_print

void
msg_rsp_cpy(msg_rsp_hdr_t  *dest,
            int            buflen,
            msg_rsp_hdr_t  *src)
{
    assert(buflen >= src->tot_len);
    memcpy(dest, src, src->tot_len);
} // msg_rsp_cpy()

bool
msg_rsp_send(int fd,
             msg_rsp_hdr_t *hdr)
{
    int         sendbyte;

    sendbyte = send(fd,
                    hdr,
                    hdr->tot_len,
                    0);

#if MSG_HEXDUMP
    hexDump("Dump(rsp<send>)", hdr, hdr->tot_len);
#endif
    return (sendbyte != -1);
} // msg_rsp_send()

void
msg_rsp_fill(msg_rsp_hdr_t *hdr,
             int buflen,
             msg_result_t result,
             const char *error_str,
             void *data,
             int data_len)
{
    hdr->result     = result;
    hdr->tot_len    = sizeof(msg_rsp_hdr_t) + data_len;

    /* buffer overflow */
    assert(hdr->tot_len <= buflen);

    strncpy(hdr->error_str, error_str, MSG_RSP_ERRSTR_LEN);
    hdr->error_str[MSG_RSP_ERRSTR_LEN-1] = '\0';

    memcpy(hdr->data, data, data_len);

    return;
} // msg_rsp_fill()

bool
msg_rsp_recv(int fd,
             msg_rsp_hdr_t *hdr,
             int buflen)
{
    int             recvbyte;
    int             tot_len;
    msg_rsp_buff_t  buff;
    bool            ret;

    recvbyte = recv(fd, &buff, sizeof(msg_rsp_hdr_t), MSG_WAITALL | MSG_PEEK);
    if(recvbyte != sizeof(msg_rsp_hdr_t)) {
        ret = false;
        goto RETURN;
    }

    tot_len = buff.hdr.tot_len;
    recvbyte = recv(fd, &buff, tot_len, MSG_WAITALL);
    if(recvbyte != tot_len) {
        ret = false;
        goto RETURN;
    }

    if(buflen >= tot_len)
    {
        memcpy(hdr, &buff, tot_len);
        ret = true;
        goto RETURN;
    }
    else
    {
        ret = false;
        goto RETURN;
    }

RETURN:
#if MSG_HEXDUMP
    hexDump("Dump(rsp<recv>)", hdr, hdr->tot_len);
#endif
    return ret;
} // msg_rsp_recv

