#include <stdlib.h>
#include <string.h>

#include "array.h"

Array*
array_new(int size, int el_size, bool init_zero)
{
    int data_size = size * el_size;
    int tot_size = sizeof(Array) + data_size;

    Array *array = malloc(tot_size);

    if(array == NULL)
    {
        exit(EXIT_FAILURE);
    }

    *(int*)&array->size     = size;
    *(int*)&array->el_size  = el_size;
    array->len = 0;
    array->count = 1;

    if(init_zero)
    {
        memset(array->data, 0, sizeof(data_size));
    }

    return array;
}

void
array_ref(Array *array)
{
    array->count++;
}

void
array_unref(Array *array)
{
    if(!--array->count)
    {
        free(array);
    }
}

void
array_append(Array *array, void *data)
{
    int el_size = array->el_size;
    memcpy(array->data + array->len*el_size, data, el_size);
    ++array->len;
}

void
array_append_vals(Array *array, void *data, int len)
{
    int el_size = array->el_size;
    int bytes = el_size*len;

    memcpy(array->data + array->len*el_size,
           data,
           bytes);

    array->len += len;
}
