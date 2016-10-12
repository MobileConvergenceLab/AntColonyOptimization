/** @file aco-types.h
 *  @brief Basic type for ACO
 *  @author Sim Young-Bo
 */
#ifndef ACO_TYPES_H
#define ACO_TYPES_H

#include <stdbool.h>
#include <arpa/inet.h>
#include <fon/fon_types.h>

// concentration of pheromone
typedef double                      aco_ph_t;

// Node identifier
typedef fon_id_t                    aco_id_t;
typedef fon_id_packed_t             aco_id_packed_t;
#define ACO_ID_LEN                  FON_ID_LEN
#define ACO_ID_WRONG                FON_ID_WRONG
#define ACO_ID_PACK(X)              FON_ID_PACK(X)
#define ACO_ID_UNPACK(X)            FON_ID_UNPACK(X)

// Array of ids, including EOF(ACO_ID_WRONG)
typedef aco_id_t*                   aco_ids_t;

// Path(or Link) Distance.
typedef int                         aco_dist_t;
typedef uint16_t                    aco_dist_packed_t;
#define ACO_DIST_LEN                (sizeof(aco_dist_packed_t)*8)
// The maximum value is Depend on MTU.
// Because ant packet size depend on this value
#define ACO_DIST_MAX                (128)
#define ACO_DIST_WRONG              (ACO_DIST_MAX + 1)
#define ACO_DIST_PACK(X)            (htons(X))
#define ACO_DIST_UNPACK(X)          (ntohs(X))

typedef int                         aco_ttl_t;
typedef uint16_t                    aco_ttl_packed_t;
#define ACO_TTL_LEN                 (sizeof(aco_ttl_packed_t)*8)
#define ACO_TTL_WRONG               (-1)
#define ACO_TTL_MAX                 ((1 << ACO_TTL_LEN) - 1)
#define ACO_TTL_PACK(X)             (htons(X))
#define ACO_TTL_UNPACK(X)           (ntohs(X))

typedef int                         aco_direction_t;
typedef uint16_t                    aco_direction_packed_t;
#define ACO_DIRECTION_LEN           (sizeof(aco_direction_packed_t)*8)
#define ACO_DIRECTION_WRONG         (-1)
#define ACO_DIRECTION_BACKWARD      (0)
#define ACO_DIRECTION_FORWARD       (1)
#define ACO_DIRECTION_PACK(X)       (htons(X))
#define ACO_DIRECTION_UNPACK(X)     (ntohs(X))

typedef int                         aco_len_t;
typedef uint16_t                    aco_len_packed_t;
#define ACO_LEN_LEN                 (sizeof(aco_len_packed_t)*8)
#define ACO_LEN_WRONG               (-1)
#define ACO_LEN_MAX                 ((1 << ACO_LEN_LEN) - 1)
#define ACO_LEN_PACK(X)             (htons(X))
#define ACO_LEN_UNPACK(X)           (ntohs(X))

typedef int                         aco_cycle_t;
typedef uint16_t                    aco_cycle_packed_t;
#define ACO_CYCLE_LEN               (sizeof(aco_cycle_packed_t)*8)
#define ACO_CYCLE_WRONG             (-1)
#define ACO_CYCLE_MAX               ((1 << ACO_CYCLE_LEN) - 1)
#define ACO_CYCLE_PACK(X)           (htons(X))
#define ACO_CYCLE_UNPACK(X)         (ntohs(X))

#endif // ACO_TYPES_H
