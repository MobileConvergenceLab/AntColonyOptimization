#include "aco-table.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "fon/algorithm.h"

#define ACO_TABLE_UNDEFINED_NHOPS       (INT_MAX)

/*==============================================================================
 * Private Type Declarations
 *==============================================================================*/
// Internal AcoValue
typedef struct _RealValue {
    // These members must be aligned in the same order in AcoValue.
    int         target_id;
    int         neigh_id;
    pheromone_t pheromone;
    int         tx_count;
    int         rx_count;
    int         dead_count;
    bool        never_visited;
    int         local_min;

    // Internal Variables
    int             row;        //< row index
    int             col;        //< col index
    int             endurance;
} RealValue;

typedef struct _RealTable {
    // These members must be aligned in the same order in AcoTable.
    const int           host_id;
    const pheromone_t   min;
    const pheromone_t   max;
    const int           max_endurance;

    // Internal Variables
    int                 ref_count;
    int                 nrow;
    int                 ncol;
    int                 col_to_neigh[ACO_TABLE_MAX_COL + 1];
    int                 row_to_dest[ACO_TABLE_MAX_ROW + 1];
    int                 global_min[ACO_TABLE_MAX_ROW];
    RealValue           array[ACO_TABLE_MAX_ROW][ACO_TABLE_MAX_COL];
} RealTable;

/*==============================================================================
 * Private Function Implemenations
 *==============================================================================*/

static inline void init_aco_value(RealValue *value, int target_id, int neigh_id, pheromone_t pheromone, int row, int col, int max_endurance)
{
    value->target_id        = target_id;
    value->neigh_id         = neigh_id;
    value->pheromone        = pheromone;
    value->tx_count         = 0;
    value->rx_count         = 0;
    value->dead_count       = 0;
    value->never_visited    = true;

    value->local_min        = ACO_TABLE_UNDEFINED_NHOPS,
    value->row              = row;
    value->col              = col;
    value->endurance        = max_endurance;
}


static void _fill(int *array, int cap, int value)
{
    for(int i=0; i<cap+1; i++)
    {
        array[i] = value;
    }
}

static int _find_idx(int *array, int cap, int id)
{
    int idx = -1;

    array[cap] = id;
    while(array[++idx] != id);
    array[cap] = -1;

    return idx == cap ? -1 : idx;
}
#define _FIND_COL(table, id)    _find_idx(table->col_to_neigh, ACO_TABLE_MAX_COL, id)
#define _FIND_ROW(table, id)    _find_idx(table->row_to_dest, ACO_TABLE_MAX_ROW, id)

static inline void _set_value(RealTable* table, RealValue *value)
{
    int* local_min = &table->global_min[value->row];

    value->pheromone    = MIN(table->max, MAX(table->min, value->pheromone));
    *local_min          = MIN(*local_min, value->local_min);
    value->endurance    = MAX(value->endurance, 0);
}

static RealValue* _get_value(RealTable* table, int target_id, int neigh_id)
{
    int row = _FIND_ROW(table, target_id);
    int col = _FIND_COL(table, neigh_id);

    if(row == -1 || col == -1)
    {
        return NULL;
    }
    else
    {
        return &table->array[row][col];
    }
}

static void _re_cache_global_min(RealTable* table)
{
    

    for(int row=0; row < table->nrow ; row++)
    {
        int global_min = ACO_TABLE_UNDEFINED_NHOPS;

        for(int col=0; col < table->ncol; col++)
        {
            RealValue* value = &table->array[row][col];
            global_min = MIN(global_min, value->local_min);
        }

        table->global_min[row] = global_min;
    }
}

/*==============================================================================
 * Public Function Implemenations
 *==============================================================================*/
