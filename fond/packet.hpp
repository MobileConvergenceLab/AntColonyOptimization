/**
 * Implementation of (FON) packet
 */
#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#include <fon/fon_packet.h>

void        pkt_pack        (const packet_hdr_t *in_pkt, void *out_buf, int *buflen);
void        pkt_unpack      (packet_hdr_t *out_pkt, void *in_buf, int buflen);

#endif /* PACKET_H */
