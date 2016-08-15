#include <glib-unix.h>
#include <fon/fon_fib_if.h>
#include "aco-table.h"
#include "ant.h"
#include "acod.h"

#define CHECK_POINT	(printf("Check point:: %s(%s:%d)\n", __FUNCTION__, __FILE__, __LINE__))

static void
_make_pkt(const Ant		*ant,
	packet_hdr_t		*pkt,
	int			*remain,
	aco_id_t		host,
	aco_id_t		neighbor)
{
	void *pos = pkt->data;

	ant_marshalling((const Ant*)ant, &pos, remain);

	// aco_id_t 와 fon_id_t는 1:1 대응된다.(캐스팅 가능)
	pkt->sid	= host;
	pkt->did	= neighbor;
	pkt->paylen	= pos - (void*)pkt->data;
	pkt->type	= FON_FUNC_TYPE_ACO;
}

static void _sendto(const Ant* ant,
		aco_id_t host,
		aco_id_t neighbor)
{
	FonClient	*fclient	= ant->user_data;
	packet_buff_t	pkt		= {0,};
	int		remain		= sizeof(packet_buff_t);

	_make_pkt(ant, &pkt.hdr, &remain, host, neighbor);
	fon_sendto(fclient, &pkt.hdr);
}

/**
 * 코어 데몬으로부터 테이블이 변경되었다는 통보를 받으면
 * 해당 루틴이 호출되어야 한다. */
static gboolean _table_add_entry(FonClient *fclient,
				AcoTable *table)
{

	int		i;
	int		len;
	Array		*tuple_array	= NULL;
	fib_tuple_t	*tuple;

	fon_table_get(fclient, &tuple_array);

	len = tuple_array->len;
	tuple = &array_index(tuple_array, fib_tuple_t, 0);

	for(i=0; i<len; i++)
	{
		if(tuple[i].hops == 1)
		{
			aco_table_add_neigh(table, tuple[i].target);
		}
		else
		{
			aco_table_add_target(table, tuple[i].target);
		}
	}

	array_unref(tuple_array);

	return TRUE;
}

static AcoTable* _table_create(FonClient *fclient,
				aco_ph_t min,
				aco_ph_t max,
				int endurance_max)
{
	fon_id_t host_id = FON_ID_WRONG;

	if(!fon_host_get(fclient, &host_id))
	{
		perror("Call fon_host_get()");
		exit(EXIT_FAILURE);
	}

	AcoTable* table = aco_table_new(host_id, min, max, endurance_max);

	if(!_table_add_entry(fclient, table))
	{
		perror("Call table_init_from_daemon()");
		exit(EXIT_FAILURE);
	}

	return table;
}

