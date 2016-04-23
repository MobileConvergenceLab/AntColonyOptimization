#include "ant-obj.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void my_object_assert(AntObject* obj, int in_direction, int in_previous, int in_from, int in_next, int in_nhops, bool in_is_backtrackted)
{
    int direction       = ant_object_get_direction(obj);
    int previous        = ant_object_previous(obj);
    int from            = ant_object_from(obj);
    int next            = ant_object_backward_next(obj);
    int nhops           = ant_object_dist(obj);
    bool is_backtrackted = ant_object_is_backtracked(obj);

    assert(direction    == in_direction);
    assert(previous     == in_previous);
    assert(from         == in_from);
    assert(next         == in_next);
    assert(nhops        == in_nhops);
    assert(is_backtrackted == in_is_backtrackted);
}

AntObject* ant_object_new_test()
{
    int direction;
    int previous;
    int from;
    int next;
    int nhops;
    bool is_backtrackted;

    AntObject *obj = ant_object_new(0,                          // SOURCE
                                    5,                          // DEST
                                    24);                        // INI TTL
    direction       = ANT_OBJ_DIRECTION_FORWARD;
    previous        = -1;
    from            = -1;
    next            = -1;
    nhops           = 0;
    is_backtrackted = false;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_arrived_at(obj, 1);
    direction       = ANT_OBJ_DIRECTION_FORWARD;
    previous        = 0;
    from            = 0;
    next            = -1;
    nhops           = 1;
    is_backtrackted = false;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_arrived_at(obj, 2);
    direction       = ANT_OBJ_DIRECTION_FORWARD;
    previous        = 1;
    from            = 1;
    next            = -1;
    nhops           = 2;
    is_backtrackted = false;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_arrived_at(obj, 3);
    direction       = ANT_OBJ_DIRECTION_FORWARD;
    previous        = 2;
    from            = 2;
    next            = -1;
    nhops           = 3;
    is_backtrackted = false;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_arrived_at(obj, 4);
    direction       = ANT_OBJ_DIRECTION_FORWARD;
    previous        = 3;
    from            = 3;
    next            = -1;
    nhops           = 4;
    is_backtrackted = false;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_arrived_at(obj, 3);
    direction       = ANT_OBJ_DIRECTION_FORWARD;
    previous        = 2;
    from            = 4;
    next            = -1;
    nhops           = 3;
    is_backtrackted = true;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_arrived_at(obj, 5);
    direction       = ANT_OBJ_DIRECTION_FORWARD;
    previous        = 3;
    from            = 3;
    next            = -1;
    nhops           = 4;
    is_backtrackted = false;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_change_direction(obj);
    direction       = ANT_OBJ_DIRECTION_BACKWARD;
    previous        = -1;
    from            = 3;
    next            = 3;
    is_backtrackted = false;
    nhops           = 0;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_arrived_at(obj, 3);
    direction       = ANT_OBJ_DIRECTION_BACKWARD;
    previous        = 5;
    from            = 5;
    next            = 2;
    nhops           = 1;
    is_backtrackted = false;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_arrived_at(obj, 2);
    direction       = ANT_OBJ_DIRECTION_BACKWARD;
    previous        = 3;
    from            = 3;
    next            = 1;
    nhops           = 2;
    is_backtrackted = false;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_arrived_at(obj, 1);
    direction       = ANT_OBJ_DIRECTION_BACKWARD;
    previous        = 2;
    from            = 2;
    next            = 0;
    nhops           = 3;
    is_backtrackted = false;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    ant_object_arrived_at(obj, 0);
    direction       = ANT_OBJ_DIRECTION_BACKWARD;
    previous        = 1;
    from            = 1;
    next            = -1;
    nhops           = 4;
    is_backtrackted = false;
    my_object_assert(obj, direction, previous, from, next, nhops, is_backtrackted);

    return obj;
}

void test_masharling(AntObject* obj1)
{
    char buf[2048];
    int remain  = 2048;
    void *pos   = buf;

    AntObject* obj2 = NULL;

    ant_object_marshalling(obj1, &pos, &remain);
    obj2 = ant_object_demarshalling(buf, pos - (void*)buf);

    if(ant_object_cmp(obj1, obj2))
    {
        printf("Test fail\n");
        exit(-1);
    }
}

int main()
{
    AntObject* obj = NULL;

    obj = ant_object_new_test();
    test_masharling(obj);

    printf("Test Successful\n");
    ant_object_print(obj);

    ant_object_unref(obj);
    return 0;
}
