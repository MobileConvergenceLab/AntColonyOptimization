#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "ant-obj.h"
#include "ant-def.h"

/*==============================================================================
 * Private Declaration
 *==============================================================================*/
#define _INVALID_VALUE                  (-1)
#define ANT_OBJ_MAXIMUM_ARR_SIZE        (ANT_MAXIMUM_TTL)
#define _MAGIC_INDEX                    (ANT_OBJ_MAXIMUM_ARR_SIZE)

static const char _DirectionString[][64] = {
    "ANT_OBJ_DIRECTION_FORWARD",
    "ANT_OBJ_DIRECTION_BACKWARD",
    "ANT_OBJ_DIRECTION_MAX",
    "ANT_OBJ_DIRECTION_INVALID"
};

typedef struct _RealObject RealObject;
typedef struct _AntObjectOp AntObjectOp;

struct _AntObjectOp
{
    void (*op_arrived_at)       (RealObject *obj, int cur_id);
    bool (*op_is_backtracked)   (const RealObject *obj);
    int  (*op_nhops)            (const RealObject *obj);
    int  (*op_previous)         (const RealObject *obj);
    int  (*op_backward_next)    (const RealObject *obj);
};

struct _RealObject {
    int             source;
    int             destination;
    int             type;

    int             direction;
    int             ini_ttl;
    int             cur_ttl;
    int             nmemory;
    int             npath;
    int             nvisited;
    int             nvhops;
    int             memory[ANT_OBJ_MAXIMUM_ARR_SIZE+1];
    int             path[ANT_OBJ_MAXIMUM_ARR_SIZE+1];
    int             visited[ANT_OBJ_MAXIMUM_ARR_SIZE+1];
    int             vhops[ANT_OBJ_MAXIMUM_ARR_SIZE+1];
    AntObjectOp     op;
};


typedef struct __attribute__((packed)) _AntObjectMarshalled {
    uint16_t    source;
    uint16_t    destination;
    uint16_t    type;
    uint16_t    direction;
    uint16_t    ini_ttl;
    uint16_t    cur_ttl;
    uint16_t    nmemory;
    uint16_t    npath;
    uint16_t    nvisited;
    uint16_t    nvhops;
    uint16_t    data[];
} AntObjectMarshalled;

/*==============================================================================
 * Virtual Function Declaration
 *==============================================================================*/
void op_forward_arrived_at          (RealObject *obj, int cur_id);
bool op_forward_is_backtracked      (const RealObject *obj);
int  op_forward_nhops               (const RealObject *obj);
int  op_forward_previous            (const RealObject *obj);
int  op_forward_backward_next       (const RealObject *obj);

void op_backward_arrived_at         (RealObject *obj, int cur_id);
bool op_backward_is_backtracked     (const RealObject *obj);
int  op_backward_nhops              (const RealObject *obj);
int  op_backward_previous           (const RealObject *obj);
int  op_backward_backward_next      (const RealObject *obj);

static const AntObjectOp AntObjectOps[ANT_OBJ_DIRECTION_MAX] = {
    {
        .op_arrived_at          = op_forward_arrived_at,
        .op_is_backtracked      = op_forward_is_backtracked,
        .op_nhops               = op_forward_nhops,
        .op_previous            = op_forward_previous,
        .op_backward_next       = op_forward_backward_next,
    },

    {
        .op_arrived_at          = op_backward_arrived_at,
        .op_is_backtracked      = op_backward_is_backtracked,
        .op_nhops               = op_backward_nhops,
        .op_previous            = op_backward_previous,
        .op_backward_next       = op_backward_backward_next,
    }

};

/*==============================================================================
 * Private Function Implementations
 *==============================================================================*/
static void _init_array(int *array)
{
    for(int i=0; i< ANT_OBJ_MAXIMUM_ARR_SIZE+1; i++)
    {
        array[i] = _INVALID_VALUE;
    }
}

static inline void _add_array(int *array, int *len, int value)
{
    if(*len == ANT_OBJ_MAXIMUM_ARR_SIZE)
    {
        /* exceed array boundary */
        abort();
    }
    array[(*len)++] = value;
}
#define _add_path(obj, val)                             _add_array((obj->path),     &(obj->npath),      (val))
#define _add_memory(obj, val)                           _add_array((obj->memory),   &(obj->nmemory),    (val))
#define _add_visited(obj, val)                          _add_array((obj->visited),  &(obj->nvisited),   (val))
#define _add_vhops(obj, val)                            _add_array((obj->vhops),    &(obj->nvhops),     (val))


