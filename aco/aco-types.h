/** @file aco-types.h
 *  @brief Basic type for ACO
 *  @author Sim Young-Bo
 */
#ifndef ACO_TYPES_H
#define ACO_TYPES_H

#include <stdbool.h>

typedef double          aco_ph_t;       //< concentration of pheromone
typedef int             aco_id_t;       //< Node identifier
typedef aco_id_t*       aco_ids_t;      //< Array of id, including EOF(-1)
typedef int             aco_dist_t;     //< Path(or Link) Distance.

#endif
