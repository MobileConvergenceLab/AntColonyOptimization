#ifndef ANT_MODEL_H
#define ANT_MODEL_H

#include "ant-type.h"

/**
 * @ph:         the pheromone value of the given pair(target_id, neigh_id).
 * @global_min: global_min of the given target_id
 * @local_min:  local_min of the given pair(target_id, neigh_id).
 * @nhops:      the length of the path that the given ant structed.
 */
typedef void (*AntModel)(pheromone_t *ph, int global_min, int local_min, int nhops);

void ant_density_model       (pheromone_t *ph, int global_min, int local_min, int nhops);
void ant_system_model        (pheromone_t *ph, int global_min, int local_min, int nhops);
void ant_colony_system_model (pheromone_t *ph, int global_min, int local_min, int nhops);
void ant_local_model         (pheromone_t *ph, int global_min, int local_min, int nhops);
void ant_normalizing_model   (pheromone_t *ph, int global_min, int local_min, int nhops);

#endif
