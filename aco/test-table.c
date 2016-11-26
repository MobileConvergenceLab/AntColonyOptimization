#include <stdio.h>
#include "aco-table.h"

AcoTable* test_new()
{
    AcoTable* table = aco_table_new(
            10,     // host id
            0,      // min pheromone
            100,    // max pheromone
            30);    // max endurance

    aco_table_add_neigh(table, 0);
    aco_table_add_neigh(table, 1);
    aco_table_add_neigh(table, 2);

    aco_table_add_target(table, 3);
    aco_table_add_target(table, 4);
    aco_table_add_target(table, 5);

    return table;
}

void test_iter(AcoTable* table, int target)
{
    printf("I want to iterate, target is %d\n", target);

    for(AcoTableIter iter = aco_table_iter_begin(table, target);
        iter.valid;
        aco_table_iter_next(table, &iter))
    {
        printf("target %d, neigh %d\n", iter.value.target, iter.value.neigh);
    }

    printf("\n");

    return;
}

void test_new_targets(AcoTable* table)
{
	aco_ids_t targets = NULL;

	targets = aco_table_new_targets(table);

	printf("%d\n", (int)targets[0]);
	printf("%d\n", (int)targets[1]);
	printf("%d\n", (int)targets[2]);
	printf("%d\n", (int)targets[3]);
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


    test_new_targets(table);

    printf("Test Successful\n");

    return 0;
}
