#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>

#include "msg_if.h"
#include "msg_defs.h"
#include "fon-utils.h"

#define BIG_BUF_LEN     (0x0800)    /* assume: big enough */

/* hexdum - from: http://stackoverflow.com/questions/7775991/
 *                how-to-get-hexdump-of-a-structure-data
 */
static void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;


    // Output description if given.
    if (desc != NULL)
        g_print ("%s:\n", desc);

    if(len == 0)
        return;

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                g_print ("  %s\n", buff);

            // Output the offset.
            g_print ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        g_print (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        g_print ("   ");
        i++;
    }

    // And print the final ASCII bit.
    g_print ("  %s\n", buff);
}

void msg_req_print(msg_req_hdr *hdr) {
    //g_print("func_type:  %d\n", msg->func_type);
    g_print("msg_type:   %s(%d)\n", MSG_TYPE_STR[hdr->msg_type], hdr->msg_type);
    g_print("tot_len:   %d\n", hdr->tot_len);
    hexDump("data", hdr->data, hdr->tot_len);
}

void msg_rsp_print(msg_rsp_hdr *hdr) {
    g_print("result:    %d\n", hdr->result);
    g_print("tot_len:    %d\n", hdr->tot_len);
    g_print("error_str: %s\n", hdr->error_str);
    hexDump("data", hdr->data, hdr->tot_len);
}

/* client side */
gboolean
msg_send_req(           int                 fd,
                        msg_req          *in_req) {
    int         sendbyte;

    sendbyte = send(fd, in_req, in_req->hdr.tot_len, 0);
    return (sendbyte != -1);
}

gboolean
msg_recv_rsp(           int                 fd,
                        msg_rsp         *out_rsp)
{
    int             recvbyte;
    msg_rsp         rsp;

    recvbyte = recv(fd, &rsp, sizeof(msg_rsp_hdr), MSG_WAITALL | MSG_PEEK);
    if(recvbyte != sizeof(msg_rsp_hdr)) {
        return FALSE;
    }

    recvbyte = recv(fd, &rsp, rsp.hdr.tot_len, MSG_WAITALL);
    if(recvbyte != rsp.hdr.tot_len) {
        return FALSE;
    }
    else {
        *out_rsp = rsp;
        return TRUE;
    }
}

/* sedatae side */
gboolean
msg_recv_req(
        int         fd,
        msg_req     *out_msg)
{
    msg_req     req;
    int         recvbyte;

    recvbyte = recv(fd, &req, sizeof(msg_req_hdr), MSG_WAITALL|MSG_PEEK);
    if(recvbyte != sizeof(msg_req_hdr)) {
        return FALSE;
    }

    recvbyte = recv(fd, &req, req.hdr.tot_len, MSG_WAITALL);
    if(recvbyte != req.hdr.tot_len) {
        return FALSE;
    }

    *out_msg = req;

    return TRUE;
}

static gboolean
msg_send_rsp(
        int                 fd,
        msg_rsp         *in_msg)
{
    int         sendbyte;
    sendbyte = send(fd, in_msg, in_msg->hdr.tot_len, 0);
    return (sendbyte != -1);
}

gboolean msg_send_rsp_full(
        int fd, gboolean result,
        const char *error_str, void *data, int data_len)
{
    msg_rsp     rsp;
    gboolean    ret;

    rsp.hdr.result     = result;
    rsp.hdr.tot_len    = sizeof(msg_rsp_hdr)+data_len;

    /* buffer overflow */
    g_assert(rsp.hdr.tot_len < sizeof(msg_rsp) );

    strncpy(rsp.hdr.error_str, error_str, MSG_RSP_ERRSTR_LEN);
    rsp.hdr.error_str[MSG_RSP_ERRSTR_LEN-1] = '\0';

    memcpy(rsp.hdr.data, data, data_len);

    if(!msg_send_rsp(fd, &rsp)) {
        perror("msg_send_rsp()");
        ret = FALSE;
    }
    else {
        ret = TRUE;
    }

    return ret;
}


