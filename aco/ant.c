#include "ant.h"
#include "fon/packet_if.h"
#include "aco-table.h"
#include "fon/fon.h"
#include "fon/algorithm.h"      /* max, min, swap */
#include "main.h"

#include "main-def.h"

#include <stdio.h>
#include <assert.h>

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

typedef void (*AntModel)(pheromone_t *ph, int global_min, int local_min, int nhops);

typedef struct _TableIteratorArgs
{
    int         neigh_id;
    int         nhops;
    AntModel    model;
} TableIteratorArgs;



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
static void _iterating_update       (AcoTable* table, AntObject* obj, int dest_id, int neigh_id, int nhops, AntModel model);
static bool _backtrack_update       (AcoTable* table, AntObject* obj);
static void _source_update          (AcoTable* table, AntObject* obj, AntModel model);
static void _destination_update     (AcoTable* table, AntObject* obj, int neigh_id, AntModel model);
static int _calc_neigh              (AcoTable* table, AntObject* obj);

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
 * Ant Model Declarations
 *==============================================================================*/
static void ant_normalizing_model   (pheromone_t *ph, int global_min, int local_min, int nhops);
static void ant_local_model         (pheromone_t *ph, int global_min, int local_min, int nhops);
static void ant_density_model       (pheromone_t *ph, int global_min, int local_min, int nhops);
static void ant_system_model        (pheromone_t *ph, int global_min, int local_min, int nhops);

static AntCallbackLogger  STATIC_LOGGER = NULL;

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

    if(ant_object_get_ttl(ant->obj) == 0) {
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

    /* 패킷을 수신했을 때 호출되는 루틴이다. */

    // 패킷을 수신했는데 목적지 등록안된 새로운 것이라면 테이블에 추가한다.
    _register_on_table(ant->table, ant->obj);

    // update statistics info
    _update_statistics(ant->table, ant->obj);

    #if ANT_BACKTRACK_UPDATE
    _backtrack_update(ant->table, ant->obj);
    #endif

    // enabled by ant_callback_logger_set() method.
    if(STATIC_LOGGER != NULL)
    {
        STATIC_LOGGER(fant);
    }

    ant->op.callback(ant);
    _forward_ant(fant);
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
                         ANT_DEFAULT_TTL);

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
    AcoValue    value   = {0,};

    // increase tx_count
    value.dest_id = obj->destination;
    value.neigh_id = rid;
    if(aco_table_get(table, &value))
    {
        value.tx_count++;
        aco_table_set(table, &value);
    }

    // 마지막으로 패킷을 송신한다.
    _make_pkt(obj, &pkt, tid, rid);
    fon_sendto(&pkt);

    return;
}

