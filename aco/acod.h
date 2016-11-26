#ifndef MAIN_H
#define MAIN_H

#include <glib.h>
#include <fon/fon.h>
#include "aco-table.h"
#include "parameters.h"

typedef struct _AcoDaemon {
	GMainContext	*context;
	GMainLoop	*loop;
	FonClient	*fclient;
	AcoTable	*table;
	aco_parameters	para;

	int		ipc_listen_fd;
	int		ipc_client_fd;

	// find list
	aco_id_t	flist[1024];
	int		flist_len;
} AcoDaemon;

AcoDaemon*	aco_daemon_create		(aco_parameters	*para);
void		aco_daemon_request_attach	(AcoDaemon* daemon,
						aco_id_t target,
						int ncycle,
						int npacket_per_cycle,
						// the timeout interval in milliseconds
						guint interval);
void		aco_daemon_run			(AcoDaemon* daemon);
void		aco_daemon_flist_add		(AcoDaemon* daemon,
						aco_id_t target);

#endif /* MAIN_H */
