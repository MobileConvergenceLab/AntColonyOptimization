/* library for FON Client */

#include <glib.h>

#include "msg_if.h"
#include "msg_defs.h"
#include "fon-utils.h"
#include "common-defs.h"

typedef gboolean (*FonCallbackRecv)     (const packet *pkt, gpointer user_data);
typedef gboolean (*FonCallbackHost)     (gpointer user_data);
typedef gboolean (*FonCallbackTable)    (gpointer user_data);

gboolean fon_init(GMainContext *context, int in_type ,int in_port);
gboolean fon_sendto(packet *pkt);
gboolean fon_table_add(table_tuple *tuple);
gboolean fon_table_del(int id);
gboolean fon_table_get(GArray **tuple_array);
gboolean fon_host_get(int *in_id);
int      fon_get_type();
void fon_set_callback_recv(FonCallbackRecv recv_callback, gpointer user_data);



// TODO
/* 데몬에서 호스트id가 변경되면 등록한 콜백함수가 호출됨 */
void fon_set_callback_host(FonCallbackRecv host_callback, gpointer user_data);
/* 데몬에서 테이블이 변경되면 등록한 콜백함수가 호출됨 */
void fon_set_callback_table(FonCallbackRecv table_callback, gpointer user_data);