static gboolean _add_timeout_source(GMainContext* context,
				GSourceFunc event_hdler,
				// the timeout interval in milliseconds
				guint interval,
				gpointer data,
				GDestroyNotify notify)
{
	if(interval <= 0)
	{
		return TRUE;
	}

	gint src_id = 0;
	GSource *source = NULL;

	source = g_timeout_source_new(interval);
	if(source == NULL)
	{
		return FALSE;
	}

	// always success
	g_source_set_callback(source,
			(GSourceFunc)event_hdler,
			data,
			notify);

	src_id = g_source_attach(source, context);
	g_source_unref(source);
	if(src_id == 0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

// Callback argument
typedef struct _RequestHandlerArg {
	GMainContext	*context;
	AcoTable	*table;
	FonClient	*fclient;
	aco_id_t	source;
	aco_id_t	target;
	// current cycle
	int		cycle;
	// maximum cycle
	int		ncycle;
	// pacekt per cycle
	int		npacket_per_cycle;
} RequestHandlerArg;

// compatible with GSourceFunc
static gboolean _request_handler(RequestHandlerArg *arg)
{
	if(arg->cycle++ < arg->ncycle)
	{
		Ant* ant = ant_factory(ANT_TYPE_ROUNDTRIP,
					arg->source,
					arg->target,
					arg->table,
					_sendto,
					arg->fclient);

		for(int i=0; i< arg->npacket_per_cycle; i++)
		{
			ant_send(ant);
		}
		ant_unref(ant);

		// TRUE: continue
		return TRUE;
	}
	else
	{
		// FALSE: stop callback
		return FALSE;
	}
}

static gboolean _add_read_source(GMainContext *context,
				int fd,
				GSourceFunc callback,
				gpointer user_data,
				GDestroyNotify notify)
{
	GSource		*source = NULL;
	int		src_id = -1;

	source = g_unix_fd_source_new(fd, G_IO_IN);
	if(source == NULL)
	{
		return FALSE;
	}

	g_source_set_callback	(source,
				callback,
				user_data,
				notify);
	src_id = g_source_attach(source, context);
	g_assert(src_id > 0);
	g_source_unref(source);

	return TRUE;
} // attach_read_source()

// Callback argument
typedef struct _RecvEventArg {
	GMainContext *context;
	AcoTable *table;
	FonClient *fclient;
	const packet_hdr_t *pkt;
} RecvEventArg;

// compatible with GSourceFunc
static gboolean _fon_recv_callack(gint fd,
				GIOCondition condition,
				RecvEventArg *arg)
{
	packet_buff_t	 buff		= {0,};

	fon_recvfrom(arg->fclient,
		 &buff.hdr,
		 sizeof(packet_buff_t));

	Ant* ant = ant_demarshalling(&buff.hdr.data,
				 buff.hdr.paylen,
				 arg->table,
				 _sendto,
				 arg->fclient);

	ant_object_arrived_at(ant->obj,
			arg->table->host);

	// DEBUG
	CHECK_POINT;
	aco_table_print_all(arg->table);

	ant_callback(ant);
	ant_unref(ant);

	return TRUE;
}

// 
static void _aco_daemon_recv_attach(AcoDaemon* daemon)
{
	RecvEventArg *arg	= malloc(sizeof(RecvEventArg));
	int fd			= daemon->fclient->async_sock;
	GDestroyNotify notify	= free;

	arg->context = daemon->context;
	arg->table = daemon->table;
	arg->fclient = daemon->fclient;

	_add_read_source(daemon->context,
			fd,
			(GSourceFunc)_fon_recv_callack,
			arg,
			notify);
}

static void _add_flow_neighbors(AcoDaemon* daemon)
{
	AcoTable *table = daemon->table;
	FonClient *fclient = daemon->fclient;

	aco_ids_t neighbors;
	fib_tuple_t tuple;

	neighbors = aco_table_new_neighs(table);

	for(int i=0; neighbors[i] != ACO_ID_WRONG; i++)
	{
		tuple.target		= (fon_id_t)neighbors[i];
		tuple.neighbor		= (fon_id_t)neighbors[i];
		tuple.hops		= (fon_dist_t)1;
		tuple.validation	= FIB_TUPLE_VALID;

		fon_table_add(fclient,
				&tuple);
	}

	aco_table_free_ids(neighbors);
}

static void _add_flow_targets(AcoDaemon* daemon)
{
	AcoTable *table = daemon->table;
	FonClient *fclient = daemon->fclient;

	aco_ids_t targets;
	aco_id_t neighbor;
	fib_tuple_t tuple;
	AcoValue value;

	targets = aco_table_new_targets(table);

	for(int i=0; targets[i] != ACO_ID_WRONG; i++)
	{
		neighbor = aco_table_max_pheromon(table, targets[i], &value);

		if(neighbor != ACO_ID_WRONG)
		{
			assert(neighbor == value.neigh);

			tuple.target		= (fon_id_t)targets[i];
			tuple.neighbor		= (fon_id_t)neighbor;
			tuple.hops		= (fon_dist_t)value.local_min;
			tuple.validation	= FIB_TUPLE_VALID;

			fon_table_add(fclient,
					&tuple);
		}
	}

	aco_table_free_ids(targets);
}

// compatible with GSourceFunc
// 계산된 경로를 적용한다.
static gboolean _fib_update_handler(AcoDaemon* daemon)
{
	_add_flow_neighbors(daemon);
	_add_flow_targets(daemon);

	return true;
}

static void _aco_daemon_fib_update_attach(AcoDaemon* daemon,
			// the timeout interval in milliseconds
			guint interval)
{
	if(!_add_timeout_source(daemon->context,
				(GSourceFunc)_fib_update_handler,
				interval,
				daemon,
				NULL))
	{
		perror("Call forward_timeout_add()");
		exit(EXIT_FAILURE);
	}
}

// compatible with GSourceFunc
static gboolean _fon_dead_handler(gint fd,
				GIOCondition condition,
				gpointer noused)
{
	if(condition != G_IO_IN)
	{
		printf("FOND may be dead.\n");
		exit(EXIT_FAILURE);
	}

	return TRUE;
}

// FOND가 죽었을 때 처리하는 함수
static void _aco_daemon_dead_handler_attach(AcoDaemon* daemon)
{
	// this fd is connection-oriented socket.
	// So, if disconnection occur,
	// we will find out that "Daemon is dead" through this fd.

	int fd			= daemon->fclient->sync_sock;

	_add_read_source(daemon->context,
			fd,
			(GSourceFunc)_fon_dead_handler,
			NULL,
			NULL);
}

static int _ipc_sock(int ipc_port)
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
	addr.sin_port=htons(ipc_port);
	if(bind(listen_sock, (struct sockaddr*)&addr, len) == -1) {
		perror("bind()");
		goto CLEAN_UP;
	}

	if(listen(listen_sock, 1) == -1) {
		perror("listen()");
		goto CLEAN_UP;
	}

	return listen_sock;

CLEAN_UP:
	close(listen_sock);
	listen_sock = -1;
ERROR:
	exit(EXIT_FAILURE);
}

// compatible with GSourceFunc
static gboolean _ipc_accept(gint fd,
			GIOCondition condition,
			AcoDaemon* daemon)
{
	assert(fd == daemon->ipc_listen_fd);

	if(daemon->ipc_client_fd == -1)
	{
		daemon->ipc_client_fd = accept(daemon->ipc_listen_fd,
						// We don't need client's sock address
						NULL,
						0);
		if(daemon->ipc_client_fd == -1)
		{
			perror("accept()");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		// ignore request
	}

	return true;
}


/*==============================================================================
 *
 *==============================================================================*/
AcoDaemon* aco_daemon_create(aco_parameters *para)
{
	AcoDaemon *daemon = malloc(sizeof(AcoDaemon));

	//
	daemon->para = *para;

	// init GMainContext && GMainLoop
	daemon->context = g_main_context_new();
	if(daemon->context == NULL)
	{
		exit(EXIT_FAILURE);
	}

	daemon->loop = g_main_loop_new(daemon->context, FALSE);
	if(daemon->loop == NULL)
	{
		exit(EXIT_FAILURE);
	}
	g_main_context_unref(daemon->context);
	
	daemon->fclient = fon_client_new(FON_FUNC_TYPE_ACO, para->fon_client_port);
	if(daemon->fclient == NULL)
	{
		perror("Call fon_init()");
		exit(EXIT_FAILURE);
	}

	// Init Table
	daemon->table = _table_create(daemon->fclient,
				para->min,
				para->max,
				para->endurance_max);
	if(daemon->table == NULL)
	{
		perror("Call _table_create()");
		exit(EXIT_FAILURE);
	}

	_aco_daemon_recv_attach(daemon);

	_aco_daemon_dead_handler_attach(daemon);

	_aco_daemon_fib_update_attach(daemon,
				// milisecond
				5000);

	return daemon;
}

void aco_daemon_request_attach(AcoDaemon* daemon,
			aco_id_t target,
			int ncycle,
			int npacket_per_cycle,
			// the timeout interval in milliseconds
			guint interval)
{
	RequestHandlerArg *arg = malloc(sizeof(RequestHandlerArg));

	arg->table = daemon->table;
	arg->fclient = daemon->fclient;
	arg->source = daemon->table->host;
	arg->target = target;
	arg->cycle = 0;
	arg->ncycle = ncycle;
	arg->npacket_per_cycle = npacket_per_cycle;

	//g_print("Call forward_timeout_add()\n");
	if(!_add_timeout_source(daemon->context,
				(GSourceFunc)_request_handler,
				interval,
				arg,
				free))
	{
		perror("Call forward_timeout_add()");
		exit(EXIT_FAILURE);
	}
}

void aco_daemon_run(AcoDaemon* daemon)
{
	g_main_loop_run(daemon->loop);
}

