#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <glib.h>
#include "packet.h"

/* ONLY USED INTERNAL */
typedef uint16_t __pkt_id_t;
typedef uint16_t __pkt_len_t;
typedef uint16_t __pkt_type_t;

/* ONLY USED INTERNAL */
#define PACKED_PKT_BUF_LEN          (0x0800)  /* 2KB */
#define PACKED_PKT_HDR_LEN          (12)
#define PACKED_PKT_PAD_LEN          (PACKED_PKT_BUF_LEN - PACKED_PKT_HDR_LEN)

/* ONLY USED INTERNAL */
typedef struct __attribute__ ((packed)) _packed_pkt {
    __pkt_id_t            packed_sid;
    __pkt_id_t            packed_did;
    __pkt_len_t           packed_len;
    __pkt_type_t          packed_type;
    uint8_t               data[];
} __packed_pkt;

void pkt_pack(packet *pkt, void *buf, int *buflen)
{
    int buf_tot_len = PACKED_PKT_HDR_LEN + pkt->hdr.pkt_len;

    if(*buflen < buf_tot_len)
    {
        /* not enough memory */
        *buflen = -1;
        return;
    }

    __packed_pkt *p = (__packed_pkt *)buf;

    p->packed_sid = htons(pkt->hdr.pkt_sid);
    p->packed_did = htons(pkt->hdr.pkt_did);
    p->packed_len = htons(pkt->hdr.pkt_len);
    p->packed_type = htons(pkt->hdr.pkt_type);
    memcpy(p->data, pkt->hdr.pkt_data, pkt->hdr.pkt_len);

    *buflen = buf_tot_len;
    return;
}

void pkt_unpack(packet *pkt, void *buf, int buflen)
{
    __packed_pkt   *p          = (__packed_pkt *)buf;

    pkt->hdr.pkt_sid = ntohs(p->packed_sid);
    pkt->hdr.pkt_did = ntohs(p->packed_did);
    pkt->hdr.pkt_len = ntohs(p->packed_len);
    pkt->hdr.pkt_type = ntohs(p->packed_type);
    memcpy(pkt->hdr.pkt_data, p->data, pkt->hdr.pkt_len);

    return;
}


