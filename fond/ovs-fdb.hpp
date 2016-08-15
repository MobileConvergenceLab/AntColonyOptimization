#include <map>
#include <memory>
#include "types.hpp"
#include "ovs-types.hpp"

struct FdbVector {
	// primary key
	mac_addr_t mac;
	ovs_port_t port;
	ovs_vlan_t vlan;
	ovs_age_t age;
};
typedef std::map<if_name_t, FdbVector> FdbIndex;
typedef std::shared_ptr<FdbIndex> FdbIndexPtr;

class OvsFdb {
public:
	OvsFdb(br_name_t bridge);
	~OvsFdb();

	ovs_port_t get_port(mac_addr_t mac);
	void print();
private:
	br_name_t m_bridge;
	FdbIndexPtr m_index;
};
