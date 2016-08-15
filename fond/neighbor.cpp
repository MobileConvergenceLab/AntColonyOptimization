#include "dbg.hpp"
#include "neighbor.hpp"

//==============================================================================
// class IdTableObserver
//==============================================================================
IdTableObserver::IdTableObserver(IdTablePtr idtable)
:m_idtable(idtable)
{
	DBG_LOGGER;

	if(!m_idtable->reg_observer(this))
	{
		throw "IdTableObserver() throw exception";
	}
}

IdTableObserver::~IdTableObserver()
{
	DBG_LOGGER;
	m_idtable->dereg_observer(this);
}

IdTablePtr IdTableObserver::get_idtable()
{
	return m_idtable;
}


//==============================================================================
// class IdTable
//==============================================================================
IdTable::IdTable(fon_id_t host)
:m_host(host)
{
	DBG_LOGGER;
}

IdTable::~IdTable()
{
	DBG_LOGGER;
}

bool IdTable::reg_observer(IdTableObserver *ob)
{
	DBG_LOGGER;
	DBG_PRINT_VALUE(ob);

	auto iter = m_obs.find(ob);

	if(iter == m_obs.end())
	{
		m_obs.insert(ob);
		return true;
	}
	else
	{
		return false;
	}
}

void IdTable::dereg_observer(IdTableObserver *ob)
{
	auto iter = m_obs.find(ob);

	if(iter != m_obs.end())
	{
		m_obs.erase(*iter);
	}
	else
	{
		// do nothing
	}
}

void IdTable::add_neighbor(fon_id_t id, mac_addr_t addr, Neighbor *neighbor)
{

	auto ifaddr_iter = m_index_ifaddr.find(id);

	DBG_LOGGER;
	#if 0
	DBG_PRINT_VALUE(id);
	DBG_PRINT_VALUE(addr);
	DBG_PRINT_VALUE(neighbor);
	DBG_PRINT_VALUE(iter != m_index_ifaddr.end());
	#endif

	if(ifaddr_iter == m_index_ifaddr.end())
	{
		m_index_ifaddr[id] = addr;
		m_index_neighbor[id] = neighbor;
		m_reverse_addr[addr] = id;

		for(auto iter = m_obs.begin();
			iter != m_obs.end();
			++iter)
		{
			DBG_PRINT_VALUE(*iter);
			(*iter)->add_callback(id);
		}
	}
	else
	{
		// TODO
	}
}

void IdTable::del_nehgibor(fon_id_t id)
{
	auto ifaddr_iter = m_index_ifaddr.find(id);

	if(ifaddr_iter != m_index_ifaddr.end())
	{
		m_reverse_addr.erase(ifaddr_iter->second);
		m_index_neighbor.erase(id);
		m_index_ifaddr.erase(id);

		for(auto iter = m_obs.begin();
			iter != m_obs.end();
			++iter)
		{
			(*iter)->del_callback(id);
		}
	}
	else
	{
		// TODO
	}
}

bool IdTable::verify_neighbor(fon_id_t id)
{
	auto iter = m_index_ifaddr.find(id);

	if(iter != m_index_ifaddr.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

mac_addr_t IdTable::get_ifaddr(fon_id_t id)
{
	auto iter = m_index_ifaddr.find(id);

	if(iter != m_index_ifaddr.end())
	{
		return iter->second;
	}
	else
	{
		// TODO
		return MAC_ADDR_WRONG;
	}
}

Neighbor* IdTable::get_neighbor(fon_id_t id)
{
	auto iter = m_index_neighbor.find(id);
	if(iter == m_index_neighbor.end())
	{
		return nullptr;
	}
	else
	{
		return iter->second;
	}
}

fon_id_t IdTable::get_id(mac_addr_t addr)
{
	auto iter = m_reverse_addr.find(addr);

	if(iter != m_reverse_addr.end())
	{
		return iter->second;
	}
	else
	{
		// TODO: 예외처리 제대로
		return -1;
	}
}

fon_id_t IdTable::get_host()
{
	return m_host;
}

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


