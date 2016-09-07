#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "ant-def.h"

#include "ant-obj.h"


/*==============================================================================
 * Private Declaration
 *==============================================================================*/
#define _INVALID_VALUE                  (-1)
#define ANT_OBJ_MAXIMUM_ARR_SIZE        (ACO_DIST_MAX)
#define _MAGIC_INDEX                    (ANT_OBJ_MAXIMUM_ARR_SIZE)

static const char _DirectionString[][64] = {
    "ANT_OBJ_DIRECTION_FORWARD",
    "ANT_OBJ_DIRECTION_BACKWARD",
    "ANT_OBJ_DIRECTION_MAX",
    "ANT_OBJ_DIRECTION_INVALID"
};

typedef struct _RealObject              RealObject;
typedef struct _AntObjectOp             AntObjectOp;
typedef struct _AntObjectMarshalled     AntObjectMarshalled;

struct _AntObjectOp
{
    void        (*op_arrived_at)       (RealObject* obj, aco_id_t id);
    bool        (*op_is_backtracked)   (const RealObject* obj);
    aco_dist_t  (*op_dist)             (const RealObject* obj);
    aco_id_t    (*op_previous)         (const RealObject* obj);
    aco_id_t    (*op_next)             (const RealObject* obj);
};

struct _RealObject {
    // These members must be aligned in the same order in AntObject.
    aco_id_t        source;
    aco_id_t        destination;

    // Internal Variables
    aco_direction_t direction;
    aco_ttl_t       ini_ttl;
    aco_ttl_t       cur_ttl;
    int             nwalk;
    int             npath;
    int             nvisited;
    int             ndists;
    aco_id_t        walk[ANT_OBJ_MAXIMUM_ARR_SIZE+1];
    aco_id_t        path[ANT_OBJ_MAXIMUM_ARR_SIZE+1];
    aco_id_t        visited[ANT_OBJ_MAXIMUM_ARR_SIZE+1];
    aco_dist_t      dists[ANT_OBJ_MAXIMUM_ARR_SIZE+1];
    AntObjectOp     ops;
};


struct __attribute__((packed)) _AntObjectMarshalled {
    aco_id_packed_t            source;
    aco_id_packed_t            destination;
    aco_direction_packed_t     direction;
    aco_ttl_packed_t           ini_ttl;
    aco_ttl_packed_t           cur_ttl;
    uint16_t    nwalk;
    uint16_t    npath;
    uint16_t    nvisited;
    uint16_t    ndists;
    uint16_t    data[];
};

/*==============================================================================
 * Virtual Function Declaration
 *==============================================================================*/
static void        _forward_op_arrived_at          (RealObject* obj, aco_id_t cur);
static bool        _forward_op_is_backtracked      (const RealObject* obj);
static aco_dist_t  _forward_op_dist                (const RealObject* obj);
static aco_id_t    _forward_op_previous            (const RealObject* obj);
static aco_id_t    _forward_op_next                (const RealObject* obj);

static void        _backward_op_arrived_at         (RealObject* obj, aco_id_t cur);
static bool        _backward_op_is_backtracked     (const RealObject* obj);
static aco_dist_t  _backward_op_dist               (const RealObject* obj);
static aco_id_t    _backward_op_previous           (const RealObject* obj);
static aco_id_t    _backward_op_next               (const RealObject* obj);

static const AntObjectOp AntObjectOps[ANT_OBJ_DIRECTION_MAX] = {
    {
        .op_arrived_at          = _forward_op_arrived_at,
        .op_is_backtracked      = _forward_op_is_backtracked,
        .op_dist                = _forward_op_dist,
        .op_previous            = _forward_op_previous,
        .op_next                = _forward_op_next,
    },

    {
        .op_arrived_at          = _backward_op_arrived_at,
        .op_is_backtracked      = _backward_op_is_backtracked,
        .op_dist                = _backward_op_dist,
        .op_previous            = _backward_op_previous,
        .op_next                = _backward_op_next,
    }

};

/*==============================================================================
 * Private Function Implementations
 *==============================================================================*/
static void
_init_dists(aco_dist_t      *dists)
{
    for(int i=0; i< ANT_OBJ_MAXIMUM_ARR_SIZE+1; i++)
    {
        dists[i] = _INVALID_VALUE;
    }
}

