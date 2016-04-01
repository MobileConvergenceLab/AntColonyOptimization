#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#include "fon/fon.h"
#include "common.h"

static gboolean recv_callack(const packet *pkt, gpointer user_data) {
    pkt_hdr_print(pkt);
    return TRUE;
}

int main(int argc, char **argv)
{
    int             type        = FON_FUNC_TYPE;
    int             port        = -1;
    GMainContext    *context    = NULL;
    GMainLoop       *loop       = NULL;

    if(argc != 2) {
        g_print("port has been set as FON_CORE_LISTEN_PORT(0x%X)\n", FON_CORE_LISTEN_PORT);
        port = FON_CORE_LISTEN_PORT;
    }
    else {
        port = atoi(argv[1]);
    }

    loop    = g_main_loop_new(context, FALSE);
    context = g_main_loop_get_context(loop);
    g_main_context_unref(context);

    if(!fon_init(context, type, port)) {
        exit(EXIT_FAILURE);
    }

    fon_set_callback_recv(recv_callack, NULL);

    attach_table_print(context);
    g_main_loop_run(loop);

    return 0;
}

