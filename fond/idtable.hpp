#ifndef IDTABLE_HPP
#define IDTABLE_HPP

#include <sys/socket.h>
#include <memory>
#include <map>
#include <set>
#include <fon/fon_types.h>
#include "types.hpp"

/**
 * This implementation use The "Observer Pattern".
 *
 * Subject(Publisher): class IdTable
 * Observer(Subscriber): class IdTableObserver
 */

class IdTable;
class Neighbor;
class IdTableObserver;
typedef std::shared_ptr<IdTable> IdTablePtr;

class IdTableObserver {
public:
	IdTableObserver(IdTablePtr idtable);
	virtual ~IdTableObserver();

	IdTablePtr get_idtable();
	virtual void add_callback(fon_id_t id) = 0;
	virtual void del_callback(fon_id_t id) = 0;
private:
	IdTablePtr m_idtable;
};

// class IdTable
// Host Id and Neighbor Id management class.
class IdTable {
public:
	IdTable(fon_id_t host);
	~IdTable();
	bool reg_observer(IdTableObserver *ob);
	void dereg_observer(IdTableObserver *ob);
	void add_neighbor(fon_id_t id, mac_addr_t addr, Neighbor *neighbor);
	void del_nehgibor(fon_id_t id);
	bool verify_neighbor(fon_id_t id);
	mac_addr_t get_ifaddr(fon_id_t id);
	Neighbor* get_neighbor(fon_id_t id);
	fon_id_t get_id(mac_addr_t addr);
	fon_id_t get_host();
private:
	fon_id_t			m_host;
	std::set<IdTableObserver*>	m_obs;
	std::map<fon_id_t, mac_addr_t> 	m_index_ifaddr;
	std::map<fon_id_t, Neighbor*>	m_index_neighbor;
	std::map<mac_addr_t, fon_id_t>	m_reverse_addr;
};

#endif /* IDTABLE_HPP */
