#ifndef AcoTable_H
#define AcoTable_H

#include "ant-type.h"

#define ACO_TABLE_MAX_ROW               (512)
#define ACO_TABLE_MAX_COL               (16)

typedef struct _AcoValue {
    int         target_id;
    int         neigh_id;
    pheromone_t pheromone;
    int         tx_count;       // 해당 인접노드로 패킷을 보낸 개수
    int         rx_count;       // 해당 인접노드에서 패킷을 수신한 개수
    int         min_hops;       // 해당 노드까지의 최소 거리
} AcoValue;

typedef struct _AcoTableIter {
    AcoValue        value;
    int             index;
} AcoTableIter;

typedef struct _AcoTable {
    const int           host_id;
    const pheromone_t   min;
    const pheromone_t   max;
    char                data[];
} AcoTable;

AcoTable* aco_table_new             (int host_id, pheromone_t min, pheromone_t max);
AcoTable* aco_table_ref             (AcoTable* table);
void aco_table_unref                (AcoTable* table);
bool aco_table_add_row              (AcoTable* table, int target_id);
bool aco_table_add_col              (AcoTable* table, int neigh_id);
bool aco_table_is_dest              (AcoTable* table, int id);
bool aco_table_is_neigh             (AcoTable* table, int id);
void aco_table_print_all            (AcoTable* table);
bool aco_table_get                  (AcoTable* table, AcoValue *value);
bool aco_table_set                  (AcoTable* table, const AcoValue *value);
void aco_table_evaporate_all        (AcoTable* table, pheromone_t remain_rate);
bool aco_table_iter_begin           (AcoTable* table, int target_id, AcoTableIter *iter);
bool aco_table_iter_next            (AcoTable* table, AcoTableIter *iter);
int* aco_table_new_neighs           (AcoTable* table);
int* aco_table_new_dests            (AcoTable* table);
void aco_table_free_array           (int *array);
int aco_table_min_hops              (AcoTable* table, int target_id);
bool aco_table_tx_info_update       (AcoTable* table, int target_id, int neigh_id);
bool aco_table_rx_info_update       (AcoTable* table, int target_id, int neigh_id, int nhops);

#endif /* AcoTable_H */
