/**
 * Implementation of (Forward Information Base, FIB) Table 
 */
#ifndef TABLE_H
#define TABLE_H

#include <stdbool.h>
#include "fon/table_if.h"

typedef struct _table table;

table*          table_create            (void);
void            table_add               (table *t, table_tuple *tuple);
void            table_del               (table *t, int id);
void            table_get               (table *t, int id, table_tuple **tuple);
GArray*         table_get_all           (table* t);
void            table_print_all         (table *t);

#endif /* TABLE_H */