static void
_init_ids(aco_id_t      *ids)
{
    for(int i=0; i< ANT_OBJ_MAXIMUM_ARR_SIZE+1; i++)
    {
        ids[i] = ACO_ID_WRONG;
    }
}

static inline void
_add_id(aco_id_t        *ids,
        int             *len,
        aco_id_t        id)
{
    if(*len == ANT_OBJ_MAXIMUM_ARR_SIZE)
    {
        /* exceed array boundary */
        abort();
    }
    ids[(*len)++] = id;
}
#define _add_path(obj, val)                             _add_id((obj->path),     &(obj->npath),      (val))
#define _add_walk(obj, val)                             _add_id((obj->walk),   &(obj->nwalk),    (val))
#define _add_visited(obj, val)                          _add_id((obj->visited),  &(obj->nvisited),   (val))

static inline void
_add_dists(RealObject       *obj,
           aco_dist_t       dist)
{
    int* len = &obj->ndists;
    if(*len == ANT_OBJ_MAXIMUM_ARR_SIZE)
    {
        /* exceed array boundary */
        abort();
    }
    obj->dists[(*len)++] = dist;
}

static inline void
_pop_ids(aco_id_t       *ids,
         int            *len)
{
    if(*len == 0)
    {
        /* exceed array boundary */
        abort();
    }
    ids[--(*len)] = ACO_ID_WRONG;
}
#define _pop_path(obj)                                  _pop_ids((obj->path),     &(obj->npath))
#define _pop_walk(obj)                                  _pop_ids((obj->walk),   &(obj->nwalk))
#define _pop_visited(obj)                               _pop_ids((obj->visited),  &(obj->nvisited))

static inline void
_pop_dists(aco_dist_t       *dists,
           int              *len)
{
    if(*len == 0)
    {
        /* exceed array boundary */
        abort();
    }
    dists[--(*len)] = _INVALID_VALUE;
}

static void
_marshalling_ids(uint16_t           data[],
                 int                *data_index,
                 const aco_id_t     ids[],
                 int                len)
{
    for(int i=0;
        i<len;
        i++, (*data_index)++)
    {
        data[*data_index] = htons(ids[i]);
    }
}
#define _marshalling_walk(obj, data, data_index)        _marshalling_ids(data,  data_index, obj->walk,    obj->nwalk)
#define _marshalling_path(obj, data, data_index)        _marshalling_ids(data,  data_index, obj->path,      obj->npath)
#define _marshalling_visited(obj, data, data_index)     _marshalling_ids(data,  data_index, obj->visited,   obj->nvisited)

static void
_marshalling_dist(uint16_t          data[],
                  int               *data_index,
                  const aco_dist_t  dists[],
                  int               len)
{
    for(int i=0;
        i<len;
        i++, (*data_index)++)
    {
        data[*data_index] = htons(dists[i]);
    }
}
#define _marshalling_vhops(obj, data, data_index)       _marshalling_dist(data,  data_index, obj->dists,     obj->ndists)

static void
_demarshalling_ids(const uint16_t       data[],
                   int                  *data_index,
                   aco_id_t             ids[],
                   int                  len)
{
    for(int i=0;
        i<len;
        i++, (*data_index)++)
    {
        ids[i] = ntohs(data[*data_index]);
    }
}
#define _demarshalling_walk(obj, data, data_index)      _demarshalling_ids(data,  data_index, obj->walk,    obj->nwalk)
#define _demarshalling_path(obj, data, data_index)      _demarshalling_ids(data,  data_index, obj->path,      obj->npath)
#define _demarshalling_visited(obj, data, data_index)   _demarshalling_ids(data,  data_index, obj->visited,   obj->nvisited)

static void
_demarshalling_dists(const uint16_t     data[],
                     int                *data_index,
                     aco_dist_t         dists[],
                     int                len)
{
    for(int i=0;
        i<len;
        i++, (*data_index)++)
    {
        dists[i] = ntohs(data[*data_index]);
    }
}
#define _demarshalling_vhops(obj, data, data_index)     _demarshalling_dists(data,  data_index, obj->dists,     obj->ndists)


