#ifndef AcoTable_H
#define AcoTable_H

#include "aco-types.h"

#ifdef __cpluscplus
extern "C" {
#endif

#define ACO_TABLE_MAX_ROW                   (512)
#define ACO_TABLE_MAX_COL                   (16)

typedef struct _AcoValue {
    aco_id_t    target;
    aco_id_t    neigh;
    aco_ph_t    pheromone;
    int         tx_count;       // 해당 인접노드로 패킷을 보낸 개수
    int         rx_count;       // 해당 인접노드에서 패킷을 수신한 개수
    int         dead_count;     // 해당 인접노드의 링크가 죽은 횟수
    bool        never_visited;  // 해당 인접노드로 적어도 한번 패킷을 보냈는지 여부. 링크가 죽을 때 마다 초기화된다.
    aco_dist_t  local_min;      // 해당 노드까지의 최소 거리
} AcoValue;

typedef struct _AcoTableIter {
    AcoValue        value;
    const int       index;
    const bool      valid;
} AcoTableIter;

typedef struct _AcoTable {
    const aco_id_t      host;
    const aco_ph_t      min;
    const aco_ph_t      max;
    const int           max_endurance;

    // For internal Varialbles
    const char          data[];
} AcoTable;

AcoTable*       aco_table_new               (aco_id_t       host,
                                             aco_ph_t       min,
                                             aco_ph_t       max,
                                             int            max_endurance);
AcoTable*       aco_table_ref               (AcoTable       *table);
void            aco_table_unref             (AcoTable       *table);
bool            aco_table_add_row           (AcoTable       *table,
                                             aco_id_t       target);
bool            aco_table_add_col           (AcoTable       *table,
                                             aco_id_t       neigh);
bool            aco_table_is_target         (AcoTable       *table,
                                             aco_id_t       target);
bool            aco_table_is_neigh          (AcoTable       *table,
                                             aco_id_t       neigh);
void            aco_table_print_all         (AcoTable       *table);
bool            aco_table_get               (AcoTable       *table,
                                             AcoValue       *value);
bool            aco_table_set               (AcoTable       *table,
                                             const AcoValue *value);
void            aco_table_evaporate_all     (AcoTable       *table,
                                             aco_ph_t       remain_rate);
AcoTableIter    aco_table_iter_begin        (AcoTable       *table,
                                             aco_id_t       target);
bool            aco_table_iter_next         (AcoTable       *table,
                                             AcoTableIter   *iter);
aco_ids_t       aco_table_new_neighs        (AcoTable       *table);
aco_ids_t       aco_table_new_targets       (AcoTable       *table);
void            aco_table_free_ids          (aco_ids_t      ids);
int             aco_table_global_min        (AcoTable       *table,
                                             aco_id_t       target);
bool            aco_table_tx_info_update    (AcoTable       *table,
                                             aco_id_t       target,
                                             aco_id_t       neigh);
bool            aco_table_rx_info_update    (AcoTable       *table,
                                             aco_id_t       target,
                                             aco_id_t       neigh,
                                             aco_dist_t     dist);

#ifdef __cpluscplus
} // extern "C"
#endif

#endif /* AcoTable_H */
