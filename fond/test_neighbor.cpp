#include "dbg.hpp"
#include "neighbor.hpp"

class TestNeighbor: public Neighbor {
public:
	TestNeighbor(IdTablePtr idtable, fon_id_t id, mac_addr_t addr)
		:Neighbor(idtable, id, addr)
	{
		DBG_LOGGER;
	}

	~TestNeighbor()
	{
		DBG_LOGGER;
	}

	int op_sendto(const packet_hdr_t *hdr) override
	{
		return -1;
	}
};

int main()
{
	fon_id_t	host(1);
	fon_id_t 	id(2);
	IdTablePtr 	idtable(new IdTable(host));
	mac_addr_t 	addr("ff:ff:ff:ff:ff:ff");
	NeighborPtr	neigh(new TestNeighbor(idtable, id, addr));

	if(addr != idtable->get_ifaddr(id))
	{
		exit(EXIT_FAILURE);
	}

	if(id != idtable->get_id(addr))
	{
		exit(EXIT_FAILURE);
	}

	std::cerr << "Test was done successfully\n";

	return 0;
}
