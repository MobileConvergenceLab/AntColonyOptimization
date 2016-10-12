#ifndef ANT_H
#define ANT_H

#include <fon/fon.h>
#include "ant-def.h"
#include "ant-obj.h"
#include "aco-table.h"

#ifdef __cpluscplus
extern "C" {
#endif

typedef struct _AcoTable    AcoTable;
typedef struct _Ant         Ant;
typedef void (*how_to_send_t)(const Ant* ant, aco_id_t host, aco_id_t neighbor);

struct _Ant
{
    AcoTable    *table;
    AntObject   *obj;

    // 가상함수. 패킷을 어떻게 보낼지 안다.
    how_to_send_t sendto;
    void        *user_data;

    // For internal Varialbles
    const char  data[];
};

typedef void (*AntLogger)(const Ant* ant);

Ant*        ant_factory         (int                type,
                                 aco_id_t           source,
                                 aco_id_t           destination,
                                 AcoTable           *table,
                                 how_to_send_t      sendto,
                                 void               *user_data);
void        ant_unref           (Ant                *ant);
void        ant_marshalling     (const Ant          *ant,
                                 void               **pos,
                                 size_t             *remain);
Ant*        ant_demarshalling   (const void         *buf,
                                 size_t             len,
                                 AcoTable           *table,
                                 how_to_send_t      sendto,
                                 void               *user_data);
int         ant_cmp             (const Ant          *ant1,
                                 const Ant          *ant2);
void        ant_send            (Ant                *ant);
void        ant_callback        (Ant                *ant);
void        ant_logger_set      (AntLogger          logger);
AntLogger   ant_logger_get      ();

#ifdef __cpluscplus
}
#endif

#endif /* ANT_H */
