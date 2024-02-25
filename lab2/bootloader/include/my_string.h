#ifndef _MY_STRING_H_
#define _MY_STRING_H_
#include "math_.h"
unsigned int vsprintf(char *, char *, __builtin_va_list );
char *ftoa(float , char *);
char *itoa(int , char *);
char *itox(int , char *);
int strcmp(const char *, const char *);
#endif // _MY_STRING_H_