#ifndef FON_TEST_H
#define FON_TEST_H

#include "fon.h"

FON_BEGIN_EXTERN_C

void    fon_test_host_print         (FonClient *client);
void    fon_test_table_add          (FonClient *client, int id);
void    fon_test_tuple_print        (const fib_tuple_t *tuple);
void    fon_test_table_print        (FonClient *client);

FON_END_EXTERN_C

#endif
