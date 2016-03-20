#ifndef ANT_DEF_H
#define ANT_DEF_H

#include "aco-table-def.h"
#include "ant-obj.h"
#include "main-def.h"

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

#include "parameters.h"

#endif /* ANT_DEF_H */
