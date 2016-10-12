#include "fon_test.h"

void
fon_test_host_print(FonClient *client) {
    fon_id_t host = -1;
    fon_host_get(client, &host);
    printf("host: %s\n", inet_ntoa(*(struct in_addr*)&host));
}

void
fon_test_table_add(FonClient *client, fon_id_t id)
{
    fib_tuple_t   tuple       = {};

    tuple.target    = id;
    tuple.neighbor  = id;
    tuple.hops      = 1;
    tuple.validation = FIB_TUPLE_VALID;
    fon_table_add(client, &tuple);

    printf("%s has been added\n", inet_ntoa(*(struct in_addr*)&id));
}

void
fon_test_table_print(FonClient *client)
{
    int             len             = -1;
    Array           *tuple_array    = NULL;
    fib_tuple_t     *tuple          = NULL;

    fon_table_get(client, &tuple_array);
    len = tuple_array->len;
    tuple = &array_index(tuple_array, fib_tuple_t  , 0);

    for(int i=0; i<len; i++)
    {
        fib_tuple_print(tuple+i);
    }

    array_unref(tuple_array);
}
