#include "fon/packet_if.h"
#include "fon/fon.h"
#include "fon/algorithm.h"      /* max, min, swap */

#include "aco-table.h"
#include "ant.h"
#include "ant-model.h"
#include "main.h"

#include "cassert.h"

#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/*==============================================================================
 * Private Data
 *==============================================================================*/
static AntCallbackLogger  STATIC_LOGGER = NULL;

/*==============================================================================
 * Private Type Declarations
 *==============================================================================*/
typedef struct _RealAnt RealAnt;

typedef struct _AntOperator {
    void    (*send)         (RealAnt* fant);
    void    (*callback)     (RealAnt* fant);
} AntOperator;

struct _RealAnt {
    AcoTable*   table;
    AntObject*  obj;
    AntOperator op;
};

/*==============================================================================
 * Common Private Function Declarations
 *==============================================================================*/
static RealAnt* _real_ant_new       (AcoTable* table, AntObject* obj, AntOperator op);
static void _make_pkt               (const AntObject* obj, packet* pkt, int sid, int did);
static void _unicast_pkt            (AcoTable* table, AntObject* obj, int rid);
static int _unicast_forward         (AcoTable* table, AntObject* obj);
static int _unicast_backward        (AcoTable* table, AntObject* obj);
static void _register_on_table      (AcoTable* table, AntObject* obj);
static void _update_statistics      (AcoTable* table, AntObject* obj);
static void _forward_ant            (Ant* fant);
static void _pheromone_update       (AcoTable* table, int target_id, int neigh_id, int nhops, AntModel model);
static void _backtrack_update       (AcoTable* table, AntObject* obj);
static void _source_update          (AcoTable* table, AntObject* obj, AntModel model);
static void _destination_update     (AcoTable* table, AntObject* obj, int neigh_id, AntModel model);
static int _select_neighbor         (AcoTable* table, AntObject* obj);

/*==============================================================================
 * Virtual Function Declarations
 *==============================================================================*/
static void flood_send              (RealAnt* ant);
static void flood_callback          (RealAnt* ant);

static void oneway_send             (RealAnt* ant);
static void oneway_callback         (RealAnt* ant);

static void forward_send            (RealAnt* ant);
static void forward_callback        (RealAnt* ant);

static const AntOperator ant_ops[] = {
    /* XXX ant-def.h 의 enum 순서와 동일한 배열을 생성해야 한다. */
    {flood_send,        flood_callback},        /* ANT_TYPE_FLOOD       */
    {oneway_send,       oneway_callback},       /* ANT_TYPE_ONEWAY      */
    {forward_send,      forward_callback},      /* ANT_TYPE_FORWARD     */
    {NULL,              NULL}                   /* ANT_TYPE_WRONG       */
};

/*==============================================================================
 * Public Function Implentations
 *==============================================================================*/
void ant_unref(Ant* fant)
{
    RealAnt* ant = (RealAnt*)fant;

    ant_object_unref(ant->obj);
    g_free(ant);
}

void ant_send(Ant* fant)
{
    RealAnt* ant = (RealAnt*)fant;

    if(ant_object_get_ttl(ant->obj) == 0 ||
       ant->obj->destination == ant->table->host_id) {
        /* Drop Packet */
        return;
    }
    else
    {
        ant->op.send(ant);
        return;
    }
}

void ant_callback(Ant* fant)
{
    RealAnt* ant = (RealAnt*)fant;
    AcoTable* table = ant->table;
    AntObject* obj = ant->obj;

    /* 패킷을 수신했을 때 호출되는 루틴이다. */

    // 패킷을 수신했는데 목적지 등록안된 새로운 것이라면 테이블에 추가한다.
    _register_on_table(table, obj);

    if(ant_object_is_backtracked(obj))
    {
        #if ANT_BACKTRACK_UPDATE
        _backtrack_update(ant->table, ant->obj);
        #endif
    }
    else
    {
        // update statistics info
        aco_table_rx_info_update(table,
                        obj->source,
                        ant_object_previous(obj),
                        ant_object_nhops(obj));
    }

    // enabled by ant_callback_logger_set() method.
    if(STATIC_LOGGER != NULL)
    {
        STATIC_LOGGER(fant);
    }

    ant->op.callback(ant);
    ant_send(fant);
}

