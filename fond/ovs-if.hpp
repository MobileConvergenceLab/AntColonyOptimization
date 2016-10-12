#ifndef OVSIF_H
#define OVSIF_H

#include <string>
#include <list>
#include <memory>
#include <fon/fon_types.h>
#include "types.hpp"
#include "neighbor.hpp"
#include "ovs-fdb.hpp"


class OvsIf {
public:
	OvsIf(IdTablePtr idtable, if_name_t internal, br_name_t bridge, if_list_t if_list, bool br_create);
	~OvsIf();
	bool add_flow_neighbor(fon_id_t neighbor);
	bool add_flow_target(fon_id_t target, fon_id_t neighbor);
private:
	IdTablePtr m_idtable;

	// internal interface to be connected to virtual bridge
	if_name_t m_internal;

	// virtual bridge(switch) name
	br_name_t m_bridge;

	// physical interface names
	if_list_t m_if_list;

	// for mininet emulation mode
	const bool m_br_create;

	OvsFdb m_db;

	std::map<fon_id_t, ovs_flow_t> m_flows;
};
typedef std::shared_ptr<OvsIf> OvsIfPtr;

#endif // OVSIF_H
