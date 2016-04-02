#ifndef ANT_DEF_H
#define ANT_DEF_H

#include <stddef.h>
#include <limits.h>

#define     FON_FUNC_TYPE_ACO       (0x8033)

enum ANT_TYPE {
    ANT_TYPE_FLOOD,
    ANT_TYPE_ONEWAY,
    ANT_TYPE_FORWARD,
    ANT_TYPE_WRONG,
};

static const char ANT_TYPE_STR[][64] = {
    "ANT_TYPE_FLOOD",
    "ANT_TYPE_ONEWAY",
    "ANT_TYPE_FORWARD",
    "ANT_TYPE_WRONG",
};

#include "ant-parameter.h"

#endif /* ANT_DEF_H */
