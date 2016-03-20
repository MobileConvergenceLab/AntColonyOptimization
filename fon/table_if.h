#ifndef TABLE_IF_H
#define TABLE_IF_H

#include <glib.h>

enum {
    HOPS_INVALID    = -1,
    HOPS_LOOP       = 0,
    HOPS_NEIGH      = 1,
};

typedef struct _table_tuple
{
    int             id;         /* primary key */
    int             hops;
    int             neigh_id;
} table_tuple;

static inline void
table_tuple_print(table_tuple* tuple) {
    g_print("%-8d%-8d%-8d\n", tuple->id, tuple->hops, tuple->neigh_id);
}

#define TUPLE_GARRAY_NEW()         (g_array_new (FALSE, TRUE, sizeof(table_tuple)))
#define TUPLE_GARRAY_INDEX(arr, i) (g_array_index((arr), table_tuple, (i)))
#define TUPLE_GARRAY_UNREF(arr)    (g_array_unref((arr)))

#endif /* TABLE_IF_H */
