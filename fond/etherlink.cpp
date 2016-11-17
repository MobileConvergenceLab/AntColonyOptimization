#include<netinet/in.h>
#include<errno.h>
#include<netdb.h>
#include<stdio.h> //For standard things
#include<stdlib.h>    //malloc
#include<string.h>    //strlen
 
#include<netinet/ip_icmp.h>   //Provides declarations for icmp header
#include<netinet/udp.h>   //Provides declarations for udp header
#include<netinet/tcp.h>   //Provides declarations for tcp header
#include<netinet/ip.h>    //Provides declarations for ip header
#include<netinet/if_ether.h>  //For ETH_P_ALL
#include<net/ethernet.h>  //For ether_header
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <glib-unix.h>
#include <fon/hexdump.h>
#include "etherlink.hpp"
#include "packet.hpp"
#include "dbg.hpp"

#define ETH_P_FON	(0xFF03)
#define ETH_P_FON_ADV	(0xFF05)
#define BUFF_SZ		(1024*4)

//==============================================================================
// private fucntions
//==============================================================================
static int _link_fd(uint16_t ethertype) {

	int broadcastEnable= 1;
	int err	= -1;
	int fd	= -1;

	fd = socket(AF_PACKET, SOCK_DGRAM ,htons(ethertype));
	if(fd < 0) {
		perror("socket()");
		throw std::runtime_error(strerror(errno));
	}

	err = setsockopt(fd,
		SOL_SOCKET,
		SO_BROADCAST,
		&broadcastEnable,
		sizeof(broadcastEnable));
	if(err < 0) {
		perror("setsockopt()");
		throw std::runtime_error(strerror(errno));
	}

	return fd;
} // _link_fd()

static int _get_iface_index(int socket, const char *iface_name) {
	struct ifreq ifr;

	// Ugly hard coded, will be changed
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy((char *)ifr.ifr_name, iface_name, IFNAMSIZ);
	if (ioctl(socket, SIOCGIFINDEX, &ifr) < 0){
		perror("ioctl: ");
		throw std::runtime_error(strerror(errno));
	}

	return ifr.ifr_ifindex;
	// Always success here 2 for eth0, 3 for eth1
} // _get_iface_index()

static void _bind_fd(
		/* output. it will be filed. */
		struct sockaddr_ll *ifaddr,
		/* input. sock_raw should be AF_PACKET && SOCKDGRAM */
		int sock_raw,
		/* input. ex) "eth0" */
		const char *iface_name)
{
	socklen_t len = sizeof(struct sockaddr_ll);

	ifaddr->sll_family = PF_PACKET; 
	ifaddr->sll_ifindex =_get_iface_index(sock_raw, iface_name);
	ifaddr->sll_protocol = 0;	// this field will be ignored
					// because the socket was created with protocol field.

	if((bind(sock_raw, (const struct sockaddr *)ifaddr, sizeof(*ifaddr))) == -1)
	{
		perror("bind: ");
		throw std::runtime_error(strerror(errno));
	}

	// get mac address which is bound to interface.
	getsockname(sock_raw, (struct sockaddr*)ifaddr, &len);

	return;
} // _bind_fd()

static inline void _sall_set_boradcast(struct sockaddr_ll *sa_ll, uint16_t ethertype, int ifindex)
{
	sa_ll->sll_family	= AF_PACKET;
	sa_ll->sll_protocol	= htons(ethertype);
	sa_ll->sll_ifindex	= 0;
	sa_ll->sll_hatype	= ARPHRD_ETHER;
	sa_ll->sll_pkttype	= PACKET_BROADCAST;
	sa_ll->sll_halen	= ETH_ALEN;
	memset(sa_ll->sll_addr, 0xFF, 8);

	return;
}

static mac_addr_t _bin_to_hex_string(struct sockaddr_ll *sa_ll)
{
	std::stringstream ss;

	int i;
	for(i=0;i<sa_ll->sll_halen-1; i++)
	{
		ss << std::setfill ('0') << std::setw(2) << std::hex << (int)sa_ll->sll_addr[i] << ":";
	}
	ss << std::setfill ('0') << std::setw(2) << std::hex << (int)sa_ll->sll_addr[i];

	return ss.str();
}

static gboolean _attach_read_source(GMainContext *context, int fd, GSourceFunc callback, gpointer user_data)
{
	GSource         *source = NULL;
	int             src_id = -1;

	source = g_unix_fd_source_new(fd, G_IO_IN);
	if(source == NULL) {
	return FALSE;
	}

	g_source_set_callback	(source,
				callback,
				user_data,
				NULL);
	src_id = g_source_attach(source, context);
	g_assert(src_id > 0);
	g_source_unref(source);

	#if 0
	DBG_PRINT_VALUE(this);
	DBG_PRINT_VALUE(fd);
	DBG_PRINT_VALUE(source);
	DBG_PRINT_VALUE(src_id);
	#endif

	return TRUE;
} // attach_read_source()

