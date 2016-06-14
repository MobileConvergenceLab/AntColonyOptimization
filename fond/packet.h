/**
 * Implementation of (FON) packet
 */
#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include "fon/packet_if.h"

void        pkt_pack        (packet *in_pkt, void *out_buf, int *buflen);
void        pkt_unpack      (packet *out_pkt, void *in_buf, int buflen);
#endif /* PACKET_H */
