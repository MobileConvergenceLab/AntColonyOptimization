/**
 * Implementation of (FON Function-)client manager.
 * 데몬-클라이언트간 동기/비동기 메세지를 위한 채널 정보를 담고 있다.
 */
#ifndef CMANAGER_H
#define CMANAGER_H

#include <glib.h>

typedef struct _ClientInfo {
    int                 sync;           /* sync message channel socket FD */
    struct sockaddr_in  deliver_addr;   /* async, deliver, UDP loopback address */
} ClientInfo;

static inline gint __client_key_compare_func (gconstpointer a, gconstpointer b, gpointer user_data)
{
    return GPOINTER_TO_INT(a) - GPOINTER_TO_INT(b);
}

#define cmanager_create()                   \
        g_tree_new_full(__client_key_compare_func, NULL, NULL, g_free)

/**
 *  tree: type  GTree*.
 *  key : type  gint.
 *  info: type  ClientInfo*, IT MUST BE allocated by g_malloc().
 */
#define cmanager_insert(tree, key, info)    \
        g_tree_insert((tree), GINT_TO_POINTER((key)), (info));

#define cmanager_remove(tree, key)          \
        g_tree_remove((tree), GINT_TO_POINTER((key)));

#define cmanager_lookup(tree, key)          \
        ((ClientInfo*)g_tree_lookup((tree), GINT_TO_POINTER((key))));

#endif /* CMANAGER_H */