//==============================================================================
// class EtherNeighbor
//==============================================================================
class EtherNeighbor: public Neighbor {
public:
	EtherNeighbor	(IdTablePtr		idtable,
			fon_id_t		id,
			mac_addr_t			addr,
			int			fd_data,
			uint16_t		ethertype_data,
			struct sockaddr_ll	sall);
	~EtherNeighbor();
	int op_sendto(const packet_hdr_t *hdr) override;
private:
	const int 		m_fd_data;
	const int		m_ethertype_data;

	// Neighbor node's interface address(NOT Local address)
	struct sockaddr_ll 	m_sall_data;
	socklen_t		m_socklen;
};

EtherNeighbor::EtherNeighbor(IdTablePtr		idtable,
			fon_id_t		id,
			mac_addr_t			addr,
			int			fd_data,
			uint16_t		ethertype_data,
			struct sockaddr_ll	sall)

			:Neighbor(idtable, id, addr),
			m_fd_data(fd_data),
			m_ethertype_data(ethertype_data),
			m_sall_data(sall),
			m_socklen(sizeof(struct sockaddr_ll))
{
	DBG_LOGGER;

	m_sall_data.sll_protocol = htons(m_ethertype_data);
}

EtherNeighbor::~EtherNeighbor()
{
	DBG_LOGGER;
}

int EtherNeighbor::op_sendto(const packet_hdr_t *hdr)
{
	int sendbyte = -1;
	uint8_t buf[BUFF_SZ];
	int buflen = BUFF_SZ;

	pkt_pack(hdr, buf, &buflen);

	sendbyte = sendto(m_fd_data,
			buf,
			buflen,
			0, /* flags */
			(struct sockaddr*)&m_sall_data,
			m_socklen);

	if(sendbyte == -1) {
		// TODO: Error handling

		DBG_PRINT_MSG("Something is wrong");
	}

	return sendbyte;
}

//==============================================================================
// class Advertiser
//==============================================================================
class Advertiser {
public:
	Advertiser	(if_name_t	ifname,
			IdTablePtr	idtable,
			int		fd_data,
			uint16_t	ethertype_data);
	~Advertiser();
	gboolean attach_send_source(GMainContext *context, int ad_period_sec);
	gboolean attach_read_source(GMainContext *context);
public:
	static gboolean send(Advertiser *adv);
	static gboolean recv(gint fd,
				GIOCondition condition,
				Advertiser *adv);
private:
	if_name_t		m_ifname;
	IdTablePtr		m_idtable;

	// 이 소켓을 이용해서 직접 통신하는것은 아니고,
	// EtherNeighbor 객체를 생성할 때 이용된다.
	int			m_fd_data;

	// ethertype value of m_fd_data(Host byte order)
	uint16_t		m_ethertype_data;

	// 이 소켓을 이용해서 통신이 이루어짐.
	int			m_fd_adv;

	// ethertype value of m_fd_adv(Host byte order)
	uint16_t		m_ethertype_adv;

	// Local Interface's LL socket address
	struct sockaddr_ll	m_if_sa;

	// Broadcast LL socket address
	struct sockaddr_ll	m_bc_sa;

	// cache variables.
	uint8_t			m_buf[BUFF_SZ];
	int			m_buflen;
private:
	static const socklen_t		c_addrlen = sizeof(struct sockaddr_ll);
}; // class Advertise

Advertiser::Advertiser	(if_name_t	ifname,
			IdTablePtr	idtable,
			int		fd_data,
			uint16_t	ethertype_data)

			:m_ifname(ifname),
			m_idtable(idtable),
			m_fd_data(fd_data),
			m_ethertype_data(ethertype_data),
			m_ethertype_adv(ETH_P_FON_ADV)
{
	DBG_LOGGER;

	m_fd_adv = _link_fd(m_ethertype_adv);

	_bind_fd(&m_if_sa, m_fd_adv, m_ifname.c_str());

	packet_buff_t pkt;
	packet_hdr_t *hdr = &pkt.hdr;

	_sall_set_boradcast(&m_bc_sa, m_ethertype_adv, m_if_sa.sll_ifindex);
	m_bc_sa.sll_ifindex = m_if_sa.sll_ifindex;

	m_buflen = BUFF_SZ;

	pkt_hdr_set(hdr,
		// sid
		idtable->get_host(),
		// did
		PACKET_ID_ANY,
		// type
		PACKET_TYPE_ADV);

	pkt_pack(hdr, m_buf, &m_buflen);
	g_assert(m_buflen != 0);

	Advertiser::send(this);
} // Advertiser()

Advertiser::~Advertiser()
{
	DBG_LOGGER;
	close(m_fd_adv);
} // ~Advertiser()

gboolean Advertiser::send(Advertiser *adv)
{
	int sendbyte;

	sendbyte = sendto(adv->m_fd_adv,
			adv->m_buf,
			adv->m_buflen,
			0, /* flags */
			(struct sockaddr*)&adv->m_bc_sa,
			Advertiser::c_addrlen);

	if(sendbyte < 0) {
	}

	return TRUE;
} // send()

