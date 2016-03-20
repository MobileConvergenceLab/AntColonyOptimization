/**
 * Written by Sim Young-Bo <twinwings1111@gmail.com>
 */
#include <stdio.h>
#include "int-pair.h"

void test_add(IntPair* pair, int first, int second)
{
    int_pair_add(pair, first, second);
    g_print("Add <%d, %d> pair\n", first, second);
    
}

void test_find_second_by_first(IntPair* pair, int first)
{
    int second;
    int ret;

    ret = int_pair_get_second(pair, first, &second);

    g_print("Try Find by first key(%d) : %s\n", first, ret ? "Found" : "Not Found");

}

void test_find_first_by_second(IntPair* pair, int second)
{
    int first;
    int ret;

    ret = int_pair_get_first(pair, second, &first);

    g_print("Try Find by second key(%d) : %s\n", second, ret ? "Found" : "Not Found");

}

int main(int argc, char **argv) {
    IntPair* pair = int_pair_new();

    test_add(pair, 0,0);
    test_add(pair, 1,10);
    test_add(pair, 2,11);
    test_add(pair, 3,12);
    test_add(pair, 4,13);
    g_print("\n");

    test_find_second_by_first(pair, 1);
    test_find_second_by_first(pair, 2);
    test_find_second_by_first(pair, 3);
    test_find_second_by_first(pair, 4);
    test_find_second_by_first(pair, 10);
    g_print("\n");


    test_find_first_by_second(pair, 10);
    test_find_first_by_second(pair, 11);
    test_find_first_by_second(pair, 12);
    test_find_first_by_second(pair, 13);
    test_find_first_by_second(pair, 1);
    g_print("\n");

    return 0;
}
