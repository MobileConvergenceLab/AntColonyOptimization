#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#include "fon/fon.h"
#include "common.h"

int g_host_id   = SNDER_ID;
int g_type      = FON_FUNC_TYPE;

static gboolean callback(gpointer user_data) {
    packet  pkt;

    pkt.hdr.pkt_sid     = g_host_id;
    pkt.hdr.pkt_did     = 2;
    pkt.hdr.pkt_type    = g_type;
    pkt.hdr.pkt_len     = RCVER_ID;

    fon_sendto(&pkt);

    return TRUE;
}

int main(int argc, char **argv)
{
    int             port        = -1;
    GMainContext    *context    = NULL;
    GMainLoop       *loop       = NULL;
    GSource         *source     = NULL;
    int             src_id      = -1;

    if(argc != 2) {
        g_print("port has been set as FON_CORE_LISTEN_PORT(0x%X)\n",
                FON_CORE_LISTEN_PORT);
        port = FON_CORE_LISTEN_PORT;
    }
    else {
        port = atoi(argv[1]);
    }

    /**/
    loop    = g_main_loop_new(context, FALSE);
    context = g_main_loop_get_context(loop);
    g_main_context_unref(context);

    /**/
    source = g_timeout_source_new(1000);
    g_source_set_callback(source, callback, NULL, NULL);
    src_id = g_source_attach(source, context);
    if(src_id < 0) {
        perror("g_source_attach");
        exit(EXIT_FAILURE);
    }

    /**/
    if(!fon_init(context, g_type, port)) {
        exit(EXIT_FAILURE);
    }

    attach_table_print(context);
    g_main_loop_run(loop);

    return 0;
}

