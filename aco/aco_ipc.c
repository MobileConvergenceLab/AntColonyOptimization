#include <arpa/inet.h>
#include <string.h>
#include "aco_ipc.h"

struct packed_request_hdr {
	uint32_t type;
	uint32_t paylen;
	uint8_t data[];
};

struct packed_response_hdr {
	uint32_t type;
	uint32_t paylen;
	uint32_t result;
	uint8_t data[];
};

void request_serial(void *buff, size_t *size, struct request_hdr *hdr)
{
	struct packed_request_hdr *packed_hdr = (struct packed_request_hdr*)buff;

	packed_hdr->type = htonl(hdr->type);
	packed_hdr->paylen = htonl(hdr->paylen);
	memcpy(packed_hdr->data, hdr->data, hdr->paylen);
}

void request_deserial(void *buff, size_t size, struct request_hdr *hdr)
{
	struct packed_request_hdr *packed_hdr = (struct packed_request_hdr*)buff;

	hdr->type = ntohl(packed_hdr->type);
	hdr->paylen = ntohl(packed_hdr->paylen);
	memcpy(hdr->data, packed_hdr->data, hdr->paylen);
}

