#include <stdio.h>
#include "aco-table.h"

AcoTable* test_new()
{
    AcoTable* table = aco_table_new(
            10,     // host id
            0,      // min pheromone
            100,    // max pheromone
            30);    // max endurance

    aco_table_add_col(table, 0);
    aco_table_add_col(table, 1);
    aco_table_add_col(table, 2);

    aco_table_add_row(table, 3);
    aco_table_add_row(table, 4);
    aco_table_add_row(table, 5);

    return table;
}

void test_iter(AcoTable* table, int target_id)
{
    AcoTableIter iter;

    printf("I want to iterate, target_id is %d\n", target_id);

    if(!aco_table_iter_begin(table, target_id, &iter))
    {
        printf("There is no target_id %d in the given table\n\n", target_id);
        return;
    }

    do
    {
        printf("target_id %d, neigh_id %d\n", iter.value.target_id, iter.value.neigh_id);
    }
    while(aco_table_iter_next(table, &iter));

    printf("\n");

    return;
}

int main(int argc, char **argv) {

    AcoTable* table = test_new();

    aco_table_print_all(table);

    test_iter(table, 0);
    test_iter(table, 1);
    test_iter(table, 2);

    test_iter(table, 3);
    test_iter(table, 4);
    test_iter(table, 5);

    printf("Test Successful\n");

    return 0;
}
