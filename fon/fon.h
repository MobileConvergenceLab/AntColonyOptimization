#ifndef FCLIENT_H
#define FCLIENT_H

/* fclien.h
 * library for FON Client
 */

#include "array.h"
#include "fon_defs.h"
#include "fon_ipc.h"
#include "fon_client.h"


FON_BEGIN_EXTERN_C

FonClient*  fon_client_new              (fon_type_t     in_type,
                                         int            in_port);
bool        fon_sendto                  (FonClient      *client,
                                         packet_hdr_t   *hdr);
int         fon_recvfrom                (FonClient      *client,
                                         packet_hdr_t   *hdr,
                                         int            buflen);
bool        fon_table_add               (FonClient      *client,
                                         fib_tuple_t    *tuple);
bool        fon_table_del               (FonClient      *client,
                                         fon_id_t       id);
bool        fon_table_get               (FonClient      *client,
                                         Array          **tuple_array);
bool        fon_host_get                (FonClient      *client,
                                         fon_id_t       *out_id);
fon_type_t  fon_get_type                (FonClient      *client);


FON_END_EXTERN_C

#endif /* FCLIENT_H */