static inline void _pop_array(int *array, int *len)
{
    if(*len == 0)
    {
        /* exceed array boundary */
        abort();
    }
    array[--(*len)] = _INVALID_VALUE;
}
#define _pop_path(obj)                                  _pop_array((obj->path),     &(obj->npath))
#define _pop_memory(obj)                                _pop_array((obj->memory),   &(obj->nmemory))
#define _pop_visited(obj)                               _pop_array((obj->visited),  &(obj->nvisited))
#define _pop_vhops(obj)                                 _pop_array((obj->vhops),    &(obj->nvhops))

static void _marshalling(uint16_t data[], int *data_index, const int arr[], int arr_len)
{
    for(int i=0;
        i<arr_len;
        i++, (*data_index)++)
    {
        data[*data_index] = htons(arr[i]);
    }
}
#define _marshalling_memory(obj, data, data_index)      _marshalling(data,  data_index, obj->memory,    obj->nmemory)
#define _marshalling_path(obj, data, data_index)        _marshalling(data,  data_index, obj->path,      obj->npath)
#define _marshalling_visited(obj, data, data_index)     _marshalling(data,  data_index, obj->visited,   obj->nvisited)
#define _marshalling_vhops(obj, data, data_index)       _marshalling(data,  data_index, obj->vhops,     obj->nvhops)

static void _demarshalling(const uint16_t data[], int *data_index, int arr[], int arr_len)
{
    for(int i=0;
        i<arr_len;
        i++, (*data_index)++)
    {
        arr[i] = ntohs(data[*data_index]);
    }
}
#define _demarshalling_memory(obj, data, data_index)    _demarshalling(data,  data_index, obj->memory,    obj->nmemory)
#define _demarshalling_path(obj, data, data_index)      _demarshalling(data,  data_index, obj->path,      obj->npath)
#define _demarshalling_visited(obj, data, data_index)   _demarshalling(data,  data_index, obj->visited,   obj->nvisited)
#define _demarshalling_vhops(obj, data, data_index)     _demarshalling(data,  data_index, obj->vhops,     obj->nvhops)


static int _find_visited_idx(const int *visited, int id)
{
    // temporarily set and rollback
    int idx = -1;

    *(int*)(visited+_MAGIC_INDEX) = id;
    while(visited[++idx] != id);
    *(int*)(visited+_MAGIC_INDEX) = _INVALID_VALUE;

    return idx;
}

static inline size_t _calc_marshalled_size(const RealObject *obj)
{
    return
        sizeof(AntObjectMarshalled) +
        sizeof(uint16_t)*(obj->nmemory + obj->npath + obj->nvisited + obj->nvhops);
}

static void _real_object_init(RealObject *obj, int source, int destination, int type, int direction, int ini_ttl, int cur_ttl)
{
    obj->source         = source;
    obj->destination    = destination;
    obj->type           = type;
    obj->direction      = direction;
    obj->ini_ttl        = ini_ttl;
    obj->cur_ttl        = cur_ttl;
    obj->op             = AntObjectOps[direction];

    obj->nmemory        = 0;
    obj->npath          = 0;
    obj->nvisited       = 0;
    obj->nvhops         = 0;

    _init_array(obj->memory);
    _init_array(obj->path);
    _init_array(obj->visited);
    _init_array(obj->vhops);

    _add_memory(obj, source);
    _add_path(obj, source);
    _add_visited(obj, source);
    _add_vhops(obj, 0);
}

static RealObject* _real_object_new(int source, int destination, int type, int direction, int ini_ttl, int cur_ttl)
{
    RealObject* obj     = malloc(sizeof(RealObject));

    _real_object_init(obj, source, destination, type, direction, ini_ttl, cur_ttl);

    return obj;
}

/*==============================================================================
 * Public Function Implementations
 *==============================================================================*/
AntObject* ant_object_new(int source, int destination, int type, int ini_ttl)
{
    return (AntObject*)_real_object_new(source,
                                        destination,
                                        type,
                                        ANT_OBJ_DIRECTION_FORWARD,
                                        ini_ttl,
                                        ini_ttl);
}

void ant_object_unref(AntObject *fobj)
{
    free(fobj);
}

