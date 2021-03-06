#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <arpa/inet.h>

#include <fon/fon.h>
#include <fon/fon_defs.h>
#include <fon/fon_packet.h>
#include <fon/cassert.h>

#include "aco-table.h"
#include "ant-model.h"
#include "acod.h"

#include "ant.h"

#define  CHECK_POINT	(printf("Check point:: %s(%s:%d)\n", __FUNCTION__, __FILE__, __LINE__))

/*==============================================================================
 * Private Data
 *==============================================================================*/
static AntLogger  STATIC_LOGGER = NULL;

/*==============================================================================
 * Private Type Declarations
 *==============================================================================*/
typedef struct _RealAnt RealAnt;

typedef struct _AntOperator {
    void    (*send)         (RealAnt* fant);
    void    (*callback)     (RealAnt* fant);
} AntOperator;

struct _RealAnt {
// These members must be aligned in the same order in Ant.
    AcoTable    *table;
    AntObject   *obj;
    how_to_send_t sendto;
    void        *user_data;

// Internal Variables
    int         type;
    AntOperator op;
};

/*==============================================================================
 * Common Private Function Declarations
 *==============================================================================*/
static RealAnt* _real_ant_new       (AcoTable* table, AntObject* obj, how_to_send_t sendto, void *user_data, int type);
static void _unicast_pkt            (const Ant *ant, aco_id_t neighbor);
static int _unicast_forward         (RealAnt *ant);
static int _unicast_backward        (RealAnt *ant);
static void _pheromone_update       (AcoTable* table, aco_id_t target, aco_id_t neigh, aco_dist_t dist, AntModel model);
static void _backtrack_update       (AcoTable* table, AntObject* obj);
static void _source_update          (AcoTable* table, AntObject* obj, AntModel model);
static void _destination_update     (AcoTable* table, AntObject* obj, aco_id_t neigh, AntModel model);
static aco_id_t _select_neighbor    (AcoTable* table, AntObject* obj);

static void _register_on_table(AcoTable *table, aco_id_t id)
{
    aco_id_t    host_id = table->host;

    /* ant의 생성지를 보고 등록 안되어 있으면 적절하게 등록한다. */
    /* source != host_id: 백트래킹한 패킷이다. 자기자신은 테이블에 추가해서는 안된다.*/
    if(!aco_table_is_target(table, id) &&
       !aco_table_is_neigh(table, id) && 
        id != host_id) 
    {
        aco_table_add_target(table, id);
    }
}


/*==============================================================================
 * Virtual Function Declarations
 *==============================================================================*/
static void flood_send              (RealAnt* ant);
static void flood_callback          (RealAnt* ant);

static void ow_send                 (RealAnt* ant);
static void ow_callback             (RealAnt* ant);

static void rt_send                 (RealAnt* ant);
static void rt_callback             (RealAnt* ant);

static void test_send               (RealAnt* ant);
static void test_callback           (RealAnt* ant);

static const AntOperator ant_ops[] = {
    /* XXX ant-def.h 의 enum 순서와 동일한 배열을 생성해야 한다. */
    {flood_send,        flood_callback},        // ANT_TYPE_FLOOD
    {ow_send,           ow_callback},           // ANT_TYPE_ONEWAY
    {rt_send,           rt_callback},           // ANT_TYPE_ROUNDTRIP
    {test_send,         test_callback},         // ANT_TYPE_TEST
    {NULL,              NULL}                   // ANT_TYPE_WRONG
};


/*==============================================================================
 * Public Function Implentations
 *==============================================================================*/
void ant_unref(Ant* fant)
{
    RealAnt* ant = (RealAnt*)fant;

    ant_object_unref(ant->obj);
    free(ant);
}

typedef uint16_t packed_type_t;

static inline packed_type_t
pack_type(int type)
{
    return htons(type);
}

static inline int
unpack_type(packed_type_t type)
{
    return (int)ntohs(type);
}

void
ant_marshalling(const Ant          *fant,
                void               **pos,
                size_t             *remain)
{
    // TODO 길이체크 루틴 추가해야한다.

    RealAnt* ant = (RealAnt*)fant;

    *(packed_type_t*)(*pos) = pack_type(ant->type);
    *pos    += sizeof(packed_type_t);
    *remain -= sizeof(packed_type_t);

    ant_object_marshalling(ant->obj, pos, remain);
}

