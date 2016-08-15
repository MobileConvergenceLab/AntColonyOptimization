#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <glib.h>
#include "packet.hpp"

/* ONLY USED INTERNAL */
typedef uint16_t __pkt_id_t;
typedef uint16_t __paylen_t;
typedef uint16_t __pkt_type_t;

/* ONLY USED INTERNAL */
#define PACKED_PKT_BUF_LEN          (0x0800)  /* 2KB */
#define PACKED_PKT_HDR_LEN          (12)
#define PACKED_PKT_PAD_LEN          (PACKED_PKT_BUF_LEN - PACKED_PKT_HDR_LEN)

/* ONLY USED INTERNAL */
typedef struct __attribute__ ((packed)) _packed_pkt {
    fon_id_packed_t       packed_sid;
    fon_id_packed_t       packed_did;
    fon_len_packed_t      packed_len;
    fon_type_packed_t     packed_type;
    uint8_t               data[];
} __packed_pkt;

void pkt_pack(const  packet_hdr_t *hdr, void *buf, int *buflen)
{
    int buf_paylen = PACKED_PKT_HDR_LEN + hdr->paylen;

    if(*buflen < buf_paylen)
    {
        /* not enough memory */
        *buflen = -1;
        return;
    }

    __packed_pkt *p = (__packed_pkt *)buf;

    p->packed_sid   = FON_ID_PACK(hdr->sid);
    p->packed_did   = FON_ID_PACK(hdr->did);
    p->packed_len   = FON_LEN_PACK(hdr->paylen);
    p->packed_type  = FON_TYPE_PACK(hdr->type);
    memcpy(p->data, hdr->data, hdr->paylen);

    *buflen = buf_paylen;
    return;
}

void pkt_unpack(packet_hdr_t *hdr, void *buf, int buflen)
{
    __packed_pkt   *p          = (__packed_pkt *)buf;

    hdr->sid    = FON_ID_UNPACK(p->packed_sid);
    hdr->did    = FON_ID_UNPACK(p->packed_did);
    hdr->paylen = FON_LEN_UNPACK(p->packed_len);
    hdr->type   = FON_TYPE_UNPACK(p->packed_type);
    memcpy(hdr->data, p->data, hdr->paylen);

    return;
}


