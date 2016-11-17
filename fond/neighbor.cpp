#include "dbg.hpp"
#include "neighbor.hpp"

//==============================================================================
// class Neighbor
//==============================================================================
Neighbor::Neighbor(IdTablePtr idtable, fon_id_t id, mac_addr_t addr)
	:m_idtable(idtable),
	m_id(id),
	m_addr(addr)
{
	DBG_LOGGER;
	m_idtable->add_neighbor(m_id, m_addr, this);
}

Neighbor::~Neighbor()
{
	DBG_LOGGER;
	m_idtable->del_nehgibor(m_id);
}


