#ifndef FON_TYPES_H
#define FON_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <arpa/inet.h>

// FON node identifier
typedef in_addr_t           fon_id_t;
typedef in_addr_t           fon_id_packed_t;
#define FON_ID_LEN          (sizeof(fon_id_packed_t)*8)
#define FON_ID_WRONG        ((fon_id_t)~0)
#define FON_ID_PACK(X)      (X)
#define FON_ID_UNPACK(X)    (X)

// FON Function Type
typedef int                 fon_type_t;
typedef uint16_t            fon_type_packed_t;
#define FON_TYPE_LEN        (sizeof(fon_type_packed_t)*8)
#define FON_TYPE_WRONG      (-1)
#define FON_TYPE_MAX        ((1<<FON_TYPE_LEN)-1)
#define FON_TYPE_PACK(X)    htons(X)
#define FON_TYPE_UNPACK(X)  ntohs(X)

// payload length
typedef int                 fon_len_t;
typedef uint16_t            fon_len_packed_t;
#define FON_LEN_LEN         (sizeof(fon_len_packed_t)*8)
#define FON_LEN_WRONG       (-1)
#define FON_LEN_MAX         ((1<<FON_LEN_LEN)-1)
#define FON_LEN_PACK(X)     htons(X)
#define FON_LEN_UNPACK(X)   ntohs(X)

// distance between nodes(# of hops)
typedef int                 fon_dist_t;
#define FON_DIST_LEN        (sizeof(fon_dist_t)*8)
#define FON_DIST_WRONG      (-1)
#define FON_DIST_MAX        ((1<<FON_DIST_LEN)-1)
#define FON_DIST_PACK(X)    htons(X)
#define FON_DIST_UNPACK(X)  ntohs(X)

#endif // FON_TYPES_H

