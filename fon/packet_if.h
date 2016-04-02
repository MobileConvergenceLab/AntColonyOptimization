#ifndef PACKET_IF_H
#define PACKET_IF_H

#include <stdint.h>
#include <stdio.h>

enum PACKET_TYPE {
    PACKET_TYPE_NONE    = 0,
    PACKET_TYPE_ADV     = 1,
    PACKET_TYPE_RSVD    = 127,
    PACKET_TYPE_MAX     = 0xFFFF
};

enum PACKET_ID {
    PACKET_ID_INVALID   = -1,       /* Something is wrong */
    PACKET_ID_LOOP      = 0,
    PACKET_ID_ANY       = 0xFFFF,   /* 브로드 캐스팅이 아님. */
    PACKET_ID_MAX       = 0xFFFF
};

typedef struct _packet_hdr {
    int             pkt_sid;
    int             pkt_did;
    int             pkt_len;
    int             pkt_type;
    char            pkt_data[];
} packet_hdr;

typedef struct _packet_adv {
    packet_hdr  hdr;
} packet_adv;

typedef union _packet {
    packet_hdr  hdr;
    packet_adv  adv;
    char        padd[0x0800];       /* 2KiB */
} packet;

static inline void
pkt_hdr_print(const packet *pkt)
{
    printf("pkt_sid:  %d\n"
           "pkt_did:  %d\n"
           "pkt_type: %d\n"
           "pkt_len:  %d\n",
           pkt->hdr.pkt_sid,
           pkt->hdr.pkt_did,
           pkt->hdr.pkt_type,
           pkt->hdr.pkt_len);
}

static inline void
pkt_hdr_set(packet* pkt, int sid, int did, int len, int type)
{
    // TODO
    // 각 필드의 범위를 체크해야 한다.
    pkt->hdr.pkt_sid = sid;
    pkt->hdr.pkt_did = did;
    pkt->hdr.pkt_len = len;
    pkt->hdr.pkt_type = type;
}

#endif /* PACKET_IF_H */
