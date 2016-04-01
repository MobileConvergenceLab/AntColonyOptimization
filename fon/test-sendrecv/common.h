#ifndef COMMON_H
#define COMMON_H

#include "fon/fon.h"

#define FON_FUNC_TYPE   (0x0008)
#define SNDER_ID        (1)
#define RCVER_ID        (2)

/* GSourceFunc */
static gboolean table_print(gpointer data) {
    GTupleArray *tuple_array;
    int         i;

    fon_table_get(&tuple_array);

    for(i=0;i<tuple_array->len; i++) {
        g_print("%-8d%-8d%-8d\n", g_array_index(tuple_array, table_tuple, i).id, g_array_index(tuple_array, table_tuple, i).hops, g_array_index(tuple_array, table_tuple, i).neigh_id);
    }

    return TRUE;
}

static void attach_table_print(GMainContext *context) {
    g_timeout_add_seconds (1000,
                           (GSourceFunc)table_print,
                            NULL);

    return;
}

#endif /* COMMON_H */