Ant*    
ant_demarshalling(const void         *buf,
                  size_t             len,
                  AcoTable           *table,
                  how_to_send_t      sendto,
                  void               *user_data)
{
    AntObject   *obj        = NULL;
    int         type        = -1;

    type = unpack_type(*(packed_type_t*)(buf));
    buf += sizeof(packed_type_t);
    len -= sizeof(packed_type_t);

    obj = ant_object_demarshalling(buf,
                                   len);

    return (Ant*)_real_ant_new(table,
                               obj,
                               sendto,
                               user_data,
                               type);
}

int
ant_cmp(const Ant       *fant1,
        const Ant       *fant2)
{
    const RealAnt* ant1 = (const RealAnt*)fant1;
    const RealAnt* ant2 = (const RealAnt*)fant2;

    if(ant1->type != ant2->type)
    {
        return -1;
    }

    return ant_object_cmp(ant1->obj, ant2->obj);
    
}

void
ant_send(Ant *fant)
{
    RealAnt* ant = (RealAnt*)fant;

    if(ant_object_get_ttl(ant->obj) == 0 ||
       ant->obj->destination == ant->table->host) {
        /* Drop Packet */
        return;
    }
    else
    {
        ant->op.send(ant);
        return;
    }
}

/* 패킷을 수신했을 때 호출되는 루틴이다. */
void
ant_callback(Ant* fant)
{

    RealAnt* ant = (RealAnt*)fant;
    AcoTable* table = ant->table;
    AntObject* obj = ant->obj;

    // 패킷을 수신했는데 목적지가 등록안된 새로운 것이라면 테이블에 추가한다.
    _register_on_table(table, obj->source);
    _register_on_table(table, obj->destination);

    if(ant_object_is_backtracked(obj))
    {
        #if ANT_BACKTRACK_UPDATE
        _backtrack_update(ant->table, ant->obj);
        #endif

        #if ANT_SOURCE_AGAIN_THEN_DROP
        if(ant->obj->source == ant->table->host)
        {
            goto RETURN;
        }
        #endif
    }
    else
    {
        // update statistics info
        aco_table_rx_info_update(table,
                        obj->source,
                        ant_object_previous(obj),
                        ant_object_dist(obj));
    }

    // enabled by ant_logger_set() method.
    if(STATIC_LOGGER != NULL)
    {
        STATIC_LOGGER(fant);
    }

    ant->op.callback(ant);
    ant_send(fant);

#if ANT_SOURCE_AGAIN_THEN_DROP
RETURN:
#endif
    return;
}

Ant*
ant_factory(int         type,
            aco_id_t    source,
            aco_id_t    destination,
            AcoTable    *table,
            how_to_send_t sendto,
            void        *user_data)
{
    if(type == ANT_TYPE_FLOOD)
    {
        destination = PACKET_ID_ANY;
    }

    AntObject   *obj = NULL;

    obj = ant_object_new(source,
                         destination,
                         ACO_DIST_MAX);

    return (Ant*)_real_ant_new(table,
                               obj,
                               sendto,
                               user_data,
                               type);
}

void
ant_logger_set(AntLogger logger)
{
    STATIC_LOGGER = logger;
}

AntLogger ant_logger_get()
{
    return STATIC_LOGGER;
}

/*==============================================================================
 * Private Function Implementations
 *==============================================================================*/
static RealAnt*
_real_ant_new(AcoTable      *table,
              AntObject     *obj,
              how_to_send_t sendto,
              void          *user_data,
              int           type)
{
    RealAnt* ant = (RealAnt*)malloc(sizeof(RealAnt));
    ant->table  = table;
    ant->obj    = obj;
    ant->sendto = sendto;
    ant->user_data = user_data;
    ant->type   = type;
    ant->op     = ant_ops[type];

    return ant;
}

static void
_unicast_pkt(const Ant      *ant,
             aco_id_t       neighbor)
{

    AcoTable    *table  = ant->table;
    AntObject   *obj    = ant->obj;

    if(neighbor == ACO_ID_WRONG)
    {
        return;
    }

    aco_table_tx_info_update(table,
                        obj->destination,
                        neighbor);

    // 마지막으로 패킷을 송신한다.
    if(ant->sendto != NULL)
    {
        ant->sendto(ant, table->host, neighbor);
    }
    else
    {
    	// Do nothing
    	// Something is wrong? well, I don't know. check constructor.
    }

    return;
}

