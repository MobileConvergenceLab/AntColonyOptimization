#ifndef FON_PACKET_H
#define FON_PACKET_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "fon_defs.h"
#include "fon_types.h"

FON_BEGIN_EXTERN_C

enum PACKET_TYPE {
    PACKET_TYPE_NONE    = 0,
    PACKET_TYPE_ADV     = 1,
    PACKET_TYPE_RSVD    = 127,
    PACKET_TYPE_MAX     = 0xFFFF
};

enum PACKET_ID {
    // Something is wrong
    PACKET_ID_INVALID   = -1,
    // For Loop-back
    PACKET_ID_LOOP      = 0,
    // NOT FOR Broadcast.
    PACKET_ID_ANY       = 0xFFFF,
};

typedef struct _packet_hdr_t {
    fon_id_t        sid;
    fon_id_t        did;
    fon_type_t      type;
    fon_len_t       paylen;
    uint8_t         data[];
} packet_hdr_t;

typedef struct _packet_buff_t {
    packet_hdr_t    hdr;
    /* 2KiB fit */
    uint8_t         data[0x0800 - sizeof(packet_hdr_t)];
} packet_buff_t;

static inline void
pkt_hdr_print(const packet_hdr_t *hdr)
{
    printf("sid:  %u\n"
           "did:  %u\n"
           "type: %d\n"
           "len:  %d\n",
           hdr->sid,
           hdr->did,
           hdr->type,
           hdr->paylen);
}

static inline void
pkt_hdr_check(packet_hdr_t *hdr)
{
    if(hdr->type > FON_TYPE_MAX)
    {
        abort();
    }

    if(hdr->paylen > FON_LEN_MAX)
    {
        abort();
    }
}

static inline void
pkt_hdr_set(packet_hdr_t *hdr, fon_id_t sid, fon_id_t did, fon_type_t type)
{
    // TODO
    // 각 필드의 범위를 체크해야 한다.
    hdr->sid    = sid;
    hdr->did    = did;
    hdr->type   = type;
    hdr->paylen = 0;
}

static inline void
pkt_payload_set(packet_hdr_t *hdr, int buflen, void *data, int datalen)
{
    if((size_t)buflen < sizeof(packet_hdr_t) + datalen)
    {
        exit(EXIT_FAILURE);
    }

    hdr->paylen = datalen;
    memcpy(hdr->data, data, datalen);
}

static inline void
pkt_cpy(packet_hdr_t *dest, int buflen, packet_hdr_t *src)
{
    assert( (size_t)buflen >= sizeof(packet_hdr_t) + src->paylen);
    memcpy(dest, src, buflen);
}

FON_END_EXTERN_C

#endif // FON_PACKET_H