AcoTable* aco_table_new(int host_id, pheromone_t min, pheromone_t max, int max_endurance)
{
    RealTable* table = malloc(sizeof(RealTable));

    *(int*)&table->host_id      = host_id;
    *(pheromone_t*)&table->min  = min;
    *(pheromone_t*)&table->max  = max;
    *(int*)&table->max_endurance= max_endurance;

    table->ref_count = 1;
    table->nrow      = 0;
    table->ncol      = 0;
    _fill(table->col_to_neigh,   sizeof(table->col_to_neigh),     -1);
    _fill(table->row_to_dest,    sizeof(table->row_to_dest),      -1);
    _fill(table->global_min,       sizeof(table->global_min),         ACO_TABLE_UNDEFINED_NHOPS);

    return (AcoTable*)table;
}

AcoTable* aco_table_ref(AcoTable* ftable)
{
    RealTable* table = (RealTable*)ftable;

    table->ref_count++;

    return (AcoTable*)table;
}

void aco_table_unref(AcoTable* ftable)
{
    RealTable* table = (RealTable*)ftable;

    if(--table->ref_count == 0)
    {
        free(table);
    }
}

bool aco_table_add_row(AcoTable* ftable, int target_id)
{
    RealTable* table = (RealTable*)ftable;

    if(target_id == table->host_id            ||
       _FIND_ROW(table, target_id)    != -1   ||
       _FIND_COL(table, target_id)    != -1)
    {
        return false;
    }

    int nrow = ++(table->nrow);
    int row = nrow-1;

    if(nrow == ACO_TABLE_MAX_ROW)
    {
        abort();
    }

    table->row_to_dest[nrow-1] = target_id;

    for(int col=0; col<table->ncol; col++)
    {
        init_aco_value(&table->array[row][col],
                target_id,
                table->col_to_neigh[col],
                table->min,
                row,
                col,
                table->max_endurance);
    }

    return true;
}

bool aco_table_add_col(AcoTable* ftable, int neigh_id)
{
    RealTable* table = (RealTable*)ftable;

    if(neigh_id == table->host_id            ||
       _FIND_ROW(table, neigh_id)    != -1   ||
       _FIND_COL(table, neigh_id)    != -1)
    {
        return false;
    }

    int ncol = ++(table->ncol);
    int col = ncol-1;

    if(ncol == ACO_TABLE_MAX_COL)
    {
        abort();
    }

    table->col_to_neigh[col] = neigh_id;

    for(int row=0; row<table->nrow; row++)
    {
        init_aco_value(&table->array[row][col],
                table->row_to_dest[row],
                neigh_id,
                table->min,
                row,
                col,
                table->max_endurance);
    }

    return true;
}

