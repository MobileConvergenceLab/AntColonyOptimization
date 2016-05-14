#ifndef ANT_MODEL_H
#define ANT_MODEL_H

#include "aco-types.h"

#ifdef __cpluscplus
extern "C" {
#endif

/**
 * @ph:         the pheromone value of the given pair(target_id, neigh_id).
 * @global_min: global_min of the given target_id
 * @local_min:  local_min of the given pair(target_id, neigh_id).
 * @nhops:      the length of the path that the given ant structed.
 */
typedef void (*AntModel)(aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist);

void ant_density_model       (aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist);
void ant_system_model        (aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist);
void ant_colony_system_model (aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist);
void ant_local_model         (aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist);
void ant_normalizing_model   (aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist);

#ifdef __cpluscplus
}
#endif

#endif