void ant_object_print(const AntObject *fobj)
{
    const RealObject* obj    = (const RealObject*)fobj;
    const int   *memory      = obj->memory;
    const int   *visited    = obj->visited;
    const int   *vhops      = obj->vhops;
    const int   *path       = obj->path;

    printf("source:      %-4d\n"
           "destination: %-4d\n"
           "direction:   %s\n"
           "type:        %-4d\n"
           "cur_ttl:     %-4d\n",
           obj->source,
           obj->destination,
           _DirectionString[obj->direction],
           obj->type,
           obj->cur_ttl);

    printf("memory:      ");
    for(int i=0; i< obj->nmemory; i++) {
        printf("%-4d", memory[i]);
    }
    printf("\n");

    printf("visited:     ");
    for(int i=0; i< obj->nvisited; i++) {
        printf("%-4d", visited[i]);
    }
    printf("\n");

    printf("vhops:       ");
    for(int i=0; i< obj->nvhops; i++) {
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
        const AntObject    *fobj)
{
    const RealObject   *obj     = (const RealObject*)fobj;
    const int          *path    = obj->path;

    for(int i=0; i< obj->npath; i++) {
        printf("%2d  ", path[i]);
    }
    printf("\n");
}

void
ant_object_print_memory(
        const AntObject    *fobj)
{
    const RealObject    *obj        = (const RealObject*)fobj;
    const int           *memory     = obj->memory;

    for(int i=0; i< obj->nmemory; i++) {
        fprintf(stderr, "%4d", memory[i]);
    }
    fprintf(stderr, "\n");

}

void
ant_object_print_dbg_hops(
        const AntObject    *fobj)
{
    const RealObject    *obj        = (const RealObject*)fobj;
    fprintf(stderr, "%4d\n", obj->vhops[obj->nvisited-1]);
}

void ant_object_marshalling(const AntObject *fobj, void *out_buf, int *out_buflen)
{
    const RealObject    *in_obj     = (const RealObject*)fobj;
    AntObjectMarshalled*
                        marshalled  = out_buf;
    size_t              len         = _calc_marshalled_size(in_obj);

    if(*out_buflen < len) {
        /* Not enough memry */
        abort();
    }
    else {
        *out_buflen = len;
    }

    marshalled->source      = htons(in_obj->source);
    marshalled->destination = htons(in_obj->destination);
    marshalled->type        = htons(in_obj->type);
    marshalled->direction   = htons(in_obj->direction);
    marshalled->ini_ttl     = htons(in_obj->ini_ttl);
    marshalled->cur_ttl     = htons(in_obj->cur_ttl);
    marshalled->nmemory     = htons(in_obj->nmemory);
    marshalled->npath       = htons(in_obj->npath);
    marshalled->nvisited    = htons(in_obj->nvisited);
    marshalled->nvhops      = htons(in_obj->nvhops);

    int data_index  = 0;
    _marshalling_memory     (in_obj, marshalled->data, &data_index);
    _marshalling_path       (in_obj, marshalled->data, &data_index);
    _marshalling_visited    (in_obj, marshalled->data, &data_index);
    _marshalling_vhops      (in_obj, marshalled->data, &data_index);

    return;
}

AntObject* ant_object_demarshalling(const void *in_buf, int buflen)
{
    const AntObjectMarshalled*
                    marshalled = in_buf;

    RealObject*         obj         = _real_object_new(ntohs(marshalled->source),
                                        ntohs(marshalled->destination),
                                        ntohs(marshalled->type),
                                        ntohs(marshalled->direction),
                                        ntohs(marshalled->ini_ttl),
                                        ntohs(marshalled->cur_ttl)
                                        );

    obj->nmemory    = ntohs(marshalled->nmemory);
    obj->npath      = ntohs(marshalled->npath);
    obj->nvisited   = ntohs(marshalled->nvisited);
    obj->nvhops     = ntohs(marshalled->nvhops);

    int data_index = 0;
    _demarshalling_memory     (obj, marshalled->data, &data_index);
    _demarshalling_path       (obj, marshalled->data, &data_index);
    _demarshalling_visited    (obj, marshalled->data, &data_index);
    _demarshalling_vhops      (obj, marshalled->data, &data_index);

    return (AntObject*)obj;
}

bool ant_object_is_visited(const AntObject *fobj, int id)
{
    const RealObject *obj = (const RealObject*)fobj;
    int         idx         = -1;
    const int   *visited    = obj->visited;

    idx = _find_visited_idx(visited, id);

    return idx != _MAGIC_INDEX;
}

void ant_object_arrived_at(AntObject *fobj, int cur_id)
{
    RealObject *obj = (RealObject*)fobj;
    obj->op.op_arrived_at(obj, cur_id);
}

bool ant_object_change_direction(AntObject *fobj)
{
    RealObject *obj = (RealObject*)fobj;

    if(obj->direction == ANT_OBJ_DIRECTION_FORWARD &&
        obj->destination == obj->memory[obj->nmemory - 1])
    {
        int tmp         = obj->source;
        obj->source     = obj->destination;
        obj->destination = tmp;

        obj->direction  = ANT_OBJ_DIRECTION_BACKWARD;
        obj->cur_ttl    = obj->ini_ttl;
        obj->op         = AntObjectOps[ANT_OBJ_DIRECTION_BACKWARD];

        return true;
    }

    return false;
}

bool
ant_object_is_backtracked(
        const AntObject    *fobj)
{
    const RealObject *obj = (const RealObject*)fobj;
    return obj->op.op_is_backtracked(obj);
}

int
ant_object_nhops(const AntObject *fobj)
{
    const RealObject *obj = (const RealObject*)fobj;
    return obj->op.op_nhops(obj);
}

int ant_object_get_direction(const AntObject *fobj)
{
    const RealObject *obj = (const RealObject*)fobj;
    return obj->direction;
}

int ant_object_get_ttl(const AntObject *fobj)
{
    const RealObject *obj = (const RealObject*)fobj;
    return obj->cur_ttl;
}

int ant_object_previous(const AntObject *fobj)
{
    const RealObject *obj = (const RealObject*)fobj;
    return obj->op.op_previous(obj);
}

int ant_object_backward_next(const AntObject *fobj)
{
    const RealObject *obj = (const RealObject*)fobj;
    return obj->op.op_backward_next(obj);
}

int ant_object_cmp(const AntObject* fobj1, const AntObject* fobj2)
{
    return memcmp(fobj1, fobj2, sizeof(RealObject));
}

int ant_object_from(const AntObject* fobj)
{
    const RealObject *obj = (const RealObject*)fobj;

    if(obj->nmemory != 1)
    {
        return obj->memory[obj->nmemory-2];
    }
    else
    {
        return _INVALID_VALUE;
    }
}

/*==============================================================================
 * Virtual Function Implementations
 *==============================================================================*/

void op_forward_arrived_at(RealObject *obj, int cur_id)
{
    /* 수신한 패킷이 backtrack 된것인지 아닌지에 따라 패킷을 업데이트한다. */
    if(ant_object_is_visited((AntObject*)obj, cur_id))
    {
        obj->cur_ttl--;
        _pop_path(obj);
        _add_memory(obj, cur_id);
    }
    else
    {
        int pre_id = obj->path[obj->npath-1];
        int pre_visited_idx = -1;

        obj->cur_ttl--;

        _add_visited(obj, cur_id);
        _add_path(obj, cur_id);
        _add_memory(obj, cur_id);

        pre_visited_idx = _find_visited_idx(obj->visited, pre_id);
        _add_vhops(obj, obj->vhops[pre_visited_idx] + 1);
    }
}

bool op_forward_is_backtracked(const RealObject *obj)
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

int  op_forward_nhops(const RealObject *obj)
{
    return obj->npath-1;
}

int  op_forward_previous(const RealObject *obj)
{
    int npath           = obj->npath;

    if(npath == 1)
    {
        return _INVALID_VALUE;
    }

    assert(npath-2 >= 0);
    return obj->path[npath-2];
}

int  op_forward_backward_next(const RealObject *obj)
{
    return _INVALID_VALUE;
}


/*==============================================================================
 * Virtual Function Implementations
 *==============================================================================*/
void op_backward_arrived_at(RealObject *obj, int cur_id)
{
    obj->cur_ttl--;
    _add_memory(obj, cur_id);

    // 만약 abort 발생 조건은,
    // 개미가 path대로 돌아가지 않을 경우이다.
    // Ex) 1 - 2 - 3 - 2 - 5 - 2 - 1
    assert(cur_id == obj->path[obj->npath - 1 - obj->ini_ttl + obj->cur_ttl]);
}

bool op_backward_is_backtracked(const RealObject *obj)
{
    return false;
}

int  op_backward_nhops(const RealObject *obj)
{
    return obj->ini_ttl - obj->cur_ttl;
}

int  op_backward_previous(const RealObject *obj)
{
    if(obj->ini_ttl == obj->cur_ttl)
    {
        return _INVALID_VALUE;
    }
    else
    {
        return obj->path[obj->npath - obj->ini_ttl + obj->cur_ttl];
    }
}

int  op_backward_backward_next(const RealObject *obj)
{
    int lst_idx = obj->npath - 1;
    int diff    = obj->ini_ttl - obj->cur_ttl;
    int cur_idx = lst_idx - diff;

    if(cur_idx == 0)
    {
        return _INVALID_VALUE;
    }
    return obj->path[cur_idx - 1];
}


