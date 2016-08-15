#ifndef TEST_H
#define TEST_H

#define TEST(EXPR)    do{ \
                          if(!(EXPR)){ \
                              printf("Assert fail:%s\n", #EXPR); \
                              abort(); \
                          } \
                      }while(0);

#endif // TEST_H