Ant* ant_restore(const packet* pkt, AcoTable* table)
{
    AntObject*  obj;

    obj = ant_object_demarshalling( pkt->hdr.pkt_data,
                                    pkt->hdr.pkt_len);

    ant_object_arrived_at(obj, table->host_id);

    return (Ant*)_real_ant_new(table, obj, ant_ops[obj->type]);
}

Ant* ant_factory(int type, int source, int destination, AcoTable* table)
{
    if(type == ANT_TYPE_FLOOD)
    {
        destination = PACKET_ID_ANY;
    }

    AntObject*  obj;
    obj = ant_object_new(source,
                         destination,
                         type,
                         ANT_MAXIMUM_TTL);

    return (Ant*)_real_ant_new(table, obj, ant_ops[obj->type]);
}

void ant_callback_logger_set(AntCallbackLogger logger)
{
    STATIC_LOGGER = logger;
}

AntCallbackLogger ant_callback_logger_get()
{
    return STATIC_LOGGER;
}

/*==============================================================================
 * Private Function Implementations
 *==============================================================================*/
static RealAnt* _real_ant_new(AcoTable* table, AntObject* obj, AntOperator op)
{
    RealAnt* ant = g_new0(RealAnt, 1);
    ant->obj    = obj;
    ant->op     = op;
    ant->table  = table;

    return ant;
}


static void _make_pkt(const AntObject* obj, packet* pkt, int tid, int rid)
{
    int len = sizeof(packet) - sizeof(packet_hdr);
    ant_object_marshalling(obj, &pkt->hdr.pkt_data, &len);
    pkt->hdr.pkt_sid     = tid;
    pkt->hdr.pkt_did     = rid;
    pkt->hdr.pkt_len     = len;
    pkt->hdr.pkt_type    = FON_FUNC_TYPE_ACO;
}

static void _unicast_pkt(AcoTable* table, AntObject* obj, int rid)
{
    if(rid == PACKET_ID_INVALID)
    {
        return;
    }

    int         tid     = table->host_id;
    packet      pkt     = {{0,}};

    aco_table_tx_info_update(table,
                        obj->destination,
                        rid);

    // 마지막으로 패킷을 송신한다.
    _make_pkt(obj, &pkt, tid, rid);
    fon_sendto(&pkt);

    return;
}

static int _unicast_forward(AcoTable* table, AntObject* obj)
{
    int         rid     = _select_neighbor(table, obj);

    _unicast_pkt(table, obj, rid);

    return rid;
}

static int _unicast_backward(AcoTable* table, AntObject* obj)
{
    int rid = ant_object_backward_next(obj);
    _unicast_pkt(table, obj, rid);

    return rid;
}

static void _register_on_table(AcoTable* table, AntObject* obj)
{
    int source = obj->source;
    int host_id = table->host_id;

    /* ant의 생성지를 보고 등록 안되어 있으면 적절하게 등록한다. */
    /* source != host_id: 백트래킹한 패킷이다. 자기자신은 테이블에 추가해서는 안된다.*/
    if(!aco_table_is_target(table, source) &&
       !aco_table_is_neigh(table, source) && 
        source != host_id) 
    {
        aco_table_add_row(table, source);
    }

}

static void _update_statistics(AcoTable* table, AntObject* obj)
{
    if(ant_object_is_backtracked(obj))
    {
        return;
    }

    aco_table_rx_info_update(table,
                    obj->source,
                    ant_object_previous(obj),
                    ant_object_nhops(obj));
}

static void _backtrack_update(AcoTable* table, AntObject* obj)
{
    int id = ant_object_from(obj);
    AcoValue value = {
        .target_id = obj->destination,
        .neigh_id = id};

    if(aco_table_get(table, &value))
    {
        value.pheromone = 0;
        aco_table_set(table, &value);
    }
}

