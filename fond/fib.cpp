#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include "dbg.hpp"
#include "fib.hpp"

Fib::Fib(IdTablePtr idtable, OvsIfPtr ovsif)

		:IdTableObserver(idtable),
		m_ovsif(ovsif)
{
	DBG_LOGGER;
}

Fib::~Fib()
{
	DBG_LOGGER;
}

void Fib::_insert_tuple(fib_tuple_t tuple)
{
	m_index[tuple.target] = tuple;
	m_reverse.insert(std::make_pair(tuple.neighbor, tuple.target));
}

void Fib::_delete_tuple(fon_id_t id)
{

	// remove in index
	m_index.erase(id);

	// remove in reverse index
	std::multimap<fon_id_t, fon_id_t>::iterator reverse_cache;

	for(auto iter = m_reverse.find(id);
		iter != m_reverse.end();
		iter++)
	{
		if(iter->second == id)
		{
			// if remove this time,
			// iterator will be no more vaild.
			// so, cache iterator and remove later.
			reverse_cache = iter;
		}
		else
		{
			fib_tuple_t *tuple = &m_index[iter->second];

			tuple->neighbor = FON_ID_WRONG;
			tuple->hops = FON_DIST_WRONG;
			tuple->validation = FIB_TUPLE_INVALID;
		}
	}

	// remove
	m_reverse.erase(reverse_cache);
}

void Fib::update(fib_tuple_t tuple)
{
	if(tuple.hops == 1)
	{
		// wrong requtest, so inogred it.
		// The tuple whose hops is 1, will be added through 
		// Fib::add_callback() method.
	}
	else
	{
		_insert_tuple(tuple);
	}

	m_ovsif->add_flow_target(tuple.target, tuple.neighbor);
}

void Fib::del(fon_id_t id)
{
	//TODO ovs-of 테이블 수정
	assert(false);
}

bool Fib::get(fon_id_t id, fib_tuple_t* tuple)
{
	auto iter = m_index.find(id);
	if(iter != m_index.end())
	{
		*tuple = iter->second;
		return true;
	}
	else
	{
		return false;
	}
}

std::vector<fib_tuple_t> Fib::get_all()
{
	std::vector<fib_tuple_t> v;

	for(auto iter = m_index.begin();
		iter != m_index.end();
		iter++)
	{
		v.push_back(iter->second);
	}

	return v;
}

// override function of IdTableObserver::add_callback
void Fib::add_callback(fon_id_t id)
{
	DBG_LOGGER;

	fib_tuple_t tuple = {
		.target = id,
		.neighbor = id,
		.hops = 1,
		.validation = FIB_TUPLE_VALID };

	_insert_tuple(tuple);
}

// override function of IdTableObserver::del_callback
void Fib::del_callback(fon_id_t id)
{
	_delete_tuple(id);
}

