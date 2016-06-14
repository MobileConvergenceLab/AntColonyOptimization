/** @file aco-types.h
 *  @brief Basic type for ACO
 *  @author Sim Young-Bo
 */
#ifndef ACO_TYPES_H
#define ACO_TYPES_H

#include <stdbool.h>

typedef double          aco_ph_t;           //< concentration of pheromone
typedef int             aco_id_t;           //< Node identifier
typedef aco_id_t*       aco_ids_t;          //< Array of ids, including EOF(-1, ACO_ID_WRONG)
typedef int             aco_dist_t;         //< Path(or Link) Distance.
typedef int             aco_ttl_t;
typedef int             aco_direction_t;

#define ACO_ID_LEN                  (16)
#define ACO_ID_WRONG                (-1)
#define ACO_ID_MAX                  ((1 << ACO_ID_LEN) - 1)

#define ACO_DIST_LEN                (16)
#define ACO_DIST_WRONG              (-1)
#define ACO_DIST_MAX                ((1 << ACO_DIST_LEN) - 1)

#define ACO_TTL_LEN                 (16)
#define ACO_TTL_WRONG               (-1)
#define ACO_TTL_MAX                 ((1 << ACO_TTL_LEN) - 1)

#define ACO_DIRECTION_LEN           (16)
#define ACO_DIRECTION_WRONG         (0)
#define ACO_DIRECTION_BACKWARD      (1)
#define ACO_DIRECTION_FORWARD       (2)

#endif // ACO_TYPES_H
