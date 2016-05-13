/** @file ant-obj.h
 *  @brief Implementation for the object for the ant used in Ant Colony Optimization(ACO).
 *  @author Sim Young-Bo
 */

#ifndef ANT_OBJ_H
#define ANT_OBJ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "aco-types.h"

// Direction constant
typedef enum _AntObjectDirection
{
    ANT_OBJ_DIRECTION_FORWARD,
    ANT_OBJ_DIRECTION_BACKWARD,
    ANT_OBJ_DIRECTION_MAX,
    ANT_OBJ_DIRECTION_INVALID
} AntObjectDirection;

typedef struct _AntObject {
    aco_id_t        source;
    aco_id_t        destination;

    const char      data[];         /**< For internal Varialbles. */
} AntObject;

AntObject*      ant_object_new                  (aco_id_t           source,
                                                 aco_id_t           destination,
                                                 int                ini_ttl);
void            ant_object_unref                (AntObject          *obj);
void            ant_object_marshalling          (const AntObject    *obj,
                                                 void               **pos,
                                                 int                *remain);
AntObject*      ant_object_demarshalling        (const void         *buf,
                                                 int                buflen);
void            ant_object_arrived_at           (AntObject          *obj,
                                                 aco_id_t           cur);
bool            ant_object_change_direction     (AntObject          *obj);
bool            ant_object_is_visited           (const AntObject    *obj,
                                                 aco_id_t           id);
bool            ant_object_is_backtracked       (const AntObject    *obj);
aco_dist_t      ant_object_dist                 (const AntObject    *obj);
int             ant_object_get_direction        (const AntObject    *obj);
int             ant_object_get_ttl              (const AntObject    *obj);
aco_id_t        ant_object_previous             (const AntObject    *obj);
aco_id_t        ant_object_backward_next        (const AntObject    *obj);
aco_id_t        ant_object_from                 (const AntObject    *obj);
int             ant_object_cmp                  (const AntObject    *obj1,
                                                 const AntObject    *obj2);
void            ant_object_print                (const AntObject    *obj);
void            ant_object_print_path           (const AntObject    *obj);
void            ant_object_print_memory         (const AntObject    *obj);
void            ant_object_print_dbg_hops       (const AntObject    *obj);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ANT_OBJ_H */
