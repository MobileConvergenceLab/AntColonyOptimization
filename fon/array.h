/* Very Simple *homogeneous* Array */

#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include "fon_defs.h"

struct _Array
{
    // maximum # of elements
    const int       size;
    // each element size
    const int       el_size;
    // current # of elements
    int             len;
    // reference count
    int             count;
    // real data
    char            data[];
};

typedef struct _Array Array;

#define array_index(A, T, I)       (*(T*)(A->data + I*A->el_size))

FON_BEGIN_EXTERN_C

Array*      array_new               (int size, int el_size, bool init_zero);
void        array_ref               (Array *array);
void        array_unref             (Array *array);
void        array_append            (Array *array, void *data);
void        array_append_vals       (Array *array, void *data, int len);

FON_END_EXTERN_C

#endif /* ARRAY_H */
