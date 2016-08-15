#ifndef FOND_HPP
#define FOND_HPP

#include <list>
#include <string>
#include "ovs-if.hpp"
#include "neighbor.hpp"
#include "etherlink.hpp"
#include "fib.hpp"
#include "ipcmanager.hpp"

class FonDaemon {
public:
	FonDaemon(fon_id_t host, if_name_t internal, br_name_t bridge, if_list_t if_list, bool mininet, int port);
	~FonDaemon();
	void run();
private:
	GMainContext	*m_context;
	GMainLoop	*m_loop;
	// internal interface to be connected to virtual bridge
	if_name_t	m_internal;
	IdTablePtr	m_idtable;
	OvsIfPtr	m_ovsif;
	FibPtr		m_fib;
	EtherLinkPtr	m_ether;
	IpcManagerPtr	m_cm;
};

#endif // FOND_HPP
