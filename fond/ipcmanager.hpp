#ifndef IPC_MANAGER_HPP
#define IPC_MANAGER_HPP

#include <memory>
#include <map>
#include <glib.h>
#include <fon/fon_types.h>
#include "fib.hpp"

class Client;
class IpcManager;

//==============================================================================
// class Client
//==============================================================================
class Client {
friend class ClientMsgHndler;

public:
	~Client();
	void reg			();
	void dereg			();
	void delivery			(packet_hdr_shared	pkt);
public:
	static Client* create		(GMainContext		*context,
					GMainLoop		*loop,
					FibPtr			fib,
					IpcManager 		&cm,
					int			syncsock,
					int			asyncsock);
	static Client* create		(GMainContext		*context,
					GMainLoop		*loop,
					FibPtr			fib,
					IpcManager 		*cm,
					int			syncsock,
					int			asyncsock);
	static gboolean callback	(gint fd,
					GIOCondition condition,
					Client *client);
private:
	// Don't allow static(stack) objects.
	Client				(GMainContext		*context,
					GMainLoop		*loop,
					FibPtr			fib,
					IpcManager 		&cm,
					int			syncsock,
					int			async_sock);
	Client				(const Client& c) = delete;
private:
	GMainContext		*m_context;
	GMainLoop		*m_loop;
	FibPtr			m_fib;
	IpcManager		&m_cm;
	/* client's sync channel socket(connection oriented) */
	int			m_syncsock;
	GSource 		*m_syncsource;
	// client's async channel address(message oriented)
	int			m_async_sock;
	struct sockaddr_in	m_asyncaddr;
	fon_type_t		m_type;
};

//==============================================================================
// class IpcManager
//==============================================================================
class IpcManager {
public:
	IpcManager			(GMainContext		*context,
					GMainLoop		*loop,
					FibPtr			fib,
					int			listen_port);
	~IpcManager			();
	void add_client			(fon_type_t		type,
					Client			*client);
	void del_client			(fon_type_t		type);
	void delivery			(packet_hdr_shared	pkt);
public:
	static gboolean accept_callback	(gint fd,
					GIOCondition condition,
					IpcManager *manager);
private:
	typedef std::map<fon_type_t, Client*> ClientIndex;
	GMainContext		*m_context;
	GMainLoop		*m_loop;
	FibPtr			m_fib;
	ClientIndex		m_index;
	int			m_listen_port;
	int			m_listen_sock;
	int			m_async_sock;
};

typedef std::shared_ptr<IpcManager> IpcManagerPtr;

#endif // IPC_MANAGER_HPP
