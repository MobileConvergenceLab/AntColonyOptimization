#include <stdlib.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "dbg.hpp"
#include "ovs-if.hpp"
#include "etherlink.hpp"


/*==============================================================================
 * private fucntions
 *==============================================================================*/
static int
_system_redirect_to_nulldev(std::string command)
{
	static const std::string SHELL_REDIRECT(" > /dev/null 2>&1");

	return system((command + SHELL_REDIRECT).c_str());
}

static void
_ovs_of_check()
{
	DBG_LOGGER;

	if(_system_redirect_to_nulldev("ovs-vswitchd --version") !=0)
	{
		throw std::runtime_error("OVS check Error");
	}

	if(_system_redirect_to_nulldev("ovs-vsctl --version") !=0)
	{
		throw std::runtime_error("OVS check Error");
	}

	if(_system_redirect_to_nulldev("ovs-ofctl --version") !=0)
	{
		throw std::runtime_error("OVS check Error");
	}

	return;
}

static void
_make_br(const br_name_t &bridge)
{
	DBG_LOGGER;

	std::stringstream command;

	command << "ovs-vsctl add-br ";
	command << bridge;
	_system_redirect_to_nulldev(command.str());

	return;
}

static void
_add_port(const br_name_t &bridge, const if_list_t &if_list)
{
	DBG_LOGGER;

	std::stringstream command;

	command << "ovs-vsctl add-port " << bridge << " ";
	for(auto iter = if_list.begin(); iter != if_list.end(); iter++)
	{
		_system_redirect_to_nulldev(command.str() + *iter);
	}
}

static void
_add_initial_flow	(const br_name_t&bridge,
				if_name_t internal,
				ovs_port_t port)
{
	DBG_LOGGER;

	std::string OVS_OF_ADD_FLOW = std::string("ovs-ofctl add-flow ") + bridge;
	std::stringstream command;

	command << OVS_OF_ADD_FLOW << " priority=1,dl_src=" << EtherLink::get_address(internal) <<",action=normal";
	_system_redirect_to_nulldev(command.str());

	command = std::stringstream("");
	command << OVS_OF_ADD_FLOW << " priority=1,dl_dst=" << EtherLink::get_address(internal) <<",action=normal";
	_system_redirect_to_nulldev(command.str());

	command = std::stringstream("");
	command << OVS_OF_ADD_FLOW << " priority=1,dl_dst=ff:ff:ff:ff:ff:ff,action=output:" << port;
	_system_redirect_to_nulldev(command.str());


	// For ICMP pacekt

	// XXX 2016.9.9
	// 이 부분을 활성화 하면
	// ONOS의 Web GUI 쪽이 정상작동하지 않는다. 왜?
//	command = std::stringstream("");
//	command << OVS_OF_ADD_FLOW << " priority=2000,ip,nw_ttl=1,icmp,action=dec_ttl,normal:" << port;
//	system(command.str().c_str());
}


static void
_clean_up_br(const br_name_t&bridge, const std::list<std::string> &if_list)
{
	DBG_LOGGER;

	std::stringstream command;
	command << "ovs-vsctl del-br " << bridge;
	_system_redirect_to_nulldev(command.str());
}

static ovs_flow_t
_make_ip_flow	(ip_addr_t ip_addr,
				br_name_t bridge,
				ovs_port_t port,
				mac_addr_t mac)
{
	std::stringstream command;

	command << bridge << " priority=100,ip,nw_dst=" << ip_addr
		<<",actions=dec_ttl,mod_dl_dst:" << mac
		<< ",output:" << port;

	return command.str();
}

static void
_add_flow	(ovs_flow_t flow)
{
	std::string command = std::string("ovs-ofctl add-flow ") + flow;

	//_system_redirect_to_nulldev(command);
	system(command.c_str());
}

static void _add_arp	(ip_addr_t ip_addr,
			mac_addr_t mac_addr)
{
	std::stringstream command;
	command << "arp -s " << ip_addr << ' ' << mac_addr;
	_system_redirect_to_nulldev(command.str());
}

/*==============================================================================
 * class OvsIf
 *==============================================================================*/
OvsIf::OvsIf(IdTablePtr idtable,
	if_name_t internal,
	br_name_t bridge,
	if_list_t if_list,
	bool br_create)

	:m_idtable(idtable),
	m_internal(internal),
	m_bridge(bridge),
	m_if_list(if_list),
	m_br_create(br_create),
	m_db(bridge)
{
	DBG_LOGGER;

	_ovs_of_check();

	if(!m_br_create)
	{
		_make_br(m_bridge);
		_add_port(m_bridge, m_if_list);

		std::string command = std::string("ifconfig ") + m_internal + std::string(" up");
		_system_redirect_to_nulldev(command);
	}
	else
	{
		// do nothing
	}

	_add_initial_flow(m_bridge,
		m_internal,
		/* assume that internal port number is 1 */
		1);

}

OvsIf::~OvsIf()
{
	DBG_LOGGER;

	if(!m_br_create)
	{
		_clean_up_br(m_bridge, m_if_list);
	}
	else
	{
		// do nothing
	}
}

bool OvsIf::add_flow_neighbor(fon_id_t neighbor)
{
	assert(0);
	exit(EXIT_FAILURE);

	/* 수정할게 많음 이유:
	 * 본 함수는 인접한 노드가 발견되었을 때 콜백함수로 호출된다.
	 * 하지만 본 데몬은 ovs-db가 갱신되기 전에 본 함수가 호출되기 때문에
	 * mac을 찾을 수 없게 된다. */
	assert(false);

	mac_addr_t mac = m_idtable->get_ifaddr(neighbor);
	if(mac == MAC_ADDR_WRONG)
	{
		return false;
	}

	ovs_port_t port = m_db.get_port(mac);
	assert(port == -1);	// XXX logic error

	std::stringstream command;
	command << m_bridge << " dl_dst=" << mac <<"action=output:" << port;

	ovs_flow_t flow = command.str();

	_add_flow(flow);

	return true;
}

bool OvsIf::add_flow_target(fon_id_t target, fon_id_t neighbor)
{
	mac_addr_t mac_addr = m_idtable->get_ifaddr(neighbor);
	if(mac_addr == MAC_ADDR_WRONG)
	{
		return false;
	}

	ovs_port_t port = m_db.get_port(mac_addr);

	if(port == -1)
	{
		// m_db가 최신 정보가 아닐 수 있다. 이럴 경우 플로우 추가 실패한다.
		// 이 경우가 아닌데 실패한다면, 로직을 잘못 짰을 것이다.
		return false;
	}

	ip_addr_t ip_addr = std::string(inet_ntoa(*(struct in_addr*)&target));
	ovs_flow_t flow = _make_ip_flow(ip_addr, m_bridge, port, mac_addr);

	auto iter = m_flows.find(target);

	if(iter == m_flows.end() ||
	   iter->second != flow)
	{
		// OVS상에 flow 추가
		_add_flow(flow);

		// flow 리스트에 추가
		m_flows[target] = flow;

		// ARP 테이블 추가
		// flow에 mac주소 변경하는 부분이 있기 때문에
		// 사실 dummy값을 넣어도 상관은 없다.
		_add_arp(ip_addr, mac_addr);
	}
	else
	{
		// Do nothing
	}

	return true;
}










