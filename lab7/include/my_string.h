#ifndef _MY_STRING_H_
#define _MY_STRING_H_
#define NULL 0
#include "math_.h"
#include "buddy_system.h"
unsigned int vsprintf(char *, char *, __builtin_va_list);
char *ftoa(float, char *);
char *itoa(int, char *);
char *itox(int, char *);
int strcmp(const char *, const char *);
int strlen(const char *);
char *strcpy(char *to, const char *from);
int strncmp(const char *a, const char *b, int n);
char *strncpy(char *to, const char *from, unsigned long size);
// align 'n' up to the value 'align', which must be a power of two
unsigned long align_up(unsigned long n, unsigned long align);
char *memcpy(void *dest, const void *src, unsigned long long len);
long hextol(char *);
char* strtok(char* str, const char* delimiters);
char* strchr(const char* str, int c);
char* strdup(const char* src);
char* dirname(char* path);
char* basename(char* path);
char* strrchr(const char* str, int c);
#endif  // _MY_STRING_H_