static int
_find_visited_idx(const aco_id_t    *visited,
                  aco_id_t          id)
{
    // temporarily set and rollback
    int idx = -1;

    *(aco_id_t*)(visited+_MAGIC_INDEX) = id;
    while(visited[++idx] != id);
    *(aco_id_t*)(visited+_MAGIC_INDEX) = ACO_ID_WRONG;

    return idx;
}

static inline size_t
_calc_marshalled_size(const RealObject  *obj)
{
    return
        sizeof(AntObjectMarshalled) +
        sizeof(uint16_t)*(obj->nwalk + obj->npath + obj->nvisited + obj->ndists);
}

static void
_real_object_init(RealObject    *obj,
                  aco_id_t      source,
                  aco_id_t      destination,
                  int           direction,
                  int           ini_ttl,
                  int           cur_ttl)
{
    obj->source         = source;
    obj->destination    = destination;
    obj->direction      = direction;
    obj->ini_ttl        = ini_ttl;
    obj->cur_ttl        = cur_ttl;
    obj->ops            = AntObjectOps[direction];

    obj->nwalk          = 0;
    obj->npath          = 0;
    obj->nvisited       = 0;
    obj->ndists         = 0;

    _init_ids(obj->walk);
    _init_ids(obj->path);
    _init_ids(obj->visited);
    _init_dists(obj->dists);

    _add_walk(obj, source);
    _add_path(obj, source);
    _add_visited(obj, source);
    _add_dists(obj, 0);
}

static RealObject*
_real_object_new(aco_id_t   source,
                 aco_id_t   destination,
                 int        direction,
                 int        ini_ttl,
                 int        cur_ttl)
{
    RealObject* obj     = malloc(sizeof(RealObject));

    _real_object_init(obj, source, destination, direction, ini_ttl, cur_ttl);

    return obj;
}

/*==============================================================================
 * Public Function Implementations
 *==============================================================================*/

AntObject*
ant_object_new(aco_id_t     source,
               aco_id_t     destination,
               int          ini_ttl)
{
    return (AntObject*)_real_object_new(source,
                                        destination,
                                        ANT_OBJ_DIRECTION_FORWARD,
                                        ini_ttl,
                                        ini_ttl);
}

void
ant_object_unref(AntObject      *fobj)
{
    free(fobj);
}

void
ant_object_print(const AntObject    *fobj)
{
    const RealObject*   obj         = (const RealObject*)fobj;
    const aco_id_t*     walk        = obj->walk;
    const aco_id_t*     visited     = obj->visited;
    const aco_id_t*     path        = obj->path;
    const aco_dist_t*   vhops       = obj->dists;

    printf("source:      %-4d\n"
           "destination: %-4d\n"
           "direction:   %s\n"
           "cur_ttl:     %-4d\n",
           obj->source,
           obj->destination,
           _DirectionString[obj->direction],
           obj->cur_ttl);

    printf("walk:        ");
    for(int i=0; i< obj->nwalk; i++) {
        printf("%-4d", walk[i]);
    }
    printf("\n");

    printf("visited:     ");
    for(int i=0; i< obj->nvisited; i++) {
        printf("%-4d", visited[i]);
    }
    printf("\n");

    printf("dists:       ");
    for(int i=0; i< obj->ndists; i++) {
        printf("%-4d", vhops[i]);
    }
    printf("\n");

    printf("path:        ");
    for(int i=0; i< obj->npath; i++) {
        printf("%-4d", path[i]);
    }
    printf("\n\n");

}

void
ant_object_print_path(
        const AntObject*    fobj)
{
    const RealObject*   obj     = (const RealObject*)fobj;
    const aco_id_t*     path    = obj->path;

    for(int i=0; i< obj->npath; i++) {
        printf("%2d  ", path[i]);
    }
    printf("\n");
}

void
ant_object_print_walk(
        const AntObject *   fobj)
{
    const RealObject*    obj        = (const RealObject*)fobj;
    const aco_id_t*      walk       = obj->walk;

    for(int i=0; i< obj->nwalk; i++) {
        fprintf(stderr, "%4d", walk[i]);
    }
    fprintf(stderr, "\n");

}

void
ant_object_print_dbg_hops(
        const AntObject *   fobj)
{
    const RealObject*    obj        = (const RealObject*)fobj;
    fprintf(stderr, "%4d\n", obj->dists[obj->nvisited-1]);
}

