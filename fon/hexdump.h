#ifndef HEXDUMP_H
#define HEXDUMP_H

#ifdef __cpluscplus
extern "C" {
#endif

void _hexdump(char *desc, char *file, int line, const void *addr, int len);
#define hexDump(desc, addr, len)    _hexdump(desc, __FILE__, __LINE__, addr, len)

#ifdef __cpluscplus
};
#endif

#endif // HEXDUMP_H
