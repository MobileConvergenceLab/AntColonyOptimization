#ifndef ETHERLINK_HPP
#define ETHERLINK_HPP

#include <list>
#include <glib.h>
#include <memory>
#include <linux/if_packet.h>
#include "types.hpp"
#include "neighbor.hpp"
#include "ipcmanager.hpp"

typedef std::list<struct sockaddr_ll> mac_addr_tList;

class Advertiser;
typedef std::shared_ptr<Advertiser> AdvertiserPtr;

class EtherLink {
public:
	EtherLink	(GMainContext	*context,
			GMainLoop	*loop,
			if_name_t	ifname,
			IdTablePtr	idtable,
			IpcManagerPtr	cm = nullptr);
	~EtherLink	();
	gboolean attach_read_source();
public:
	static gboolean recv	(gint		fd,
				GIOCondition	condition,
				EtherLink	*etherlink);
private:
	GMainContext		*m_context;
	GMainLoop		*m_loop;
	if_name_t			m_ifname;
	IdTablePtr		m_idtable;
	IpcManagerPtr		m_cm;
	uint16_t		m_ethertype_data;

	// socket bound to OVS-VS internal interface.
	int			m_fd_data;

	// interface's address.
	struct sockaddr_ll	m_ifaddr;

	AdvertiserPtr		m_adv;
public:
	static mac_addr_t get_address(if_name_t ifname);
};
typedef std::shared_ptr<EtherLink> EtherLinkPtr;


#endif // ETHERLINK_HPP
