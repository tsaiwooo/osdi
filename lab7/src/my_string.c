#include "my_string.h"

unsigned int vsprintf(char* dst, char* fmt, __builtin_va_list args)
{
    char* dst_orig = dst;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            // escape %
            if (*fmt == '%') {
                goto put;
            }
            // string
            if (*fmt == 's') {
                char* p = __builtin_va_arg(args, char*);
                while (*p) {
                    *dst++ = *p++;
                }
            }
            // number
            if (*fmt == 'd') {
                int arg = __builtin_va_arg(args, int);
                char buf[11];
                char* p = itoa(arg, buf);
                while (*p) {
                    *dst++ = *p++;
                }
            }
            // long
            if (*fmt == 'l') {
                fmt++;
                if (*fmt == 'd') {
                    long long arg = __builtin_va_arg(args, long long);
                    char buf[17];
                    char* p = itoa(arg, buf);
                    while (*p) {
                        *dst++ = *p++;
                    }
                }
            }
            // hex
            if (*fmt == 'x') {
                int arg = __builtin_va_arg(args, int);
                char buf[8 + 1];
                char* p = itox(arg, buf);

                while (*p) {
                    *dst++ = *p++;
                }
            }
            // float
            if (*fmt == 'f') {
                float arg = (float)__builtin_va_arg(args, double);
                char buf[19]; // sign + 10 int + dot + 6 float
                char* p = ftoa(arg, buf);
                while (*p) {
                    *dst++ = *p++;
                }
            }
        } else {
        put:
            *dst++ = *fmt;
        }
        fmt++;
    }
    *dst = '\0';

    return dst - dst_orig; // return written bytes
}

char* itoa(int value, char* s)
{
    int idx = 0;
    if (value < 0) {
        value *= -1;
        s[idx++] = '-';
    }

    char tmp[10];
    int tidx = 0;
    do {
        tmp[tidx++] = '0' + value % 10;
        value /= 10;
    } while (value != 0 && tidx < 11);

    // reverse tmp
    int i;
    for (i = tidx - 1; i >= 0; i--) {
        s[idx++] = tmp[i];
    }
    s[idx] = '\0';

    return s;
}

char* ftoa(float value, char* s)
{
    int idx = 0;
    if (value < 0) {
        value = -value;
        s[idx++] = '-';
    }

    int ipart = (int)value;
    float fpart = value - (float)ipart;

    // convert ipart
    char istr[11]; // 10 digit
    itoa(ipart, istr);

    // convert fpart
    char fstr[7]; // 6 digit
    fpart *= (int)pow(10, 6);
    itoa((int)fpart, fstr);

    // copy int part
    char* ptr = istr;
    while (*ptr)
        s[idx++] = *ptr++;
    s[idx++] = '.';
    // copy float part
    ptr = fstr;
    while (*ptr)
        s[idx++] = *ptr++;
    s[idx] = '\0';

    return s;
}

int strcmp(const char* X, const char* Y)
{
    while (*X) {
        if (*X != *Y)
            break;
        X++;
        Y++;
    }
    return *(const unsigned char*)X - *(const unsigned char*)Y;
}

char* itox(int value, char* s)
{
    int idx = 0;

    char tmp[8 + 1];
    int tidx = 0;
    if (value == 0) {
        s[0] = '0';
        s[1] = '\0';
        return s;
    }
    while (value) {
        int r = value % 16;
        if (r < 10) {
            tmp[tidx++] = '0' + r;
        } else {
            tmp[tidx++] = 'a' + r - 10;
        }
        value /= 16;
    }

    // reverse tmp
    int i;
    for (i = tidx - 1; i >= 0; i--) {
        s[idx++] = tmp[i];
    }
    s[idx] = '\0';

    return s;
}

int strlen(const char* str)
{
    const char* s;
    for (s = str; *s; ++s) {
    }
    return (s - str);
}

