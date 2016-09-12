#ifndef ANT_DEF_H
#define ANT_DEF_H

#include <stddef.h>
#include <limits.h>
#include <stdint.h>

#define     FON_FUNC_TYPE_ACO       (0x8033)

enum ant_type_t {
    ANT_TYPE_FLOOD	= 0,
    ANT_TYPE_ONEWAY	= 1,
    ANT_TYPE_ROUNDTRIP	= 2,
    ANT_TYPE_TEST	= 3,
    ANT_TYPE_WRONG	= 4,
};
typedef uint16_t		ant_type_packed_t;
#define ANT_TYPE_LEN		(sizeof(ant_type_packed_t)*8)
#define ANT_TYPE_PACK(X)	(htons(X))
#define ANT_TYPE_UNPACK(X)	(ntohs(X))

static const char ANT_TYPE_STR[][64] = {
    "ANT_TYPE_FLOOD",
    "ANT_TYPE_ONEWAY",
    "ANT_TYPE_ROUNDTRIP",
    "ANT_TYPE_TEST",
    "ANT_TYPE_WRONG",
};

#include "aco-policies.h"

#endif /* ANT_DEF_H */
