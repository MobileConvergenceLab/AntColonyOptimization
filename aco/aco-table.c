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
typedef struct _IAcoValue {
    AcoValue        value;
    int             row;        //< row index
    int             col;        //< col index
} IAcoValue;

typedef struct _RealTable {
    const int           host_id;
    const pheromone_t   min;
    const pheromone_t   max;
    int                 ref_count;
    int                 nrow;
    int                 ncol;
    int                 col_to_neigh[ACO_TABLE_MAX_COL + 1];
    int                 row_to_dest[ACO_TABLE_MAX_ROW + 1];
    int                 min_hops[ACO_TABLE_MAX_ROW];
    IAcoValue           array[ACO_TABLE_MAX_ROW][ACO_TABLE_MAX_COL];
} RealTable;

/*==============================================================================
 * Private Data
 *==============================================================================*/
static const AcoValue DEFAULT_ACO_VALUE =
{
    .target_id    = -1,
    .neigh_id   = -1,
    .tx_count   = 0,
    .rx_count   = 0,
    .min_hops   = ACO_TABLE_UNDEFINED_NHOPS,
    .pheromone  = 0
};

/*==============================================================================
 * Private Function Implemenations
 *==============================================================================*/

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

static void _set_value(RealTable* table, IAcoValue *ivalue)
{
    int* min_hops = &table->min_hops[ivalue->row];
    ivalue->value.pheromone = MIN(table->max, MAX(table->min, ivalue->value.pheromone));
    *min_hops = MIN(*min_hops, ivalue->value.min_hops);
}

/*==============================================================================
 * Public Function Implemenations
 *==============================================================================*/
AcoTable* aco_table_new(int host_id, pheromone_t min, pheromone_t max)
{
    RealTable* table = malloc(sizeof(RealTable));

    *(int*)&table->host_id      = host_id;
    *(pheromone_t*)&table->min  = min;
    *(pheromone_t*)&table->max  = max;

    table->ref_count = 1;
    table->nrow      = 0;
    table->ncol      = 0;
    _fill(table->col_to_neigh,   sizeof(table->col_to_neigh),     -1);
    _fill(table->row_to_dest,    sizeof(table->row_to_dest),      -1);
    _fill(table->min_hops,       sizeof(table->min_hops),         ACO_TABLE_UNDEFINED_NHOPS);

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
        table->array[row][col].value             = DEFAULT_ACO_VALUE;
        table->array[row][col].value.target_id   = target_id;
        table->array[row][col].value.neigh_id    = table->col_to_neigh[col];
        table->array[row][col].value.pheromone   = table->min;
        table->array[row][col].row               = row;
        table->array[row][col].col               = col;
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
        table->array[row][col].value           = DEFAULT_ACO_VALUE;
        table->array[row][col].value.target_id   = table->row_to_dest[row];
        table->array[row][col].value.neigh_id  = neigh_id;
        table->array[row][col].value.pheromone = table->min;
        table->array[row][col].row             = row;
        table->array[row][col].col             = col;
    }

    return true;
}

int aco_table_min_hops(AcoTable* ftable, int target_id)
{
    RealTable* table = (RealTable*)ftable;

    int         min_hops    = ACO_TABLE_UNDEFINED_NHOPS;
    int         ncol        = table->ncol;
    int         row         = _FIND_ROW(table, target_id);
    int         hops;
    int         col;
    AcoValue*   value;

    if(row == -1)
    {
        return ACO_TABLE_UNDEFINED_NHOPS;
    }

    if(aco_table_is_neigh(ftable, target_id))
    {
        return 1;
    }

    for(col=0; col< ncol; col++)
    {
        value = &table->array[row][col].value;

        hops = value->min_hops;
        
        if(min_hops > hops)
        {
            min_hops = hops;
        }
    }

    return min_hops;
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

bool aco_table_is_dest(AcoTable* ftable, int id)
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
            int min_hops = table->array[row][col].value.min_hops;
            min_hops = min_hops != ACO_TABLE_UNDEFINED_NHOPS ? min_hops : -1;
            printf("%05.2f %4d %4d  %2d   ",
                    table->array[row][col].value.pheromone,
                    table->array[row][col].value.tx_count,
                    table->array[row][col].value.rx_count,
                    min_hops);
        }
        printf("\n");
    }
    printf("\n");
}

bool aco_table_get(AcoTable* ftable, AcoValue *value)
{
    RealTable* table = (RealTable*)ftable;

    int row = _FIND_ROW(table, value->target_id);
    int col = _FIND_COL(table, value->neigh_id);

    if(row == -1 || col == -1)
    {
        return false;
    }

    *value = table->array[row][col].value;

    return true;
}

bool aco_table_set(AcoTable* ftable, const AcoValue *value)
{
    RealTable* table = (RealTable*)ftable;

    int row = _FIND_ROW(table, value->target_id);
    int col = _FIND_COL(table, value->neigh_id);

    if(row == -1 || col == -1)
    {
        return false;
    }

    // Pheromone
    table->array[row][col].value = *value;
    _set_value(table, table->array[row]+col);

    return true;
}

void aco_table_evaporate_all(AcoTable* ftable, pheromone_t remain_rate)
{
    RealTable* table = (RealTable*)ftable;

    for(int row=0; row<table->nrow; row++)
    {
        for(int col=0; col<table->ncol; col++)
        {
            table->array[row][col].value.pheromone *= remain_rate;
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

    iter->value = table->array[row][0].value;
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
    iter->value = table->array[row][index].value;
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

