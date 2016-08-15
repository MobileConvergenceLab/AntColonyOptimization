#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <list>
#include <memory>
#include <fon/fon_packet.h>

// ex) FF:FF:FF:FF:FF:FF
typedef std::string mac_addr_t;
const mac_addr_t MAC_ADDR_WRONG("Wrong MAC Address");

// ex) 10.0.0.1
typedef std::string ip_addr_t;
const ip_addr_t IP_ADDR_WRONG("Wrong IP Address");

// ex) eth0
typedef std::string if_name_t;
const if_name_t IF_NAME_WRONG("Wrong Interface Name");

// list of external(physical) iterface names.
typedef std::list<if_name_t> if_list_t;

typedef std::string br_name_t;
const br_name_t BR_NAME_WRONG("Wrong Bridge Name");

typedef std::shared_ptr<packet_hdr_t> packet_hdr_shared;

inline packet_hdr_shared make_packet_hdr_shared(size_t sz)
{
	return packet_hdr_shared((packet_hdr_t*)new char[sz]);
}

#endif
