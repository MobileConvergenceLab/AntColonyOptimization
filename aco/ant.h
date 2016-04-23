#ifndef ANT_H
#define ANT_H

#include "ant-def.h"
#include "ant-obj.h"
#include "fon/packet_if.h"
#include "aco-table.h"

typedef struct _AcoTable    AcoTable;

typedef struct _Ant
{
    AcoTable*   table;
    AntObject*  obj;
    int         type;

    // For internal Varialbles
    const char  data[];
}Ant;

typedef void (*AntLogger)(const Ant* ant);

Ant*        ant_factory         (int                type,
                                 aco_id_t           source,
                                 aco_id_t           destination,
                                 AcoTable           *table);
void        ant_unref           (Ant                *ant);
void        ant_marshalling     (const Ant          *ant,
                                 void               **pos,
                                 int                *remain);
Ant*        ant_demarshalling   (const void         *buf,
                                 int                len,
                                 AcoTable           *table);
int         ant_cmp             (const Ant          *ant1,
                                 const Ant          *ant2);
void        ant_send            (Ant                *ant);
void        ant_callback        (Ant                *ant);
void        ant_logger_set      (AntLogger          logger);
AntLogger   ant_logger_get      ();

#endif /* ANT_H */
