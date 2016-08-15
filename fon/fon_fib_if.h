#ifndef FON_FIB_IF_H
#define FON_FIB_IF_H

#include <stdio.h>
#include "fon_types.h"

#define FIB_TUPLE_VALID       (0)
#define FIB_TUPLE_INVALID     (1)

typedef struct _fib_tuple_t
{
    /* primary key */
    fon_id_t        target;
    fon_id_t        neighbor;
    fon_dist_t      hops;
    int             validation;
} fib_tuple_t;

static inline void
fib_tuple_print(const fib_tuple_t* tuple)
{
    printf("%-8d%-8d%-8d\n", tuple->target, tuple->neighbor, tuple->hops);
}

#endif // FON_FIB_IF_H
