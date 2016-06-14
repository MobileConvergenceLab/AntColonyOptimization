#include "aco-table.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "fon/algorithm.h"
#include "aco-parameters.h"

#define ACO_TABLE_UNDEFINED_DIST        (ACO_DIST_MAX)

/*==============================================================================
 * Private Type Declarations
 *==============================================================================*/
// Internal AcoValue
typedef struct _RealValue {
// These members must be aligned in the same order in AcoValue.
    aco_id_t    target;
    aco_id_t    neigh;
    aco_ph_t    pheromone;
    int         tx_count;
    int         rx_count;
    int         dead_count;
    bool        never_visited;
    aco_dist_t  local_min;

// Internal Variables
    int             row;        //< row index
    int             col;        //< col index
    int             endurance;
} RealValue;

typedef struct _RealTable {
// These members must be aligned in the same order in AcoTable.
    const aco_id_t      host;
    const aco_ph_t      min_pheromone;
    const aco_ph_t      max_pheromone;
    const int           max_endurance;

// Internal Variables
    int                 ref_count;
    int                 nrow;
    int                 ncol;
    aco_id_t            col_to_neigh[ACO_TABLE_MAX_COL + 1];
    aco_id_t            row_to_target[ACO_TABLE_MAX_ROW + 1];
    aco_dist_t          global_mins[ACO_TABLE_MAX_ROW];
    RealValue           array[ACO_TABLE_MAX_ROW][ACO_TABLE_MAX_COL];

    // called when aco_table_evaporate_all() is called
    void            (*callee)(AcoTable* table, void* arg);
    void            *callee_arg;
    /// called when table object is destroyed.
    void            (*arg_dtor)(void *arg);
} RealTable;

/*==============================================================================
 * Private Function Implemenations
 *==============================================================================*/

static inline void
aco_value_init(RealValue    *value,
               aco_id_t     target,
               aco_id_t     neigh,
               aco_ph_t     pheromone,
               int          row,
               int          col,
               int          max_endurance)
{
    value->target           = target;
    value->neigh            = neigh;
    value->pheromone        = pheromone;
    value->tx_count         = 0;
    value->rx_count         = 0;
    value->dead_count       = 0;
    value->never_visited    = true;

    value->local_min        = ACO_TABLE_UNDEFINED_DIST,
    value->row              = row;
    value->col              = col;
    value->endurance        = max_endurance;
}

static void
_init_ids(aco_id_t      *idx_to_id,
          int           cap)
{
    for(int idx=0; idx<cap; idx++)
    {
        idx_to_id[idx] = -1;
    }
}

static void
_init_dists(aco_dist_t      *idx_to_dist,
            int             cap)
{
    for(int idx=0; idx<cap; idx++)
    {
        idx_to_dist[idx] = ACO_TABLE_UNDEFINED_DIST;
    }
}

static int
_find_idx(aco_id_t       *idx_to_id,
          int            cap,
          aco_id_t       id)
{
    int idx = -1;

    idx_to_id[cap] = id;
    while(idx_to_id[++idx] != id);
    idx_to_id[cap] = -1;

    return idx == cap ? -1 : idx;
}
#define _FIND_COL(table, id)    _find_idx(table->col_to_neigh, ACO_TABLE_MAX_COL, id)
#define _FIND_ROW(table, id)    _find_idx(table->row_to_target, ACO_TABLE_MAX_ROW, id)

static void
_aco_table_re_cache_global_min(RealTable *table, int row)
{
    int global_min = ACO_TABLE_UNDEFINED_DIST;

    for(int col=0; col < table->ncol; col++)
    {
        RealValue* value = &table->array[row][col];
        global_min = MIN(global_min, value->local_min);
    }

    table->global_mins[row] = global_min;
}

