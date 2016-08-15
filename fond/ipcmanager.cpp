#include <unistd.h>
#include <glib-unix.h>
#include <fon/fon_ipc_msgs.h>
#include <fon/fon_ipc.h>
#include <fon/fon_ipc.h>
#include "dbg.hpp"
#include "msg_hndler.hpp"
#include "ipcmanager.hpp"

static int
_init_listen_sock(int port, int listen_max)
{
	int			listen_sock = -1;
	struct sockaddr_in	addr = {};
	socklen_t		len = sizeof(struct sockaddr_in);
	int			bf = 1;

	listen_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(listen_sock == -1)
	{
		perror("socket()");
		goto ERROR;
	}

	if(setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &bf, (int)sizeof(bf)))
	{
		perror("setsockopt()");
		goto CLEAN_UP;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port=htons(port);
	if(bind(listen_sock, (struct sockaddr*)&addr, len) == -1) {
		perror("bind()");
		goto CLEAN_UP;
	}

	if(listen(listen_sock, listen_max) == -1) {
		perror("listen()");
		goto CLEAN_UP;
	}

	return listen_sock;

CLEAN_UP:
	close(listen_sock);
	listen_sock = -1;
ERROR:
	DBG_PRINT_MSG("Error");
	exit(EXIT_FAILURE);
} // init_listen_sock()

static void
_attach_listen_sock(GMainContext *context, int listen_sock, GSourceFunc callback, gpointer user_data)
{
	GSource *listen_source;
	guint src_id;

	listen_source = g_unix_fd_source_new(listen_sock, G_IO_IN);
	g_source_set_callback	(listen_source,
				callback,
				user_data,
				NULL);
	src_id = g_source_attach(listen_source, context);
	if(src_id == 0) {
		perror("g_source_attach()");
		exit(EXIT_FAILURE);
	}
	/* from now, listen_source belong to context in loop */
	g_source_unref(listen_source);

	return;

} // _attach_listen_sock()

static int
_init_async_sock(int port) {
	int async_sock = -1;
	struct sockaddr_in addr		= {};
	socklen_t len		 = sizeof(struct sockaddr_in);
	int bf		  = 1;

	async_sock = -1;
	//addr = sockaddr_in addr(0, );
	len = sizeof(struct sockaddr_in);
	bf = 1;

	async_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(async_sock == -1) {
		perror("socket()");
		goto ERROR;
	}

	if(setsockopt(async_sock, SOL_SOCKET, SO_REUSEADDR, &bf, (int)sizeof(bf)))
	{
		perror("setsockopt()");
		goto CLEAN_UP;
	}
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port=htons(port);
	if(bind(async_sock, (struct sockaddr*)&addr, len) == -1) {
		perror("bind()");
		goto CLEAN_UP;
	}

	return async_sock;

CLEAN_UP:
	close(async_sock);
	async_sock = -1;
ERROR:
	exit(EXIT_FAILURE);
} // init_async_sock()

//==============================================================================
// class Client
//==============================================================================
Client::Client	(GMainContext		*context,
		GMainLoop		*loop,
		FibPtr			fib,
		IpcManager 		&cm,
		int			syncsock,
		int			async_sock)

		:m_context(context),
		m_loop(loop),
		m_fib(fib),
		m_cm(cm),
		m_syncsock(syncsock),
		m_async_sock(async_sock),
		m_type(-1)
{
	int src_id;

	m_syncsource = g_unix_fd_source_new(m_syncsock, G_IO_IN);
	g_source_set_callback	(m_syncsource,
				(GSourceFunc)Client::callback,
				this,
				NULL);

	src_id = g_source_attach(m_syncsource,
				m_context);

	if(src_id < 0) {
		// ERROR
		// TODO
		perror("g_source_attach()");
		exit(EXIT_FAILURE);
	}

	DBG_PRINT_MSG("Session has been established.");
}

void Client::reg()
{
	// add(register) to clientmanager
	m_cm.add_client(m_type, this);
}

void Client::dereg()
{
	if(m_type != -1)
	{
		m_cm.del_client(m_type);
	}
}

