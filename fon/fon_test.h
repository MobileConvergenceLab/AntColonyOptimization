#ifndef FON_TEST_H
#define FON_TEST_H

#include "fon.h"

#ifdef __cpluscplus
extern "C" {
#endif

void    fon_test_host_print         (FonClient *client);
void    fon_test_table_add          (FonClient *client, fon_id_t id);
void    fon_test_tuple_print        (const fib_tuple_t *tuple);
void    fon_test_table_print        (FonClient *client);

#ifdef __cpluscplus
};
#endif

#endif
