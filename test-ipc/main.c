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
    g_print("%12d%12d%12d\n", tuple->id, tuple->hops, tuple->neigh_id);
}

static void test_host_get() {
    int host;
    fon_host_get(&host);
    g_print("host: %d\n", host);
}

static void test_table() {

    table_tuple tuple       = {};
    GTupleArray *tuple_arry = NULL;
    int         i;

    tuple.id        = 3;
    tuple.hops      = 1;
    tuple.neigh_id  = 3;
    fon_table_add(&tuple);

    tuple.id        = 4;
    tuple.hops      = 1;
    tuple.neigh_id  = 4;
    fon_table_add(&tuple);

    tuple.id        = 5;
    tuple.hops      = 1;
    tuple.neigh_id  = 5;
    fon_table_add(&tuple);

    fon_table_get(&tuple_arry);

    g_print("tuple_arry->len: %d\n", tuple_arry->len);
    g_print("g_array_get_element_size(): %d\n", g_array_get_element_size(tuple_arry));
    for(i=0; i<tuple_arry->len; i++) {
        print_tuple(&TUPLE_GARRAY_INDEX(tuple_arry, i));
    }
}

int main(int argc, char **argv)
{
    int         type        = 33;
    int         port        = -1;
    GMainContext *context = g_main_context_new();

    if(argc != 2) {
        g_print("port has been set as FON_CORE_LISTEN_PORT(0x%X)\n", FON_CORE_LISTEN_PORT);
        port = FON_CORE_LISTEN_PORT;
    }
    else {
        port = atoi(argv[1]);
    }


    if(!fon_init(context, type, port)) {
        exit(EXIT_FAILURE);
    }

    test_host_get();
    test_table();

    return 0;
}

