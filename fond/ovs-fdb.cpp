#include <stdexcept>
#include <sstream>
#include <pstreams/pstream.h> // See [http://pstreams.sourceforge.net/]
#include "dbg.hpp"
#include "ovs-fdb.hpp"

static void ovs_check()
{
	if(system("ovs-appctl --version") != 0)
	{
		throw std::runtime_error("ovs-appctl");
	}
}

static FdbIndexPtr get_index(br_name_t bridge)
{
	FdbIndexPtr index(new FdbIndex);
	std::string line;
	FdbVector vector;

	redi::ipstream in(std::string("ovs-appctl fdb/show ") + bridge);
	std::getline(in.out(), line);
	std::string str;
	while (in >> vector.port >> vector.vlan >> vector.mac >> vector.age)
	{
		(*index)[vector.mac] = vector;
	}

	return index;
}

OvsFdb::OvsFdb(br_name_t bridge)
:m_bridge(bridge),
m_index(new FdbIndex)
{
	ovs_check();
}

OvsFdb::~OvsFdb()
{
}

ovs_port_t OvsFdb::get_port(mac_addr_t mac)
{
	auto iter = m_index->find(mac);

	// if cache fail, update fdb and try again
	if(iter == m_index->end())
	{
		m_index = get_index(m_bridge);
		iter = m_index->find(mac);
	}

	if(iter != m_index->end())
	{
		return iter->second.port;
	}
	else
	{
		return OVS_PORT_WRONG;
	}
}


void OvsFdb::print()
{
	for(auto iter = m_index->begin();
		iter != m_index->end();
		iter++)
	{
		std::cout << iter->second.port << '\t'
			<< iter->second.mac << std::endl;
	}
}