void ant_object_marshalling(const AntObject *fobj, void **pos, int *reamin)
{
    const RealObject    *in_obj     = (const RealObject*)fobj;
    AntObjectMarshalled*
                        marshalled  = *pos;
    size_t              len         = _calc_marshalled_size(in_obj);

    if(*reamin < len) {
        /* Not enough memry */
        abort();
    }
    else {
        *reamin -= len;
        *pos    += len;
    }

    marshalled->source      = ACO_ID_PACK(in_obj->source);
    marshalled->destination = ACO_ID_PACK(in_obj->destination);
    marshalled->direction   = ACO_DIRECTION_PACK(in_obj->direction);
    marshalled->ini_ttl     = ACO_TTL_PACK(in_obj->ini_ttl);
    marshalled->cur_ttl     = ACO_TTL_PACK(in_obj->cur_ttl);
    marshalled->nwalk       = htons(in_obj->nwalk);
    marshalled->npath       = htons(in_obj->npath);
    marshalled->nvisited    = htons(in_obj->nvisited);
    marshalled->ndists      = htons(in_obj->ndists);

    int data_index  = 0;
    _marshalling_walk       (in_obj, marshalled->data, &data_index);
    _marshalling_path       (in_obj, marshalled->data, &data_index);
    _marshalling_visited    (in_obj, marshalled->data, &data_index);
    _marshalling_vhops      (in_obj, marshalled->data, &data_index);

    return;
}

AntObject*
ant_object_demarshalling(const void     *in_buf,
                         int            buflen)
{
    const AntObjectMarshalled*
                    marshalled = in_buf;

    RealObject*         obj         = _real_object_new(ACO_ID_UNPACK(marshalled->source),
                                        ACO_ID_UNPACK(marshalled->destination),
                                        ACO_DIRECTION_UNPACK(marshalled->direction),
                                        ACO_TTL_UNPACK(marshalled->ini_ttl),
                                        ACO_TTL_UNPACK(marshalled->cur_ttl)
                                        );

    obj->nwalk      = ntohs(marshalled->nwalk);
    obj->npath      = ntohs(marshalled->npath);
    obj->nvisited   = ntohs(marshalled->nvisited);
    obj->ndists     = ntohs(marshalled->ndists);

    int data_index = 0;
    _demarshalling_walk       (obj, marshalled->data, &data_index);
    _demarshalling_path       (obj, marshalled->data, &data_index);
    _demarshalling_visited    (obj, marshalled->data, &data_index);
    _demarshalling_vhops      (obj, marshalled->data, &data_index);

    return (AntObject*)obj;
}

bool
ant_object_is_visited(const AntObject   *fobj,
                      aco_id_t          id)
{
    const RealObject* obj = (const RealObject*)fobj;
    int         idx         = -1;
    const aco_id_t*   visited    = obj->visited;

    idx = _find_visited_idx(visited, id);

    return idx != _MAGIC_INDEX;
}

void
ant_object_arrived_at(AntObject     *fobj,
                      aco_id_t      cur)
{
    RealObject* obj = (RealObject*)fobj;
    obj->ops.op_arrived_at(obj, cur);
}

bool
ant_object_change_direction(AntObject *fobj)
{
    RealObject* obj = (RealObject*)fobj;

    if(obj->direction == ANT_OBJ_DIRECTION_FORWARD &&
        obj->destination == obj->walk[obj->nwalk - 1])
    {
        int tmp         = obj->source;
        obj->source     = obj->destination;
        obj->destination = tmp;

        obj->direction  = ANT_OBJ_DIRECTION_BACKWARD;
        obj->cur_ttl    = obj->ini_ttl;
        obj->ops        = AntObjectOps[ANT_OBJ_DIRECTION_BACKWARD];

        return true;
    }

    return false;
}

bool
ant_object_is_backtracked(
        const AntObject *   fobj)
{
    const RealObject* obj = (const RealObject*)fobj;
    return obj->ops.op_is_backtracked(obj);
}

aco_dist_t
ant_object_dist(const AntObject *fobj)
{
    const RealObject* obj = (const RealObject*)fobj;
    return obj->ops.op_dist(obj);
}

int
ant_object_get_direction(const AntObject *fobj)
{
    const RealObject* obj = (const RealObject*)fobj;
    return obj->direction;
}

