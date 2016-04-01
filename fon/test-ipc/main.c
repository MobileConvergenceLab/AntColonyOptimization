/**
 * client-daemon 간 IPC 테스트를 위해 작성한 코드.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#include "fon/fon.h"

#define LISTEN_MAX              (10)

static void print_tuple(table_tuple *tuple) {
    printf("%12d%12d%12d\n", tuple->id, tuple->hops, tuple->neigh_id);
}

static void test_host_get() {
    int host;
    fon_host_get(&host);
    printf("host: %d\n", host);
}

static void test_table_add(int id)
{
    table_tuple tuple       = {};

    tuple.id        = id;
    tuple.hops      = 1;
    tuple.neigh_id  = id;
    fon_table_add(&tuple);

    printf("%d has been added\n", id);
}

static void test_table_adds() {
    test_table_add(3);
    test_table_add(4);
    test_table_add(5);
}

static void test_table_print()
{
    int             len             = -1;
    GArray          *tuple_array    = NULL;
    table_tuple     *tuple          = NULL;

    /* wait for the local daemon to make up the forwarding table.
    During this time, the daemon communicates with neighbor daemons. */
    sleep(2);

    fon_table_get(&tuple_array);
    len = tuple_array->len;
    tuple = &g_array_index(tuple_array, table_tuple, 0);

    for(int i=0; i<len; i++)
    {
        print_tuple(tuple+i);
    }

    g_array_unref(tuple_array);
}

int main(int argc, char **argv)
{
    int         type        = 33;
    int         port        = -1;
    GMainContext *context = g_main_context_new();

    if(argc != 2) {
        printf("port has been set as FON_CORE_LISTEN_PORT(0x%X)\n", FON_CORE_LISTEN_PORT);
        port = FON_CORE_LISTEN_PORT;
    }
    else {
        port = atoi(argv[1]);
    }


    if(!fon_init(context, type, port)) {
        exit(EXIT_FAILURE);
    }

    test_host_get();
    test_table_adds();
    test_table_print();

    return 0;
}