char* strcpy(char* to, const char* from)
{
    char* save = to;
    int len = strlen(from);
    // while (*from != 0) {
    // *to = *from;
    // to++;
    // from++;
    // }
    while (len--) {
        *to = *from;
        to++;
        from++;
    }
    *to = '\0';
    return save;
}

char* strncpy(char* to, const char* from, unsigned long size)
{
    char* save = to;
    while (size--) {
        *to = *from;
        to++;
        from++;
    }
    return save;
}

int strncmp(const char* a, const char* b, int n)
{
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return a[i] - b[i];
        }
        if (a[i] == 0) {
            return 0;
        }
    }
    return 0;
}

// 先把原本的值位移align的值，之後透過~(align - 1)
// mask，把位移的值設成0，其他都跟1 做and，所以還是原本的值
unsigned long align_up(unsigned long n, unsigned long align)
{
    return (n + align - 1) & (~(align - 1));
}

char* memcpy(void* dest, const void* src, unsigned long long len)
{
    char* d = dest;
    const char* s = src;
    while (len--)
        *d++ = *s++;
    return dest;
}

long hextol(char* s)
{
    unsigned long i, r = 0;

    // for (i = 0; s[i] != '\0'; i++) {
    for (i = 0; i < strlen(s); i++) {
        r *= 16;
        if (s[i] >= '0' && s[i] <= '9') {
            r += s[i] - '0';
        } else if (s[i] >= 'a' && s[i] <= 'f') {
            r += s[i] - 'a' + 10;
        } else if (s[i] >= 'A' && s[i] <= 'F') {
            r += s[i] - 'A' + 10;
        } else {
            return r;
        }
    }
    return r;
}

char* strtok(s, delim)
register char* s;
register const char* delim;
{
    register char* spanp;
    register int c, sc;
    char* tok;
    static char* last;

    if (s == NULL && (s = last) == NULL)
        return (NULL);

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
cont:
    c = *s++;
    for (spanp = (char*)delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }

    if (c == 0) { /* no non-delimiter characters */
        last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;) {
        c = *s++;
        spanp = (char*)delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}
// char* strtok(char* str, const char* delimiters)
// {
//     static char* last = NULL;
//     if (str == NULL) {
//         str = last;
//     }
//     if (str == NULL) {
//         return NULL;
//     }
//     while (*str && strchr(delimiters, *str)) {
//         str++;
//     }
//     if (*str == '\0') {
//         last = NULL;
//         return NULL;
//     }
//     char* token_start = str;
//     while (*str && !strchr(delimiters, *str)) {
//         str++;
//     }
//     if (*str) {
//         *str = '\0';
//         last = str + 1;
//     } else {
//         last = NULL;
//     }
//     return token_start;
// }

char* strchr(const char* str, int c)
{
    while (*str) {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}

char* strdup(const char* src)
{
    int len = strlen(src) + 1;
    char* dest = (char*)kmalloc(len);
    // for (int i = 0; i < 128; ++i) {
    //     dest[i] = '\0';
    // }
    if (dest) {
        strcpy(dest, src);
    }
    // dest[len] = '\0';
    return dest;
}

char* dirname(char* path)
{
    char* last_slash = strrchr(path, '/');
    if (last_slash) {
        if (last_slash == path) {
            path[1] = '\0';
        } else {
            *last_slash = '\0';
        }
    } else {
        path[0] = '.';
        path[1] = '\0';
    }
    return path;
}

char* basename(char* path)
{
    char* last_slash = strrchr(path, '/');
    return last_slash ? last_slash + 1 : path;
}

char* strrchr(const char* str, int c)
{
    char* last = NULL;
    char ch = (char)c;
    while (*str) {
        if (*str == ch) {
            last = (char*)str;
        }
        str++;
    }
    return last;
}

char* strcat(char* dest, const char* src)
{
    strcpy(dest + strlen(dest), src);
    return dest;
}