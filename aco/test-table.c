#include <stdio.h>
#include "aco-table.h"

AcoTable* test_new()
{
    AcoTable* table = aco_table_new(
            10,     // host id
            0,      // min pheromone
            100);   // max pheromone

    aco_table_add_col(table, 0);
    aco_table_add_col(table, 1);
    aco_table_add_col(table, 2);

    aco_table_add_row(table, 3);
    aco_table_add_row(table, 4);
    aco_table_add_row(table, 5);

    return table;
}

int main(int argc, char **argv) {

    AcoTable* table = test_new();

    aco_table_print_all(table);

    printf("Test Successful\n");

    return 0;
}