static int _unicast_forward(RealAnt *ant)
{
    AcoTable    *table  = ant->table;
    AntObject   *obj    = ant->obj;
    aco_id_t    neighbor= _select_neighbor(table, obj);

    _unicast_pkt((Ant*)ant, neighbor);

    return neighbor;
}

static int _unicast_backward(RealAnt *ant)
{
    AntObject   *obj    = ant->obj;
    aco_id_t    neighbor= ant_object_next(obj);

    _unicast_pkt((Ant*)ant, neighbor);

    return neighbor;
}

static void _backtrack_update(AcoTable* table, AntObject* obj)
{
    aco_id_t id = ant_object_from(obj);
    AcoValue value = {
        .target = obj->destination,
        .neigh = id,
        };

    if(aco_table_get(table, &value))
    {
        value.pheromone = 0;
        aco_table_set(table, &value);
    }
}

static void
_pheromone_update(AcoTable      *table,
                  aco_id_t      target,
                  aco_id_t      neigh,
                  aco_dist_t    dist,
                  AntModel      model)
{
    if(target == ACO_ID_WRONG ||
       neigh == ACO_ID_WRONG)
    {
        return;
    }

    AcoValue value = {
            .target = target,
            .neigh = neigh,
        };

    if(aco_table_get(table, &value))
    {
        aco_dist_t local_min   = value.local_min;
        aco_dist_t global_min  = aco_table_global_min(table, value.target);
        model(&value.pheromone, global_min, local_min, dist);
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

    aco_id_t    neigh       = ant_object_previous(obj);
    aco_id_t    target      = obj->source;
    aco_dist_t  dist        = ant_object_dist(obj);

    _pheromone_update(table,
                      target,
                      neigh,
                      dist,
                      model);
}

static void _destination_update(AcoTable* table, AntObject* obj, aco_id_t neigh, AntModel model)
{
    if(ant_object_is_visited(obj, neigh))
    {
        return;
    }

    aco_id_t    target          = obj->destination;
    aco_dist_t  dist            = ant_object_dist(obj) + 1;

    _pheromone_update(table,
                      target,
                      neigh,
                      dist,
                      model);
}

static void _acs_update(AcoTable* table, AntObject* obj, aco_id_t neigh)
{
    if(ant_object_is_visited(obj, neigh))
    {
        return;
    }

    aco_id_t target = obj->destination;

    if(target == ACO_ID_WRONG ||
       neigh == ACO_ID_WRONG)
    {
        return;
    }

    AcoValue value =
        {
            .target = target,
            .neigh = neigh,
        };

    if(aco_table_get(table, &value))
    {
        aco_dist_t  global_min  = aco_table_global_min(table, obj->source)+1; 
        aco_dist_t  nhops       = ant_object_dist(obj) + 1;

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
    aco_ph_t        initial_accumulated;
    aco_ph_t        accumulated[ACO_TABLE_MAX_COL];
    int             len;

    // If this flags is true, the last element of array is never visited.
    bool            last_is_never_visited;
} Candidates;

/**
 * It means(asserts) that "accumulated[-1] == initial_accumulated".
 */
CASSERT(offsetof(Candidates, accumulated) == offsetof(Candidates, initial_accumulated) + sizeof(aco_ph_t), ant_c);

static Candidates* _candidates_new(const AntObject* obj, AcoTable* table)
{
    Candidates*     candidates      = malloc(sizeof(Candidates));
    AcoValue        *value          = NULL;

    candidates->initial_accumulated     = 0.0;
    candidates->len                     = 0;
    candidates->last_is_never_visited   = false;

    for(AcoTableIter iter =aco_table_iter_begin(table, obj->destination);
        iter.valid;
        aco_table_iter_next(table, &iter))
    {
        value = &iter.value;

        if(!ant_object_is_visited(obj, value->neigh))
        {
            candidates->candi[candidates->len] = *value;
            candidates->accumulated[candidates->len] = candidates->accumulated[candidates->len-1] + value->pheromone;
            candidates->len++;

            // If the given neighbor is never visited,
            // stop iterating and return.
            if(value->never_visited)
            {
                // switch flag.
                // because the ant will be send through this neighbor node.
                value->never_visited = false;
                aco_table_set(table, value);

                candidates->last_is_never_visited = true;
                goto RETURN;
            }
        }
    }

RETURN:
    return candidates;
}

static inline void
_candidates_free(Candidates* candidates)
{
    free(candidates);
}

static aco_id_t
_select_neighbor_randomly(const Candidates* candidates)
{
    assert(candidates->len > 0);

    aco_ph_t    sum_pheromone   = 0;
    const int   len             = candidates->len;
    aco_id_t    neigh           = ACO_ID_WRONG;
    double      random          = 0;

    sum_pheromone = candidates->accumulated[len-1];
    random = g_random_double_range(0, sum_pheromone);
    for(int i=0; i<len; i++)
    {
        if(random < candidates->accumulated[i])
        {
            neigh = candidates->candi[i].neigh;
            break;
        }
    }

    return neigh;
}

static aco_id_t
_select_neighbor(AcoTable* table, AntObject* obj)
{
    aco_id_t        destination     = obj->destination;
    aco_id_t        neigh           = ACO_ID_WRONG;
    Candidates*     candidates      = NULL;

    if(aco_table_is_neigh(table, destination))
    {
        neigh = destination;
        return neigh;
    }

    // Get neighbor nodes that do not make loop.
    candidates = _candidates_new(obj, table);

    // If there are no candidates
    // Do backtracking
    if(candidates->len == 0)
    {
        neigh = ant_object_previous(obj);
        goto RETURN;
    }

    // If there is the node never visited,
    // Give it a preference.
    if(candidates->last_is_never_visited)
    {
        neigh = candidates->candi[candidates->len-1].neigh;
        goto RETURN;
    }

    // If all nodes are visited at least once,
    // Randomly select a node considering the pheromone concentration.
    neigh = _select_neighbor_randomly(candidates);

RETURN:
    _candidates_free(candidates);

    return neigh;
}

/*
 * END of _select_neighbor()
 */

/*==============================================================================
 * Virtual Function Implementations
 *==============================================================================*/

// Called by virtual function, ant->op.send()
static void
flood_send(RealAnt* ant)
{
    AcoTable        *table  = ant->table;
    AntObject       *obj    = ant->obj;
    aco_id_t        tid     = table->host;
    aco_id_t        rid     = ACO_ID_WRONG;
    aco_ids_t       neighs  = NULL;

    /* flooding packet to neihbors except the already visited neihbors. */
    neighs = aco_table_new_neighs(table);
    int i= -1;
    while(neighs[++i] != ACO_ID_WRONG)
    {
        rid = neighs[i];

        /* 이미 방문한 곳이면, 보낼 필요가 없다. */
        if(ant_object_is_visited(obj, rid)) continue;

	ant->sendto((Ant*)ant, tid, rid);
    }

    aco_table_free_ids(neighs);

    return;
}

// Called by virtual function, ant->op.callback()
static void flood_callback(RealAnt* ant)
{
    // Do nothing.
}

/*==============================================================================
 * Virtual Function Implementations
 *==============================================================================*/

// Called by virtual function, ant->op.send()
static void
ow_send(RealAnt* ant)
{
    _unicast_forward(ant);

    return;
}

// Called by virtual function, ant->op.callback()
static void    ow_callback (RealAnt* ant)
{
    _source_update(ant->table, ant->obj, ANT_MODEL_SELECTOR);
}

/*==============================================================================
 * Virtual Function Implementations
 *==============================================================================*/

// Called by virtual function, ant->op.send()
static void rt_send(RealAnt* ant)
{
    AntObject* obj = ant->obj;

    if(ant_object_get_direction(obj) == ANT_OBJ_DIRECTION_FORWARD)
    {

#if ANT_DESTINATION_UPDATE
        aco_id_t neigh = _unicast_forward(ant);
        _destination_update(table, obj, neigh, ANT_MODEL_SELECTOR);
#elif ANT_ACS_UPDATE
        _unicast_forward(ant);
        _acs_update(table, obj, neigh);
#elif ANT_SOURCE_UPDATE
        _unicast_forward(ant);
#else
#  error
#endif
    }
    else if(ant_object_get_direction(obj) ==  ANT_OBJ_DIRECTION_BACKWARD)
    {
        _unicast_backward(ant);
    }
    else
    {
        abort();
    }
}

// Called by virtual function, ant->op.callback()
static void rt_callback(RealAnt* ant)
{
    AntObject* obj = ant->obj;

#if ANT_SOURCE_UPDATE
    _source_update(ant->table, ant->obj, ANT_MODEL_SELECTOR);
#endif

    ant_object_change_direction(obj);
}

/*==============================================================================
 * Virtual Function Implementations
 *==============================================================================*/
static void test_send(RealAnt* ant)
{
    // Do nothing
}

static void test_callback(RealAnt* ant)
{
    // Do nothing
}
