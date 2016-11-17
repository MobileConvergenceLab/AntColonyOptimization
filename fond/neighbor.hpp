#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#include <sys/socket.h>
#include <memory>
#include <map>
#include <set>
#include <fon/fon_types.h>
#include "types.hpp"
#include "idtable.hpp"

typedef struct _packet_hdr_t packet_hdr_t;

// reponsible for how to send a packet
class Neighbor {
public:
	Neighbor(IdTablePtr idtable, fon_id_t id, mac_addr_t addr);
	virtual ~Neighbor();
	virtual int op_sendto(const packet_hdr_t *hdr) = 0;
protected:
	IdTablePtr	m_idtable;
	const fon_id_t	m_id;
	mac_addr_t	m_addr;
};
typedef std::shared_ptr<Neighbor> NeighborPtr;

#endif /* NEIGHBOR_H */
