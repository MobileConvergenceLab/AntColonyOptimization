#ifndef MAIN_H
#define MAIN_H

#include <glib.h>
#include "aco-table.h"

/**
 * @loop:
 * @context:
 * @table:
 */
typedef struct _AcoObj {
    GMainLoop*      loop;
    GMainContext*   context;
    AcoTable*       table;
} MainObj;

#endif /* MAIN_H */
