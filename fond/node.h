/**
 * Implementation of (Near) node
 */
#ifndef NODE_H
#define NODE_H

#include <sys/socket.h>
#include <stdint.h>
#include <stdbool.h>
#include "table.h"

typedef struct  _NodePool   NodePool;

typedef union   _packet     packet;
typedef struct  _NearNode   NearNode;
typedef int (*node_op_sendto)(NearNode *n, packet *pkt);

typedef struct _NearNode {
    int32_t             id;
    node_op_sendto      op_sendto;
} NearNode;

enum node_pool_valid {
    NODE_VALID_FALSE,
    NODE_VALID_TRUE,
};

NodePool*       node_pool_create             (table *in_t);
void            node_pool_add                (NodePool *in_pool, NearNode *in_n);
void            node_pool_del                (NodePool *in_pool, int in_id);
NearNode*       node_pool_lookup             (NodePool *in_pool, int in_id);

#endif /* NODE_H */