static void _pheromone_update(AcoTable* table,
                            int target_id,
                            int neigh_id,
                            int nhops,
                            AntModel model)
{
    if(target_id == PACKET_ID_INVALID ||
       neigh_id == PACKET_ID_INVALID)
    {
        return;
    }

    AcoValue value = {
            .target_id = target_id,
            .neigh_id = neigh_id,
        };

    if(aco_table_get(table, &value))
    {
        int local_min   = value.local_min;
        int global_min  = aco_table_global_min(table, value.target_id);
        model(&value.pheromone, global_min, local_min, nhops);
        aco_table_set(table, &value);
    }

    return;
}

static void _source_update(AcoTable* table, AntObject* obj, AntModel model)
{
    // Using "Source update" algorithm introduced in the Paper
    // "A Parallel Ant Colony Optimization Algorithm for All-Pair Routing in MANETs"

    if(ant_object_is_backtracked(obj))
    {
        return;
    }

    int neigh_id        = ant_object_previous(obj);
    int target          = obj->source;
    int nhops           = ant_object_nhops(obj);

    _pheromone_update(table,
                      target,
                      neigh_id,
                      nhops,
                      model);
}

static void _destination_update(AcoTable* table, AntObject* obj, int neigh_id, AntModel model)
{
    if(ant_object_is_visited(obj, neigh_id))
    {
        return;
    }

    int target          = obj->destination;
    int nhops           = ant_object_nhops(obj) + 1;

    _pheromone_update(table,
                      target,
                      neigh_id,
                      nhops,
                      model);
}

static void _acs_update(AcoTable* table, AntObject* obj, int neigh_id)
{
    if(ant_object_is_visited(obj, neigh_id))
    {
        return;
    }

    int target_id = obj->destination;

    if(target_id == PACKET_ID_INVALID ||
       neigh_id == PACKET_ID_INVALID)
    {
        return;
    }

    AcoValue value =
        {
            .target_id = target_id,
            .neigh_id = neigh_id,
        };

    if(aco_table_get(table, &value))
    {
        int global_min  = aco_table_global_min(table, obj->source)+1; 
        int nhops       = ant_object_nhops(obj) + 1;

        ant_colony_system_model(&value.pheromone,
                                global_min,
                                -1/*noused in ACS model*/,
                                nhops);
        aco_table_set(table, &value);
    }

    return;
}

/*
 * BEGINNING of _select_neighbor()
 */
typedef struct _Candidates
{
    AcoValue        candi[ACO_TABLE_MAX_COL];
    pheromone_t     initial_accumulated;
    pheromone_t     accumulated[ACO_TABLE_MAX_COL];
    int             len;
    bool last_is_never_visited;
} Candidates;

/**
 * It means(asserts) that "accumulated[-1] == initial_accumulated".
 */
CASSERT(offsetof(Candidates, accumulated) == offsetof(Candidates, initial_accumulated) + sizeof(pheromone_t), ant_c);

static Candidates* _candidates_new(const AntObject* obj, AcoTable* table)
{
    Candidates*     candidates      = malloc(sizeof(Candidates));
    AcoTableIter    iter            = {{0,}};
    AcoValue        *value          = NULL;

    candidates->initial_accumulated     = 0.0;
    candidates->len                     = 0;
    candidates->last_is_never_visited   = false;

    if(!aco_table_iter_begin(table, obj->destination, &iter))
    {
        goto RETURN;
    }

    // iterating
    do
    {
        value = &iter.value;

        if(!ant_object_is_visited(obj, value->neigh_id))
        {
            candidates->candi[candidates->len] = *value;
            candidates->accumulated[candidates->len] = candidates->accumulated[candidates->len-1] + value->pheromone;
            candidates->len++;

            // If the given neighbor is never visited,
            // stop iterating and return.
            if(value->tx_count == 0)
            {
                candidates->last_is_never_visited = true;
                goto RETURN;
            }
        }
    }
    while(aco_table_iter_next(table, &iter));

RETURN:
    return candidates;
}