int aco_table_global_min(AcoTable* ftable, int target_id)
{
    RealTable* table = (RealTable*)ftable;

    int         global_min  = ACO_TABLE_UNDEFINED_NHOPS;
    int         ncol        = table->ncol;
    int         row         = _FIND_ROW(table, target_id);
    int         col;

    if(row == -1)
    {
        goto RETURN;
    }

    if(aco_table_is_neigh(ftable, target_id))
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

bool aco_table_is_neigh(AcoTable* ftable, int id)
{
    RealTable* table = (RealTable*)ftable;

    if(id == table->host_id ||
       _FIND_COL(table, id)    == -1)
    {
        return false;
    }

    return true;
}

bool aco_table_is_target(AcoTable* ftable, int id)
{
    RealTable* table = (RealTable*)ftable;

    if(id == table->host_id ||
       _FIND_ROW(table, id)    == -1)
    {
        return false;
    }

    return true;
}

void aco_table_print_all(AcoTable* ftable)
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
        printf("<%3d>   ", table->row_to_dest[row]);
        for(int col=0; col<table->ncol; col++)
        {
            int local_min = table->array[row][col].local_min;
            local_min = local_min != ACO_TABLE_UNDEFINED_NHOPS ? local_min : -1;
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

bool aco_table_get(AcoTable* ftable, AcoValue *fvalue)
{
    RealTable* table = (RealTable*)ftable;
    RealValue* value = _get_value(table, fvalue->target_id, fvalue->neigh_id);

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

bool aco_table_set(AcoTable* ftable, const AcoValue *fvalue)
{
    RealTable* table = (RealTable*)ftable;
    RealValue* value = _get_value(table, fvalue->target_id, fvalue->neigh_id);

    if(value == NULL)
    {
        return false;
    }
    else
    {
        // Pheromone
        *(AcoValue*)value = *fvalue;
        _set_value(table, value);
    }

    return true;
}

void aco_table_evaporate_all(AcoTable* ftable, pheromone_t remain_rate)
{
    RealTable* table = (RealTable*)ftable;
    RealValue* value = NULL;

    for(int row=0; row<table->nrow; row++)
    {
        for(int col=0; col<table->ncol; col++)
        {
            value = &table->array[row][col];

            if(value->endurance == 0)
            {
                // Clear all column in given row.

                for(int col=0; col<table->ncol; col++)
                {
                    value = &table->array[row][col];
                    value->pheromone        = table->min;
                    value->dead_count       += 1;
                    value->never_visited    = true;
                    value->local_min        = ACO_TABLE_UNDEFINED_NHOPS;
                    value->endurance        = table->max_endurance;

                    // the cached global_min(s) are not valid any more.
                    _re_cache_global_min(table);
                }
                break;
            }
            else
            {
                value->pheromone = MAX(value->pheromone*remain_rate, table->min);
            }
        }
    }
}

bool aco_table_iter_begin(AcoTable* ftable, int target_id, AcoTableIter *iter)
{
    RealTable* table = (RealTable*)ftable;

    if(table->ncol == 0)
    {
        return false;
    }

    int row = _FIND_ROW(table, target_id);
    if(row == -1)
    {
        iter->index = -1;
        return false;
    }

    iter->value = *(AcoValue*)&table->array[row][0];
    iter->index = 0;

    return true;
}

bool aco_table_iter_next(AcoTable* ftable, AcoTableIter *iter)
{
    RealTable* table = (RealTable*)ftable;

    int row = _FIND_ROW(table, iter->value.target_id);;
    int index = iter->index;

    if(row == -1)
    {
        iter->index = -1;
        return false;
    }

    if(index < 0 ||
       index == table->ncol-1)
    {
        iter->index = -1;
        return false;
    }

    index++;
    iter->value = *(AcoValue*)&table->array[row][index];
    iter->index = index;

    return true;
}

int* aco_table_new_neighs(AcoTable* ftable)
{
    RealTable* table = (RealTable*)ftable;

    static const int SIZE = sizeof(int)*ACO_TABLE_MAX_COL;
    int *neighs = malloc(SIZE);

    memcpy(neighs, table->col_to_neigh, SIZE);

    return neighs;
}

int* aco_table_new_dests(AcoTable* ftable)
{
    RealTable* table = (RealTable*)ftable;

    static const int SIZE = sizeof(int)*ACO_TABLE_MAX_ROW;
    int *dests = malloc(SIZE);

    memcpy(dests, table->row_to_dest, SIZE);

    return dests;
}

void aco_table_free_array(int *array)
{
    free(array);

    return;
}

bool aco_table_tx_info_update(AcoTable* ftable, int target_id, int neigh_id)
{
    RealTable* table = (RealTable*)ftable;
    RealValue* value = _get_value(table, target_id, neigh_id);

    if(value != NULL)
    {
        value->tx_count     += 1;
        value->endurance    -= 1;
        _set_value(table, value);

        return true;
    }
    else
    {
        return false;
    }
}

bool aco_table_rx_info_update(AcoTable* ftable, int target_id, int neigh_id, int nhops)
{
    RealTable* table = (RealTable*)ftable;
    RealValue* value = _get_value(table, target_id, neigh_id);

    if(value != NULL)
    {
        // all these values are properly set in aco_table_set()
        value->local_min    = MIN(value->local_min, nhops);
        value->rx_count     += 1;
        value->endurance    = table->max_endurance;
        _set_value(table, value);

        return true;
    }
    else
    {
        return false;
    }
}
