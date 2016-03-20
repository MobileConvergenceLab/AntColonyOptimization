#ifndef ALGORITHM_H
#define ALGORITHM_H

#ifndef MAX
  #define MIN(X,Y) ((X) < (Y) ? (X) : (Y))  
  #define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#endif

/* 
 * http://stackoverflow.com/questions/3982348/implement-generic-swap-macro-in-c
 */
#define SWAP(x, y, T) do { T SWAP = x; x = y; y = SWAP; } while (0)

#endif