// value값을 변경하고 나면 반드시 호출되어야 한다.
static inline void
_aco_value_set(RealTable    *table,
               RealValue    *value,
               bool         local_min_changed)   /* if value->local_min is changed,
                                                    this flag must be ture */
{
    if(local_min_changed)
    {
        _aco_table_re_cache_global_min(table, value->row);
    }
    value->pheromone    = MIN(table->max_pheromone, MAX(table->min_pheromone, value->pheromone));
    value->endurance    = MAX(value->endurance, 0);

}

static RealValue*
_aco_value_get(RealTable    *table,
               aco_id_t     target,
               int          neigh)
{
    int row = _FIND_ROW(table, target);
    int col = _FIND_COL(table, neigh);

    if(row == -1 || col == -1)
    {
        return NULL;
    }
    else
    {
        return &table->array[row][col];
    }
}

/*==============================================================================
 * Public Function Implemenations
 *==============================================================================*/
AcoTable*
aco_table_new(aco_id_t      host,
              aco_ph_t      min,
              aco_ph_t      max,
              int           max_endurance)
{
    RealTable* table = malloc(sizeof(RealTable));

    *(aco_id_t*)&table->host      = host;
    *(aco_ph_t*)&table->min_pheromone  = min;
    *(aco_ph_t*)&table->max_pheromone  = max;
    *(int*)&table->max_endurance= max_endurance;

    table->ref_count = 1;
    table->nrow      = 0;
    table->ncol      = 0;
    _init_ids(table->col_to_neigh,      sizeof(table->col_to_neigh));
    _init_ids(table->row_to_target,     sizeof(table->row_to_target));
    _init_dists(table->global_mins,     sizeof(table->global_mins));

    table->callee       = NULL;
    table->callee_arg   = NULL;
    table->arg_dtor     = NULL;

    return (AcoTable*)table;
}

AcoTable*
aco_table_ref(AcoTable      *ftable)
{
    RealTable* table = (RealTable*)ftable;

    table->ref_count++;

    return (AcoTable*)table;
}

static void
aco_table_dtor(RealTable        *table)
{
    table->arg_dtor(table->callee_arg);
    free(table);
}

void
aco_table_unref(AcoTable        *ftable)
{
    RealTable* table = (RealTable*)ftable;

    if(--table->ref_count == 0)
    {
        aco_table_dtor(table);
    }
}

bool
aco_table_add_target(AcoTable      *ftable,
                     aco_id_t      target)
{
    RealTable* table = (RealTable*)ftable;

    if(target == table->host            ||
       _FIND_ROW(table, target)    != -1   ||
       _FIND_COL(table, target)    != -1)
    {
        return false;
    }

    int nrow = ++(table->nrow);
    int row = nrow-1;

    if(nrow == ACO_TABLE_MAX_ROW)
    {
        abort();
    }

    table->row_to_target[nrow-1] = target;

    for(int col=0; col<table->ncol; col++)
    {
        aco_value_init(&table->array[row][col],
                target,
                table->col_to_neigh[col],
                table->min_pheromone,
                row,
                col,
                table->max_endurance);
    }

    return true;
}

bool
aco_table_add_neigh(AcoTable      *ftable,
                    aco_id_t      neigh)
{
    RealTable* table = (RealTable*)ftable;

    if(neigh == table->host            ||
       _FIND_ROW(table, neigh)    != -1   ||
       _FIND_COL(table, neigh)    != -1)
    {
        return false;
    }

    int ncol = ++(table->ncol);
    int col = ncol-1;

    if(ncol == ACO_TABLE_MAX_COL)
    {
        abort();
    }

    table->col_to_neigh[col] = neigh;

    for(int row=0; row<table->nrow; row++)
    {
        aco_value_init(&table->array[row][col],
                table->row_to_target[row],
                neigh,
                table->min_pheromone,
                row,
                col,
                table->max_endurance);
    }

    return true;
}

