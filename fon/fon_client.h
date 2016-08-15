#ifndef FON_CLIENT_H
#define FON_CLIENT_H

#include "fon_types.h"

typedef struct _FonClient {
    /* synchronous channel(connection-oriented) */
    int                 sync_sock;

    /* server sync port */
    int                 sync_port;

    /* asynchronous channel(connection-less) */
    int                 async_sock;

    /* server async Port */
    int                 async_port;

    /* FON type */
    fon_type_t          func_type;
} FonClient;

#endif // FON_CLIENT_H