int
ant_object_get_ttl(const AntObject *fobj)
{
    const RealObject* obj = (const RealObject*)fobj;
    return obj->cur_ttl;
}

aco_id_t
ant_object_previous(const AntObject *fobj)
{
    const RealObject* obj = (const RealObject*)fobj;
    return obj->ops.op_previous(obj);
}

aco_id_t
ant_object_next(const AntObject *fobj)
{
    const RealObject* obj = (const RealObject*)fobj;
    return obj->ops.op_next(obj);
}

aco_id_t
ant_object_from(const AntObject* fobj)
{
    const RealObject* obj = (const RealObject*)fobj;

    if(obj->nwalk != 1)
    {
        return obj->walk[obj->nwalk-2];
    }
    else
    {
        return ACO_ID_WRONG;
    }
}

int
ant_object_cmp(const AntObject      *fobj1,
               const AntObject      *fobj2)
{
    return memcmp(fobj1, fobj2, sizeof(RealObject));
}

/*==============================================================================
 * Virtual Function Implementations
 *==============================================================================*/

void
_forward_op_arrived_at(RealObject    *obj,
                      aco_id_t      cur)
{
    /* 수신한 패킷이 backtrack 된것인지 아닌지에 따라 패킷을 업데이트한다. */
    if(ant_object_is_visited((AntObject*)obj, cur))
    {
        obj->cur_ttl--;
        _pop_path(obj);
        _add_walk(obj, cur);
    }
    else
    {
        int pre_id = obj->path[obj->npath-1];
        int pre_visited_idx = -1;

        obj->cur_ttl--;

        _add_visited(obj, cur);
        _add_path(obj, cur);
        _add_walk(obj, cur);

        pre_visited_idx = _find_visited_idx(obj->visited, pre_id);
        _add_dists(obj, obj->dists[pre_visited_idx] + 1);
    }
}

bool
_forward_op_is_backtracked(const RealObject  *obj)
{
    /*
     * last_path: current node id
     */
    int visited_len     = obj->nvisited;
    int last_visited    = obj->visited[obj->nvisited-1];
    int last_path      = obj->path[obj->npath-1];

    if(visited_len == 1)
    {
        return false;
    }

    if(last_visited == last_path)
    {
        return false;
    }
    else
    {
        return true;
    }
}

aco_dist_t
_forward_op_dist(const RealObject    *obj)
{
    return obj->npath-1;
}

aco_id_t
_forward_op_previous(const RealObject    *obj)
{
    int npath           = obj->npath;

    if(npath == 1)
    {
        return ACO_ID_WRONG;
    }

    assert(npath-2 >= 0);
    return obj->path[npath-2];
}

aco_id_t
_forward_op_next(const RealObject   *obj)
{
    return ACO_ID_WRONG;
}


/*==============================================================================
 * Virtual Function Implementations
 *==============================================================================*/
void
_backward_op_arrived_at(RealObject       *obj,
                       aco_id_t         cur)
{
    obj->cur_ttl--;
    _add_walk(obj, cur);

    // 만약 abort 발생 조건은,
    // 개미가 path대로 돌아가지 않을 경우이다.
    // Ex) 1 - 2 - 3 - 2 - 5 - 2 - 1
    assert(cur == obj->path[obj->npath - 1 - obj->ini_ttl + obj->cur_ttl]);
}

bool
_backward_op_is_backtracked(const RealObject     *obj)
{
    return false;
}

aco_dist_t
_backward_op_dist(const RealObject       *obj)
{
    return obj->ini_ttl - obj->cur_ttl;
}

aco_id_t
_backward_op_previous(const RealObject       *obj)
{
    if(obj->ini_ttl == obj->cur_ttl)
    {
        return ACO_ID_WRONG;
    }
    else
    {
        return obj->path[obj->npath - obj->ini_ttl + obj->cur_ttl];
    }
}

aco_id_t
_backward_op_next(const RealObject      *obj)
{
    int lst_idx = obj->npath - 1;
    int diff    = obj->ini_ttl - obj->cur_ttl;
    int cur_idx = lst_idx - diff;

    if(cur_idx == 0)
    {
        return ACO_ID_WRONG;
    }
    return obj->path[cur_idx - 1];
}