void Client::delivery(packet_hdr_shared	pkt)
{
	int		sendbyte	= -1;
	socklen_t	len = sizeof(struct sockaddr_in);

	sendbyte = sendto(m_async_sock,
			&*pkt,
			sizeof(packet_buff_t),
			0,
			(struct sockaddr*)&m_asyncaddr,
			len);

	if(sendbyte == -1)
	{
		// TODO
		perror("sendto()");
		exit(EXIT_FAILURE);
	}

	return;
}

Client* Client::create		(GMainContext		*context,
				GMainLoop		*loop,
				FibPtr			fib,
				IpcManager 		&cm,
				int			syncsock,
				int			asyncsock)
{
	return new Client	(context,
				loop,
				fib,
				cm,
				syncsock,
				asyncsock);
}

Client* Client::create		(GMainContext		*context,
				GMainLoop		*loop,
				FibPtr			fib,
				IpcManager 		*cm,
				int			syncsock,
				int			asyncsock)
{
	return new Client	(context,
				loop,
				fib,
				*cm,
				syncsock,
				asyncsock);
}

Client::~Client()
{
	this->dereg();

	g_source_unref(m_syncsource);
	// after unref(), still, context has a reference and source is alive.
	g_source_destroy(m_syncsource);

	close(m_syncsock);
}

gboolean Client::callback	(gint fd,
				GIOCondition condition,
				Client *client)
{
	msg_req_buff_t  req_buff;

	if(!msg_req_recv(fd, &req_buff.hdr, sizeof(msg_req_buff_t)))
	{
		DBG_PRINT_MSG("Session has been closed");
		delete client;

		goto RETURN;
	}

	if(req_buff.hdr.msg_type < 0 || req_buff.hdr.msg_type >= MSG_TYPE_MAX) {
		/* Critical error(in client or library side). */
		goto ERROR;
	}

	ClientMsgHndler::req_handlers(client, &req_buff.hdr);

RETURN:
	return TRUE;
ERROR:
	//dbg("Error");
	g_assert_not_reached();
	exit(EXIT_FAILURE);
}

//==============================================================================
// class IpcManager
//==============================================================================
IpcManager::IpcManager	(GMainContext		*context,
			GMainLoop		*loop,
			FibPtr			fib,
			int			listen_port)
			
			:m_context(context),
			m_loop(loop),
			m_fib(fib),
			m_listen_port(listen_port)
{
	int listen_max = 0xDEAD;

	m_listen_sock = _init_listen_sock(m_listen_port,
			listen_max);

	_attach_listen_sock(m_context,
			m_listen_sock,
			(GSourceFunc)IpcManager::accept_callback,
			this);

	int async_port = 0xDEAD;
	m_async_sock = _init_async_sock(async_port);

}

IpcManager::~IpcManager()
{
	for(auto iter = m_index.begin();
		iter != m_index.end();
		iter++)
	{
		delete iter->second;
	}

	close(m_listen_sock);
	close(m_async_sock);
}

void IpcManager::add_client	(fon_type_t		type,
				Client			*client)
{
	m_index[type] = client;
}

void IpcManager::del_client	(fon_type_t		type)
{
	m_index.erase(type);
}

void IpcManager::delivery(packet_hdr_shared	pkt)
{
	auto iter = m_index.find(pkt->type);

	if(iter == m_index.end())
	{
		DBG_PRINT_MSG("There is no type");
	}
	else
	{
		iter->second->delivery(pkt);
	}
}

/*GUnixFDSourceFunc */
gboolean IpcManager::accept_callback	(gint fd,
					GIOCondition condition,
					IpcManager *manager)
{

	//struct sockaddr_in  addr_from;
	//socklen_t len = sizeof(struct sockaddr_in);
	int client;

	client = accept(fd, NULL, 0);
	//client = accept(fd, (struct sockaddr*)&addr_from, &len);
	if(client == -1) {
		// ERROR;
		perror("accept()");
		exit(EXIT_FAILURE);
	}

	// create client object
	Client::create	(manager->m_context,
			manager->m_loop,
			manager->m_fib,
			manager,
			client,
			manager->m_async_sock);

	return TRUE;
} // accept_callback()