static int _unicast_forward(AcoTable* table, AntObject* obj)
{
    int         rid     = _calc_neigh(table, obj);

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
    if(!aco_table_is_dest(table, source) &&
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

    AcoValue value  = {0,};

    value.dest_id   = obj->source;
    value.neigh_id  = ant_object_previous(obj);

    aco_table_get(table, &value);

    if(aco_table_get(table, &value))
    {
        value.min_hops = MIN(ant_object_nhops(obj), value.min_hops);

        value.rx_count++;
        aco_table_set(table, &value);
    }
}

static void _forward_ant(Ant* fant)
{
    RealAnt* ant = (RealAnt*)fant;

    if(ant->obj->destination != ant->table->host_id)
    {
        ant_send(fant);
    }
    else
    {
        // Drop packet.
        // Do nothing.
    }
}

// Compatible with AcoTableIterator
static bool __pheromone_iterator(AcoTable *table, AcoValue *value, TableIteratorArgs *args)
{
    // 증가
    if(value->neigh_id == args->neigh_id)
    {
        int local_min   = value->min_hops;
        int global_min  = MIN(aco_table_min_hops(table, value->dest_id), local_min);
        int nhops       = args->nhops;

        args->model(&value->pheromone, global_min, local_min, nhops);
    }
    // 감소
    else
    {
        /* multiply the complement of evaporation rate */
        value->pheromone *= ANT_REMAINS_RATE;
    }

    return true;
}

static bool _backtrack_update(AcoTable* table, AntObject* obj)
{
    if(ant_object_is_backtracked(obj))
    {
        int id = ant_object_from(obj);
        AcoValue value = {.dest_id = obj->destination, .neigh_id = id};
        aco_table_get(table, &value);
        value.pheromone = 0;
        aco_table_set(table, &value);

        return true;
    }

    return false;
}

static void _iterating_update(AcoTable* table,
                            AntObject *obj,
                            int dest_id,
                            int neigh_id,
                            int nhops,
                            AntModel model)
{
    if(dest_id == PACKET_ID_INVALID ||
       neigh_id == PACKET_ID_INVALID)
    {
        return;
    }

    if(ant_object_is_backtracked(obj))
    {
        return;
    }

    TableIteratorArgs args =
                    {
                    .neigh_id       = neigh_id,
                    .nhops          = nhops,
                    .model          = model
                    };

    // 모든 인접노드 링크에 대한 페로몬 농도를 갱신한다.
    // 개미가 통과한 링크의 페로몬 농도는 증가 시키고
    // 그 외의 링크의 페로몬 농도는 감소시킨다.
    aco_table_iterate_by_dest(table, dest_id, (AcoTableIterator)__pheromone_iterator, &args);

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

    int previous        = ant_object_previous(obj);
    int source          = obj->source;
    int nhops           = ant_object_nhops(obj);

    _iterating_update(table,
                      obj,
                      source,
                      previous,
                      nhops,
                      model);
}

static void _destination_update(AcoTable* table, AntObject* obj, int neigh_id, AntModel model)
{
    if(ant_object_is_visited(obj, neigh_id))
    {
        return;
    }

    int destination     = obj->destination;
    int nhops           = ant_object_nhops(obj) + 1;

    _iterating_update(table,
                      obj,
                      destination,
                      neigh_id,
                      nhops,
                      model);
}

typedef struct _TestArg
{
    GPtrArray*          candidates;
    const AntObject*    obj;
} TestArg;

bool test(AcoTable *table, AcoValue *value, void *userdata)
{
    TestArg* arg = (TestArg*)userdata;

    // 개미가 방문한 노드가 아니면 후보에 추가한다.
    if(!ant_object_is_visited(arg->obj, value->neigh_id))
    {
        g_ptr_array_add(arg->candidates, value);
        if(value->tx_count == 0)
        {
            return false;
        }
    }

    return true;
}

static GPtrArray* get_did_candidates(const AntObject* obj, AcoTable* table)
{
    GPtrArray*      candidates      = g_ptr_array_new();
    TestArg         arg             = {candidates, obj};

    aco_table_iterate_by_dest(table, obj->destination, test, &arg);

    return candidates;
}

static int calc_randomly_neigh(const GPtrArray* candidate)
{
    g_assert(candidate->len > 0);

    typedef struct _Accumlated{
        int     id;
        double  accumulated;
    } Accumlated;

    Accumlated  ap              = {0, };
    AcoValue*   value           = NULL;
    pheromone_t sum_pheromone   = 0;
    const int   len             = candidate->len;
    int         did             = PACKET_ID_INVALID;
    double      random          = 0;
    GArray*     ap_array        = g_array_sized_new(FALSE,
                                    FALSE, sizeof(Accumlated), len);

    ap.id = PACKET_ID_INVALID;
    ap.accumulated = 0.0;
    //
    for(int i=0; i< len; i++)
    {
        value = (AcoValue*)g_ptr_array_index(candidate, i);
        ap.id             = value->neigh_id;
        ap.accumulated    += value->pheromone;
        g_array_append_val(ap_array, ap);
    }

    //
    sum_pheromone = g_array_index(ap_array, Accumlated, len-1).accumulated;
    random = g_random_double_range(0, sum_pheromone);
    for(int i=0; i<len; i++)
    {
        ap = g_array_index(ap_array, Accumlated, i);
        if(random < ap.accumulated)
        {
            did = ap.id;
            break;
        }
    }

    g_array_unref(ap_array);

    return did;
}

static int _calc_neigh(AcoTable* table, AntObject* obj)
{
    int             destination     = obj->destination;
    int             did             = PACKET_ID_INVALID;       /* neigh_id(next hop id) */
    GPtrArray*      candidates      = NULL;     /* array of AcoValue* */
    AcoValue*       unvisited       = NULL;

    // 목적지가 인접한 노드면, 즉시 리턴한다.
    if(aco_table_is_neigh(table, destination))
    {
        did = destination;
        return did;
    }

    // loop 발생시키는 노드 등을 제외한
    // neighbor가 될 수 있는 후보들을 얻는다.
    candidates = get_did_candidates(obj, table);

    // 만약 후보가 없다면
    // backtracking 할 수 있도록 한다.
    if(candidates->len == 0)
    {
        did = ant_object_previous(obj);
        goto RETURN;
    }

    // unvisited neighbor node가 존재하면
    // 페로몬 양에 관계없이 해당 노드에 최우선권을 준다.
    unvisited = g_ptr_array_index(candidates, candidates->len -1);
    if(!unvisited->tx_count)
    {
        did = unvisited->neigh_id;
        goto RETURN;
    }

    // 만약 모두 적어도 한번 방문했으면,
    // 페로몬양을 고려하여 확률에 기반한 다음 후보를 고른다.
    did = calc_randomly_neigh(candidates);

RETURN:
    g_ptr_array_unref(candidates);

    return did;
}

/*==============================================================================
 * Ant Model Implementations
 *==============================================================================*/
static void ant_normalizing_model(pheromone_t *ph, int global_min, int local_min, int nhops)
{
    int normalized = local_min - global_min + 1;
    int divider = normalized*normalized;

    // 정규화된 거리를 이용하여 페로몬을 갱신한다.
    // ANT_COCENTRATION_CONST를 적절히 선택했기 때문에
    // ACO_TABLE_PHEROMONE_MAX 값을 넘어 설 수 없다.
    *ph = (*ph)*ANT_REMAINS_RATE + ANT_COCENTRATION_CONST / (double)(divider);
}

static void ant_local_model(pheromone_t *ph, int global_min, int local_min, int nhops)
{
    *ph = (*ph)*ANT_REMAINS_RATE + ANT_COCENTRATION_CONST / (double)(local_min);
}

static void ant_density_model(pheromone_t *ph, int global_min, int local_min, int nhops)
{
    *ph = (*ph)*ANT_REMAINS_RATE + ANT_COCENTRATION_CONST;
}

static void ant_system_model(pheromone_t *ph, int global_min, int local_min, int nhops)
{
    *ph = (*ph)*ANT_REMAINS_RATE + ANT_COCENTRATION_CONST / (double)(nhops);
}

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
        #if ANT_DESTINATION_UPDATE
        int neigh_id = _unicast_forward(table, obj);
        _destination_update(table, obj, neigh_id, ANT_MODEL_SELECTOR);
        #endif /* ANT_DESTINATION_UPDATE */

        #if ANT_SOURCE_UPDATE
        _unicast_forward(table, obj);
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
    _source_update(ant->table, ant->obj, ant_system_model);
    #endif

    ant_object_change_direction(obj);
}




