#ifndef ANT_H
#define ANT_H

#include "ant-def.h"
#include "ant-obj.h"
#include "fon/packet_if.h"

typedef struct _AcoTable    AcoTable;

typedef struct _Ant
{
    AcoTable*   table;
    AntObject*  obj;
}Ant;

typedef void (*AntCallbackLogger)(const Ant* ant);

Ant*    ant_factory         (int type, int source, int destination, AcoTable* table);
Ant*    ant_restore         (const packet* pkt, AcoTable* table);
void    ant_unref           (Ant* ant);
void    ant_send            (Ant* ant);
void    ant_callback        (Ant* ant);
void    ant_callback_logger_set(AntCallbackLogger logger);
AntCallbackLogger ant_callback_logger_get();


#endif /* ANT_H */
