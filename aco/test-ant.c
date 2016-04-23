#include <stdlib.h>

#include "ant.h"

Ant* test_new(aco_id_t source, aco_id_t dest)
{
    Ant         *ant    = NULL;

    ant = ant_factory(ANT_TYPE_TEST,      // type
                      source,             // source
                      dest,               // dest
                      NULL);              // table

    return ant;
}

void test_masharling()
{
    char        buf[1024*4];
    Ant         *ant1   = test_new(0, 1);
    Ant         *ant2   = NULL;
    void        *pos    = buf;
    int         remain  = 1024*4;

    ant_marshalling(ant1,
                    &pos,
                    &remain);

    ant2 = ant_demarshalling(buf,
                             pos - (void*)buf,
                             ant1->table);

    if(ant_cmp(ant1, ant2))
    {
        printf("masharling test fail\n");
        exit(-1);
    }

    printf("masharling test success\n");
}

Ant* do_visit(Ant *ant, aco_id_t id)
{
    char        buf[1024*4];
    Ant*        new_ant = NULL;
    void        *pos    = buf;
    int         remain  = 1024*4;

    ant_marshalling(ant,
                    &pos,
                    &remain);

    new_ant = ant_demarshalling(buf,
                             pos - (void*)buf,
                             NULL);

    ant_object_print(new_ant->obj);
    ant_object_arrived_at(new_ant->obj, id);
    ant_object_change_direction(new_ant->obj);
    ant_object_print(new_ant->obj);
    printf("\n");

    ant_unref(ant);

    return new_ant;
}

void test_visit()
{
    Ant *ant = test_new(0, 4);
    ant = do_visit(ant, 1);
    ant = do_visit(ant, 2);
    ant = do_visit(ant, 3);
    ant = do_visit(ant, 4);
    ant = do_visit(ant, 3);
    ant = do_visit(ant, 2);
    ant = do_visit(ant, 1);
    ant = do_visit(ant, 0);
}

int main()
{
    test_masharling();
    test_visit();

    return 0;
}