int
aco_table_global_min(AcoTable       *ftable,
                     aco_id_t       target)
{
    RealTable* table = (RealTable*)ftable;

    int         global_min  = ACO_TABLE_UNDEFINED_DIST;
    int         ncol        = table->ncol;
    int         row         = _FIND_ROW(table, target);
    int         col         = -1;

    if(row == -1)
    {
        goto RETURN;
    }

    if(aco_table_is_neigh(ftable, target))
    {
        global_min = 1;
        goto RETURN;
    }

    for(col=0; col< ncol; col++)
    {
        global_min = MIN(global_min, table->array[row][col].local_min);
    }

RETURN:
    return global_min;
}

bool
aco_table_is_neigh(AcoTable     *ftable,
                   aco_id_t     id)
{
    RealTable* table = (RealTable*)ftable;

    if(id == table->host ||
       _FIND_COL(table, id)    == -1)
    {
        return false;
    }

    return true;
}

bool
aco_table_is_target(AcoTable        *ftable,
                    aco_id_t        id)
{
    RealTable* table = (RealTable*)ftable;

    if(id == table->host ||
       _FIND_ROW(table, id)    == -1)
    {
        return false;
    }

    return true;
}

void
aco_table_print_all(AcoTable        *ftable)
{
    RealTable* table = (RealTable*)ftable;


    printf("Print Table(Pheromon, TX count, RX count, MIN hops)\n");
    // print neighbors
    printf("        ");
    for(int col=0; col<table->ncol; col++)
    {
        printf("<%3d>                 ", table->col_to_neigh[col]);
    }
    printf("\n");

    // print destinations
    for(int row=0; row<table->nrow; row++)
    {
        printf("<%3d>   ", table->row_to_target[row]);
        for(int col=0; col<table->ncol; col++)
        {
            int local_min = table->array[row][col].local_min;
            local_min = local_min != ACO_TABLE_UNDEFINED_DIST ? local_min : -1;
            printf("%05.2f %4d %4d  %2d   ",
                    table->array[row][col].pheromone,
                    table->array[row][col].tx_count,
                    table->array[row][col].rx_count,
                    local_min);
        }
        printf("\n");
    }
    printf("\n");
}

bool
aco_table_get(AcoTable      *ftable,
              AcoValue      *fvalue)
{
    RealTable* table = (RealTable*)ftable;
    RealValue* value = _aco_value_get(table, fvalue->target, fvalue->neigh);

    if(value == NULL)
    {
        return false;
    }
    else
    {
        *fvalue = *(AcoValue*)value;

        return true;
    }

}

bool
aco_table_set(AcoTable          *ftable,
              const AcoValue    *fvalue)
{
    RealTable* table = (RealTable*)ftable;
    RealValue* value = _aco_value_get(table, fvalue->target, fvalue->neigh);

    if(value == NULL)
    {
        return false;
    }
    else
    {
        // Pheromone
        *(AcoValue*)value = *fvalue;
        _aco_value_set(table, value, true);
    }

    return true;
}

void
aco_table_evaporate_all(AcoTable        *ftable,
                        aco_ph_t        remain_rate)
{
    RealTable* table = (RealTable*)ftable;
    RealValue* value = NULL;

    for(int row=0; row<table->nrow; row++)
    {
        for(int col=0; col<table->ncol; col++)
        {
            value = &table->array[row][col];

            if(value->endurance <= 0)
            {
                // Clear all column in given row.

                for(int col=0; col<table->ncol; col++)
                {
                    value = &table->array[row][col];
                    value->pheromone        = table->min_pheromone;
                    value->dead_count       += 1;
                    value->never_visited    = true;
                    value->local_min        = ACO_TABLE_UNDEFINED_DIST;
                    value->endurance        = table->max_endurance;

                    _aco_value_set(table, value, false/* for lazy evaluation */);
                }
                // the cached global_min(s) are not valid any more.
                _aco_table_re_cache_global_min(table, row);
                break;
            }
            else
            {
                value->pheromone = MAX(value->pheromone*remain_rate, table->min_pheromone);
            }
        }
    }

    if(table->callee != NULL)
    {
        table->callee((AcoTable*)table, table->callee_arg);
    }
}

