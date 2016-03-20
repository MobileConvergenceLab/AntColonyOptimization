/**
 * ANT Colony 알고리즘에서 쓰일 패킷을 정의.
 */
#ifndef ANT_OBJ_H
#define ANT_OBJ_H

#include <stdbool.h>
#include <stddef.h>

// The maximum value is Depend on MTU.
#define ANT_OBJ_MAXIMUM_ARR_SIZE        (128)

// Direction constant
typedef enum _AntObjectDirection
{
    ANT_OBJ_DIRECTION_FORWARD,
    ANT_OBJ_DIRECTION_BACKWARD,
    ANT_OBJ_DIRECTION_MAX,
    ANT_OBJ_DIRECTION_INVALID
} AntObjectDirection;

/**
 * @source: 최초에 패킷을 생성한 자
 * @destination: 개미가 찾을 대상
 * @type: 개미의 타입
 */
typedef struct _AntObject {
    int             source;
    int             destination;
    int             type;
    char            data[];
} AntObject;

AntObject* ant_object_new           (int source, int destination, int type, int ini_ttl);
void ant_object_unref               (AntObject* obj);
void ant_object_marshalling         (const AntObject* in_obj, void* out_buf, int* out_buflen);
AntObject* ant_object_demarshalling (const void* in_buf, int buflen);
void ant_object_arrived_at          (AntObject* obj, int cur_id);
bool ant_object_change_direction    (AntObject* obj);
bool ant_object_is_visited          (const AntObject* obj, int id);
bool ant_object_is_backtracked      (const AntObject* obj);
int ant_object_nhops                (const AntObject* obj);
int ant_object_get_direction        (const AntObject* obj);
int ant_object_get_ttl              (const AntObject* obj);
int ant_object_previous             (const AntObject* obj);
int ant_object_backward_next        (const AntObject* obj);
int ant_object_from                 (const AntObject* obj);
int ant_object_cmp                  (const AntObject* obj1, const AntObject* obj2);
void ant_object_print               (const AntObject* obj);
void ant_object_print_path          (const AntObject* obj);
void ant_object_print_memory        (const AntObject* obj);
void ant_object_print_dbg_hops      (const AntObject* obj);

#endif /* ANT_OBJ_H */

