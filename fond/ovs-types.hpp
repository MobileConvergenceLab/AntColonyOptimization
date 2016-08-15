#ifndef OVS_TYPES_HPP
#define OVS_TYPES_HPP

typedef int ovs_port_t;
#define OVS_PORT_WRONG	(-1);

typedef int ovs_vlan_t;
#define OVS_VLAN_WRONG	(-1);

typedef int ovs_age_t;
#define OVS_AGE_WRONG	(-1)

// ovs-ofctl add-flow "OVS_FLOW"
typedef std::string ovs_flow_t;

#endif // OVS_TYPES_HPP
