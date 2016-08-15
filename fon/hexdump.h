#ifndef HEXDUMP_H
#define HEXDUMP_H

#include "fon_defs.h"

FON_BEGIN_EXTERN_C

void _hexdump(char *desc, char *file, int line, const void *addr, int len);
#define hexDump(desc, addr, len)    _hexdump(desc, __FILE__, __LINE__, addr, len)

FON_END_EXTERN_C

#endif // HEXDUMP_H
