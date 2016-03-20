/**
 * Written by Sim Young-Bo <twinwings1111@gmail.com>
 */
#include "int-pair.h"

struct _IntPair {
    int     reference_count;
    GTree*  first_to_second;
    GTree*  second_to_first;
};

static gint
int_pair_cmp    (gconstpointer   a,
                 gconstpointer   b,
                 gpointer        user_data /* not used */)
{
    return GPOINTER_TO_INT(a) - GPOINTER_TO_INT(b);
}

IntPair* int_pair_new()
{
    IntPair* ipair = g_new0(IntPair, 1);

    ipair->reference_count  = 1;
    ipair->first_to_second  = g_tree_new_full(int_pair_cmp, NULL, NULL, g_free);
    ipair->second_to_first  = g_tree_new_full(int_pair_cmp, NULL, NULL, g_free);

    g_assert(ipair->first_to_second != NULL);
    g_assert(ipair->second_to_first != NULL);

    return ipair;
}

IntPair* int_pair_ref(IntPair* ipair)
{
    ipair->reference_count++;

    return ipair;
}

void int_pair_unref(IntPair* ipair)
{
    g_assert(ipair->reference_count != 0);
    ipair->reference_count--;
    if(ipair->reference_count == 0) {
        g_tree_unref(ipair->second_to_first);
        g_tree_unref(ipair->first_to_second);
        g_free(ipair);
    }
}

int int_pair_npairs(IntPair* ipair)
{
    g_assert(g_tree_nnodes(ipair->first_to_second) ==
            g_tree_nnodes(ipair->second_to_first));

    return g_tree_nnodes(ipair->first_to_second);
}

gboolean int_pair_add(IntPair* ipair, int first, int second)
{
    if(g_tree_lookup(ipair->first_to_second, GINT_TO_POINTER(first)) != NULL ||
       g_tree_lookup(ipair->second_to_first, GINT_TO_POINTER(second)) != NULL) {
        return FALSE;
    }

    g_assert(g_tree_lookup(ipair->first_to_second, GINT_TO_POINTER(first)) == NULL);
    g_assert(g_tree_lookup(ipair->second_to_first, GINT_TO_POINTER(second)) == NULL);

    int* p_first    = g_new(int,1);
    int* p_second   = g_new(int,1);

    *p_first = first;
    *p_second = second;

    g_tree_insert(ipair->first_to_second, GINT_TO_POINTER(first), p_second);
    g_tree_insert(ipair->second_to_first, GINT_TO_POINTER(second), p_first);

    return TRUE;
}

void int_pair_remove   (IntPair*     ipair,
                     int          first)
{
    int* p_second = g_tree_lookup(ipair->first_to_second, &first);
    if(p_second != NULL) {
        g_tree_remove(ipair->second_to_first, p_second);
        g_tree_remove(ipair->first_to_second, &first);
    }
}

gboolean
int_pair_get_second (IntPair*    ipair,
                     int         first,
                     int*        out_second)
{
    int* p_second = g_tree_lookup(ipair->first_to_second, GINT_TO_POINTER(first));
    if(p_second != NULL) {
        *out_second = *p_second;
        return TRUE;
    }
    else {
        return FALSE;
    }
}

gboolean
int_pair_get_first  (IntPair*    ipair,
                     int         second,
                     int*        out_first)
{
    int* p_first = g_tree_lookup(ipair->second_to_first, GINT_TO_POINTER(second));

    if(p_first != NULL) {
        *out_first = *p_first;
        return TRUE;
    }
    else {
        return FALSE;
    }
}