AcoTableIter
aco_table_iter_begin(AcoTable       *ftable,
                     aco_id_t       target)
{
    RealTable       *table  = (RealTable*)ftable;
    AcoTableIter    iter    = {.value = {0},
                               .index = -1,
                               .valid = false};

    if(table->ncol == 0)
    {
        goto RETURN;
    }

    int row = _FIND_ROW(table, target);
    if(row == -1)
    {
        goto RETURN;
    }

    iter.value = *(AcoValue*)&table->array[row][0];
    *(int*)&iter.index = 0;
    *(bool*)&iter.valid = true;

RETURN:
    return iter;
}


bool
aco_table_iter_next(AcoTable        *ftable,
                    AcoTableIter    *iter)
{
    if(!iter->valid)
    {
        return false;
    }

    RealTable* table = (RealTable*)ftable;

    int row = _FIND_ROW(table, iter->value.target);;
    int index = iter->index;

    if(row == -1)
    {
        *(int*)&iter->index = -1;
        *(bool*)&iter->valid = false;
        return false;
    }

    if(index < 0 ||
       index == table->ncol-1)
    {
        *(int*)&iter->index = -1;
        *(bool*)&iter->valid = false;
        return false;
    }

    index++;
    iter->value = *(AcoValue*)&table->array[row][index];
    *(int*)&iter->index = index;

    return true;
}

aco_ids_t
aco_table_new_neighs(AcoTable       *ftable)
{
    RealTable           *table  = (RealTable*)ftable;
    static const int    SIZE    = sizeof(table->col_to_neigh);
    aco_ids_t           neighs  = malloc(SIZE);

    memcpy(neighs, table->col_to_neigh, SIZE);

    return neighs;
}

aco_ids_t
aco_table_new_targets(AcoTable      *ftable)
{
    RealTable           *table  = (RealTable*)ftable;
    static const int    SIZE    = sizeof(table->row_to_target);
    aco_ids_t           targets = malloc(SIZE);

    memcpy(targets, table->row_to_target, SIZE);

    return targets;
}

void
aco_table_free_ids(aco_ids_t     ids)
{
    free(ids);

    return;
}

bool
aco_table_tx_info_update(AcoTable       *ftable,
                         aco_id_t       target,
                         aco_id_t       neigh)
{
    RealTable* table = (RealTable*)ftable;
    RealValue* value = _aco_value_get(table, target, neigh);

    if(value != NULL)
    {
        value->tx_count     += 1;

#if ENDURANCE_ENABLE
#pragma message("Enable Enduracne")
        value->endurance    -= 1;
#else
#pragma message("Disable Enduracne")
        value->endurance    = 1;
#endif /* ENDURANCE_ENABLE */

        _aco_value_set(table, value, false);

        return true;
    }
    else
    {
        return false;
    }
}

bool
aco_table_rx_info_update(AcoTable       *ftable,
                         aco_id_t       target,
                         aco_id_t       neigh,
                         aco_dist_t     dist)
{
    RealTable* table = (RealTable*)ftable;
    RealValue* value = _aco_value_get(table, target, neigh);

    if(value != NULL)
    {
        // all these values are properly set in aco_table_set()
        value->local_min    = MIN(value->local_min, dist);
        value->rx_count     += 1;
        value->endurance    = table->max_endurance;
        _aco_value_set(table, value, true);

        return true;
    }
    else
    {
        return false;
    }
}

void
aco_table_register_callee(AcoTable       *ftable,
                          void            (*callee)(AcoTable* table, void* arg),
                          void            *callee_arg,
                          void            (*arg_dtor)(void *arg))
{
    RealTable* table = (RealTable*)ftable;

    if(table->arg_dtor != NULL)
    {
        table->arg_dtor(table->callee_arg);
    }

    table->callee           = callee;
    table->callee_arg       = callee_arg;
    table->arg_dtor         = arg_dtor;
}
