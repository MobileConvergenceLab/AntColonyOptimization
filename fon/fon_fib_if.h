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

#ifdef __cplusplus
extern "C" {
#endif

static inline void
fib_tuple_print(const fib_tuple_t* tuple)
{
    printf("%s%-8d%-8d\n", inet_ntoa(*(struct in_addr*)&tuple->target), tuple->neighbor, tuple->hops);
}

#ifdef __cplusplus
}
#endif

#endif // FON_FIB_IF_H
