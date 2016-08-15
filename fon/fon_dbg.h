#ifndef FON_DBG_H
#define FON_DBG_H

#include <stdlib.h>

#ifdef DEBUG
#define dbg(fmtstr, args...) \
  (printf("DBG: %s: " fmtstr "\n", __func__, ##args))
#else
#define dbg(dummy...)
#endif

#endif // FON_DBG_H
