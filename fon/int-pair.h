/**
 * Written by Sim Young-Bo <twinwings1111@gmail.com>
 */
#ifndef INT_PAIR_H
#define INT_PAIR_H

#include <glib.h>

typedef struct _IntPair IntPair;

IntPair* int_pair_new();
IntPair* int_pair_ref(IntPair* ipair);
void int_pair_unref(IntPair* ipair);
int int_pair_npairs(IntPair* ipair);
gboolean int_pair_add(IntPair* ipair, int first, int second);
void int_pair_remove(IntPair* ipair, int first);
gboolean int_pair_get_second(IntPair* ipair, int first, int* out_second);
gboolean int_pair_get_first(IntPair* ipair, int second, int* out_first);

#endif /* INT_PAIR_H */