static inline void _candidates_free(Candidates* candidates)
{
    free(candidates);
}

static int _select_neighbor_randomly(const Candidates* candidates)
{
    g_assert(candidates->len > 0);

    pheromone_t sum_pheromone   = 0;
    const int   len             = candidates->len;
    int         neigh_id             = PACKET_ID_INVALID;
    double      random          = 0;

    sum_pheromone = candidates->accumulated[len-1];
    random = g_random_double_range(0, sum_pheromone);
    for(int i=0; i<len; i++)
    {
        if(random < candidates->accumulated[i])
        {
            neigh_id = candidates->candi[i].neigh_id;
            break;
        }
    }

    return neigh_id;
}

static int _select_neighbor(AcoTable* table, AntObject* obj)
{
    int             destination     = obj->destination;
    int             neigh_id        = PACKET_ID_INVALID;
    Candidates*     candidates      = NULL;

    if(aco_table_is_neigh(table, destination))
    {
        neigh_id = destination;
        return neigh_id;
    }

    // Get neighbor nodes that do not make loop.
    candidates = _candidates_new(obj, table);

    // If there are no candidates
    // Do backtracking
    if(candidates->len == 0)
    {
        neigh_id = ant_object_previous(obj);
        goto RETURN;
    }

    // If there is the node never visited,
    // Give it a preference.
    if(candidates->last_is_never_visited)
    {
        neigh_id = candidates->candi[candidates->len-1].neigh_id;
        goto RETURN;
    }

    // If all nodes are visited at least once,
    // Randomly select a node considering the pheromone concentration.
    neigh_id = _select_neighbor_randomly(candidates);

RETURN:
    _candidates_free(candidates);

    return neigh_id;
}

/*
 * END of _select_neighbor()
 */

/*==============================================================================
 * Virtual Function Implementations
 *==============================================================================*/
static void flood_send(RealAnt* ant)
{
    AcoTable* table = ant->table;
    AntObject* obj = ant->obj;
    int     sid     = table->host_id;
    int     did;
    packet  pkt;
    int     *neighs = NULL;

    /* flooding except the already visited nodes. */
    neighs = aco_table_new_neighs(table);
    int i= -1;
    while(neighs[++i] != -1)
    {
        did = neighs[i];

        /* 이미 방문한 곳이면, 보낼 필요가 없다. */
        if(ant_object_is_visited(obj, did)) continue;

        _make_pkt(obj, &pkt, sid, did);

        fon_sendto(&pkt);
    }

    aco_table_free_array(neighs);

    return;
}

static void flood_callback(RealAnt* ant)
{
    // Do nothing.
}

/*==============================================================================
 *
 *==============================================================================*/
static void oneway_send(RealAnt* ant)
{
    _unicast_forward(ant->table, ant->obj);

    return;
}

static void    oneway_callback (RealAnt* ant)
{
    _source_update(ant->table, ant->obj, ant_normalizing_model);
}

/*==============================================================================
 *
 *==============================================================================*/

static void forward_send(RealAnt* ant)
{
    AcoTable* table = ant->table;
    AntObject* obj = ant->obj;

    if(ant_object_get_direction(obj) == ANT_OBJ_DIRECTION_FORWARD)
    {
        int neigh_id = _unicast_forward(table, obj);

        #if ANT_DESTINATION_UPDATE
        _destination_update(table, obj, neigh_id, ANT_MODEL_SELECTOR);
        #elif ANT_ACS_UPDATE
        _acs_update(table, obj, neigh_id);
        #endif
    }
    else if(ant_object_get_direction(obj) ==  ANT_OBJ_DIRECTION_BACKWARD)
    {
        _unicast_backward(table, obj);
    }
    else
    {
        abort();
    }
}

static void forward_callback(RealAnt* ant)
{
    AntObject* obj = ant->obj;

    #if ANT_SOURCE_UPDATE
    _source_update(ant->table, ant->obj, ANT_MODEL_SELECTOR);
    #endif

    ant_object_change_direction(obj);
}



