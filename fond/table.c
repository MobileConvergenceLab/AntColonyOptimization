#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "table.h"
#include "fon/packet_if.h"
#include "fon/fon-utils.h"

struct _table {
    GTree            *m_table;
};

/*==============================================================================
 * static function declaration
 *=============================================================================*/
static gint     key_compare_func        (gconstpointer a, gconstpointer b, gpointer user_data);
static GTree*   g_tree_table            ();
static gboolean traverse_print_all      (int *id, table_tuple *tuple, gpointer unused);
static gboolean traverse_get_all        (int *id, table_tuple *tuple, GArray *arr);

/*==============================================================================
 * static function definition
 *=============================================================================*/

#define TABLE_ADD(t, vvalue)       g_tree_replace(t->m_table, &vvalue->id, vvalue)
#define TABLE_GET(t, iid)          ((table_tuple*)g_tree_lookup(t->m_table, iid))
#define TABLE_DEL(t, iid)          g_tree_remove(t->m_table, iid)

static gint key_compare_func(gconstpointer a, gconstpointer b, gpointer user_data)
{
    return *(int *)(a) - *(int *)(b);
}

static GTree* g_tree_table()
{
    return g_tree_new_full(key_compare_func, NULL, NULL, g_free);
}

static gboolean
traverse_print_all (int *id,
               table_tuple *tuple,
               gpointer unused)
{
    printf("ID:%-8d HOPS:%-8d NEIGH_ID:%-8d\n", tuple->id,tuple->hops,tuple->neigh_id);
    return 0;
}

static gboolean
traverse_get_all (int *id, table_tuple *tuple, GArray *arr)
{
    g_array_append_val(arr, *tuple);
    return FALSE;
}

/*==============================================================================
 * public function definition
 *=============================================================================*/

table* table_create()
{
    dbg("Called");
    table* t = (table*)g_try_malloc0(sizeof(table));
    t->m_table = g_tree_table();

    dbg("Done");

    return t;
}

void table_add(table* t, table_tuple *tuple)
{
    TABLE_ADD(t, tuple);
    return;
}

void table_del(table* t, int id)
{
    TABLE_DEL(t, &id);

    return;
}

void table_get(table* t, int id, table_tuple **tuple)
{
    table_tuple *target;

    target = TABLE_GET(t, &id);
    if(target == NULL) {
        *tuple = NULL;
        return;
    }

    *tuple = g_new0(table_tuple, 1);
    **tuple = *target;

    return;
}

GArray* table_get_all(table* t)
{
    GArray *arr = TUPLE_GARRAY_NEW();
    g_tree_foreach(t->m_table, (GTraverseFunc)traverse_get_all, arr);

    return arr;
}

void table_print_all(table *t)
{
    g_tree_foreach(t->m_table, (GTraverseFunc)traverse_print_all, NULL);
}





