#ifndef AcoTable_H
#define AcoTable_H

#include "aco-types.h"

#ifdef __cpluscplus
extern "C" {
#endif

// The maximum # of ROW(=Target)
#define ACO_TABLE_MAX_ROW                   (512)

// The maximum # of COL(=Neighbor)
#define ACO_TABLE_MAX_COL                   (8)

typedef struct _AcoValue {
    // primary key
    aco_id_t    target;
    aco_id_t    neigh;
    aco_ph_t    pheromone;

    // 해당 인접노드로 패킷을 보낸 개수
    int         tx_count;

    // 해당 인접노드에서 패킷을 수신한 개수
    int         rx_count;

    // 해당 인접노드의 링크가 죽은 횟수
    int         dead_count;

    // 해당 인접노드로 적어도 한번 패킷을 보냈는지 여부. 링크가 죽을 때 마다 초기화(false)된다.
    bool        never_visited;

    // 해당 노드까지의 최소 거리
    aco_dist_t  local_min;
} AcoValue;

// If target is given, Iterate all neighbors.
typedef struct _AcoTableIter {
    AcoValue        value;

/* XXX: DO NOT DIRECTLY MANIPULATE THESE MEMBERS.
        OR UNDEFINED BEHAVIOR WILL OCCUR. */

    // correspond to neighbor.
    int             col;

    // iterator's valid flag.
    bool            valid;
} AcoTableIter;

typedef struct _AcoTable {
    const aco_id_t      host;
    const aco_ph_t      min;
    const aco_ph_t      max;
    const int           max_endurance;

// For internal Varialbles
/* XXX: DO NOT DIRECTLY MANIPULATE THESE MEMBERS.
        OR UNDEFINED BEHAVIOR WILL OCCUR. */
    const char          data[];
} AcoTable;

// called when aco_table_evaporate_all() is called
typedef void*   evapor_cbarg_t;
// the argument to be passed to evapor_cbfunc
typedef void 	(*evapor_cbfunc_t)(AcoTable* table, evapor_cbarg_t arg);
// destroyer of evapor_cbarg
typedef void    (*evapor_dtor_t)(evapor_cbarg_t arg);

AcoTable*       aco_table_new               (aco_id_t       host,
                                             aco_ph_t       min,
                                             aco_ph_t       max,
                                             int            max_endurance);
AcoTable*       aco_table_ref               (AcoTable       *table);
void            aco_table_unref             (AcoTable       *table);
bool            aco_table_add_target        (AcoTable       *table,
                                             aco_id_t       target);
bool            aco_table_add_neigh         (AcoTable       *table,
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
aco_dist_t      aco_table_global_min        (AcoTable       *table,
                                             aco_id_t       target);
aco_id_t        aco_table_max_pheromon      (AcoTable       *table,
                                             aco_id_t       target,
                                             AcoValue       *value);
bool            aco_table_tx_info_update    (AcoTable       *table,
                                             aco_id_t       target,
                                             aco_id_t       neigh);
bool            aco_table_rx_info_update    (AcoTable       *table,
                                             aco_id_t       target,
                                             aco_id_t       neigh,
                                             aco_dist_t     dist);
void            aco_table_register_callee   (AcoTable       *table,
                                            evapor_cbfunc_t evapor_cbfunc,
                                            evapor_cbarg_t  evapor_cbarg,
                                            evapor_dtor_t   evapor_dtor);

#ifdef __cpluscplus
} // extern "C"
#endif

#endif /* AcoTable_H */
