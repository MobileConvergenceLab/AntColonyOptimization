#include <iostream>
#include <glib.h>
#include <fon/fon_ipc_defs.h>
#include "ipcmanager.hpp"

int main()
{
	GMainContext	*context = g_main_context_new();
	GMainLoop	*loop = g_main_loop_new(context, FALSE);
	fon_id_t	host = 14;
	IdTablePtr	idtable(new IdTable(host));
	FibPtr		fib(new Fib(idtable, nullptr));

	IpcManager manager	(context,
				loop,
				fib,
				FON_IPC_LISTEN_PORT);

	g_main_loop_run(loop);

	std::cout << "TEST IPC" << std::endl;
}