gboolean Advertiser::recv	(gint fd,
				GIOCondition condition,
				Advertiser *adv)
{
	uint8_t			buf[BUFF_SZ];
	int			recvbyte;
	packet_buff_t		pkt;
	// source's socket address
	struct sockaddr_ll	sa_ll;
	socklen_t		len = sizeof(struct sockaddr_ll);

	recvbyte = recvfrom(adv->m_fd_adv,
				buf,
				BUFF_SZ,
				0, /* flags */
				(struct sockaddr*)&sa_ll,
				&len);

	if(recvbyte == -1)
	{
		// TODO: Error handling

		//error occured
		throw std::runtime_error(strerror(errno));
		
	}

	pkt_unpack(&pkt.hdr, buf, recvbyte);

	if(!adv->m_idtable->verify_neighbor(pkt.hdr.sid))
	{
		fon_id_t id = pkt.hdr.sid;
		if_name_t addr = _bin_to_hex_string(&sa_ll);

		// managed by idatable
		new EtherNeighbor(adv->m_idtable,
				id,
				addr,
				adv->m_fd_data,
				adv->m_ethertype_data,
				sa_ll);
	}

	return TRUE;
} // recv()

gboolean Advertiser::attach_send_source(GMainContext *context, int ad_period_sec)
{
	GSource         *source = NULL;
	int             src_id = -1;

	source = g_timeout_source_new_seconds (ad_period_sec);
	g_assert(source != NULL);
	if(source == NULL)
	return FALSE;

	g_source_set_callback	(source,
				(GSourceFunc)Advertiser::send,
				this,
				NULL);
	src_id = g_source_attach(source, context);
	g_source_unref(source);
	g_assert(src_id > 0);

	#if 0
	DBG_PRINT_VALUE(this);
	DBG_PRINT_VALUE(m_fd_adv);
	DBG_PRINT_VALUE(source);
	DBG_PRINT_VALUE(src_id);
	#endif

	return TRUE;
} // attach_send_source()

gboolean Advertiser::attach_read_source(GMainContext *context)
{
	return _attach_read_source(
			context,
			m_fd_adv,
			(GSourceFunc) Advertiser::recv,
			this);
} // attach_read_source()


//==============================================================================
// class EtherLink
//==============================================================================
EtherLink::EtherLink	(GMainContext	*context,
			GMainLoop	*loop,
			if_name_t		ifname,
			IdTablePtr	idtable,
			IpcManagerPtr	cm)

			:m_context(context),
			m_loop(loop),
			m_ifname(ifname),
			m_idtable(idtable),
			m_cm(cm),
			m_ethertype_data(ETH_P_FON)
{
	DBG_LOGGER;
	m_fd_data = _link_fd(m_ethertype_data);


	_bind_fd(&m_ifaddr, m_fd_data, m_ifname.c_str());

	this->attach_read_source();

	// init adv object
	m_adv = std::make_shared<Advertiser>(m_ifname,
				m_idtable,
				m_fd_data,
				m_ethertype_data);

	m_adv->attach_send_source(m_context, 3/* advertise period */);
	m_adv->attach_read_source(m_context);

} // EtherLink()


EtherLink::~EtherLink	()
{
	DBG_LOGGER;
	close(m_fd_data);
} // ~EtherLink()

mac_addr_t EtherLink::get_address(if_name_t ifname)
{
	struct sockaddr_ll sa_ll;
	int fd;

	fd = _link_fd(ETH_P_ALL);

 	_bind_fd(&sa_ll,
		fd,
		ifname.c_str());

	close(fd);

	return _bin_to_hex_string(&sa_ll);
}



// compatible with "GSourceFunc" type
gboolean EtherLink::recv	(gint		fd,
				GIOCondition	condition,
				EtherLink	*etherlink)
{
	uint8_t			buf[BUFF_SZ];
	int			recvbyte;
	packet_hdr_shared	pkt = make_packet_hdr_shared(sizeof(packet_buff_t));
	struct sockaddr_ll	addr;
	socklen_t		len = sizeof(struct sockaddr_ll);

	recvbyte = recvfrom(etherlink->m_fd_data,
				buf,
				BUFF_SZ,
				// flags
				0,
				(struct sockaddr*)&addr,
				&len);

	if(recvbyte == -1)
	{
		// TODO: Error handling

		// error occured. ignore it.
		DBG_PRINT_MSG(strerror(errno));
		goto RETURN;
	}

#if LOOP_BACK_CHECK
	// loop-back && cycle check
	if(addr.sll_halen != etherlink->m_ifaddr.sll_halen)
	{
		goto RETURN;
	}
	if(memcmp(addr.sll_addr, etherlink->m_ifaddr.sll_addr, addr.sll_halen) == 0)
	{
		goto RETURN;
	}
#endif // LOOP_BACK_CHECK

	pkt_unpack(&*pkt, buf, recvbyte);

	if(etherlink->m_cm == nullptr)
	{
		DBG_PRINT_MSG("etherlink->m_cm is null pointer. Is it debug mode?");
	}
	else
	{
		etherlink->m_cm->delivery(pkt);
	}

RETURN:
	return TRUE;
} // recv()

gboolean EtherLink::attach_read_source()
{
	return _attach_read_source(
			m_context,
			m_fd_data,
			(GSourceFunc) EtherLink::recv,
			this);
} // attach_read_source()

