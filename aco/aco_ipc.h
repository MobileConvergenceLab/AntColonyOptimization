#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include "aco-types.h"

enum message_type {
	message_type_none = 0,
	message_type_find = 1,
};

struct request_hdr {
	uint32_t type;
	uint32_t paylen;
	uint8_t data[];
};

struct response_hdr {
	uint32_t type;
	uint32_t paylen;
	uint32_t result;
	uint8_t data[];
};

struct find_reqeust {
	struct request_hdr hdr;
	aco_id_packed_t target;
	aco_cycle_packed_t ncycle;
};

struct find_response {
	struct response_hdr hdr;
};

void request_serial(void *buff, size_t *size, struct request_hdr *hdr);
void request_deserial(void *buff, size_t size, struct request_hdr *hdr);

// not implemented
void response_serial(uint8_t *buff, size_t *size, struct response_hdr *hdr);
void response_deserial(uint8_t *buff, size_t size, struct response_hdr *hdr);

#endif /* CLIENT_H */
