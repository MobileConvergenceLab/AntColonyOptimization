#ifndef FON_DEFS_H
#define FON_DEFS_H

#ifndef MAX
  #define MIN(X,Y) ((X) < (Y) ? (X) : (Y))  
  #define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#endif

/* 
 * http://stackoverflow.com/questions/3982348/implement-generic-swap-macro-in-c
 */
#ifndef SWAP
  #define SWAP(x, y, T) do { T SWAP = x; x = y; y = SWAP; } while (0)
#endif

#ifdef __cplusplus
  #define FON_BEGIN_EXTERN_C    extern "C" {
  #define FON_END_EXTERN_C      }
#else
  #define FON_BEGIN_EXTERN_C
  #define FON_END_EXTERN_C
#endif

/* if compiler version is c99 */
#if __STDC_VERSION__ == 199901L
extern char *strdup(const char *s);
#endif

#endif // FON_DEFS_H
