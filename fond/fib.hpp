/**
 * Implementation of (Forward Information Base, FIB) Table 
 */
#ifndef FIB_HPP
#define FIB_HPP

#include <vector>
#include <map>
#include <memory>
#include <fon/fon_types.h>
#include <fon/fon_fib_if.h>
#include "neighbor.hpp"
#include "ovs-if.hpp"

// responsible for knowing where packet to be sent.
class Fib: public IdTableObserver {
public:
	Fib(IdTablePtr idtable, OvsIfPtr ovsif);
	~Fib();
	void update(fib_tuple_t tuple);
	void del(fon_id_t id);
	bool get(fon_id_t id, fib_tuple_t* tuple);
	std::vector<fib_tuple_t> get_all();

	void add_callback(fon_id_t id) override;
	void del_callback(fon_id_t id) override;
private:
	void _insert_tuple(fib_tuple_t tuple);
	void _delete_tuple(fon_id_t id);
private:
	OvsIfPtr			m_ovsif;

	// index of target to FIB tuple
	std::map<fon_id_t, fib_tuple_t>	m_index;

	// reverse index of neighbor to target
	std::multimap<fon_id_t, fon_id_t> m_reverse;
};
typedef std::shared_ptr<Fib> FibPtr;

#endif /* FIB_HPP */
