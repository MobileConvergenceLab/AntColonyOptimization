/*
 * neighbor node implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <assert.h>
#include "node.h"
#include "table.h"
#include "fon/fon-utils.h"

/*==============================================================================
 * static data definition
 *=============================================================================*/
struct _NodePool {
    GTree           *nodes;
    table           *table;
};

/*==============================================================================
 * static function declaration
 *=============================================================================*/
static gint
key_compare_func(gconstpointer a, gconstpointer b, gpointer user_data);

static GTree*
g_tree_nodes();

/*==============================================================================
 * static function definition
 *=============================================================================*/
#define NODES_ADD(pool, vvalue)       g_tree_replace(pool->nodes, &vvalue->id, vvalue)
#define NODES_GET(pool, iid)          ((NearNode*)g_tree_lookup(pool->nodes, iid))
#define NODES_DEL(pool, iid)          g_tree_remove(pool->nodes, iid)

static gint key_compare_func(gconstpointer a, gconstpointer b, gpointer user_data)
{
    return *(int *)(a) - *(int *)(b);
}

static GTree* g_tree_nodes()
{
    return g_tree_new_full(key_compare_func, NULL, NULL, g_free); 
}

/*==============================================================================
 * public function definition
 *=============================================================================*/
NodePool* node_pool_create(table *t)
{
    dbg("Called");
    NodePool *pool = g_new0(NodePool, 1);;
    pool->nodes = g_tree_nodes();
    pool->table = t;
    dbg("Done");
    return pool;
}

void node_pool_add(NodePool *pool, NearNode *nnode)
{
    NODES_ADD(pool, nnode);
    table_tuple *tuple = g_new0(table_tuple, 1);
    tuple->id = nnode->id;
    tuple->hops = HOPS_NEIGH;
    tuple->neigh_id = nnode->id;
    table_add(pool->table, tuple);
    return;
}

void node_pool_del(NodePool *pool, int id)
{
    table_del(pool->table, id);
    NODES_DEL(pool, &id);
    return;
}

NearNode* node_pool_lookup(NodePool *pool, int id)
{
    return NODES_GET(pool, &id);
}

