#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>

#include <sys/syscall.h>

#define MAX_DIGITS 20

static int long_to_string(char *dst, unsigned long num, size_t length) {
    static const char base = 10;
    char buf[MAX_DIGITS]; // 64 bits --> max 20 digits
    unsigned long aux;
    int digit;
    
    int i = MAX_DIGITS-1;
    aux = num;
    if (num == 0) {
        buf[i--] = '0';
    }
    while (aux > 0) {
        digit = aux % base;
        buf[i--] = '0' + digit;
        aux /= base;
    }
    
    int nwrite = 0;
    for (++i; i < MAX_DIGITS && nwrite < length; i++, nwrite++) {
        dst[nwrite] = buf[i];
    }
    
    return nwrite;
}

static int copy(char *s1, char *s2, size_t length) {
    int nwrite = 0;
    for (; *s2 && nwrite < length; s1++, s2++, nwrite++) {
        *s1 = *s2;
    }
    return nwrite;
}

// consider adding support for negative values or other types of data (char, float, ...) 
// consider adding support for bigger strings
void print(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    int c;
    int n = 1024;
    char buf[1024] = {0,};
    for (c = 0; *fmt && c < n-1; fmt++) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'l')
                fmt++;
            switch (*fmt) {
            case 'd':
                c += long_to_string(buf+c, (long)va_arg(args, int), n-1-c);
                break;
            case 's':
                c += copy(buf+c, va_arg(args, char *), n-1-c);
                break;
            default:
                break;
            }
        } else {
            buf[c++] = *fmt;
        }
    }

    va_end(args);
    syscall(__NR_write, 2, buf, c);
}