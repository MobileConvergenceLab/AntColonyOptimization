#ifndef MAIN_H
#define MAIN_H

#include <glib.h>
#include "table.h"
#include "node.h"
#include "cmanager.h"

typedef struct _ether_link  ether_link;
typedef struct _bt_link     bt_link;

typedef struct {
    int             listen_sock;
    int             deliver_sock;
    int             host_id;
    GMainLoop*      loop;
    GMainContext*   context;
    table*          table;
    NodePool*       pool;
    GTree*          cmanager;
    ether_link*     ether_link;
} FonCoreObject;

/* defined in Main.c */
void delivery_to_client_function(FonCoreObject* obj, packet *pkt);

/* defined in ether_link.c (Too big)*/
ether_link* ether_link_create(FonCoreObject *obj);

#endif /* MAIN_H */
