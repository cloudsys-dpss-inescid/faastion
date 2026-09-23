#include <stddef.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include "cr_logger.h"

// CR printf (stack-allocated) buffer.
#define PRINTF_BUF_SZ 1024

// Forward declaration of this function (see deps/printf/printf.c).
int vsnprintf_(char* buffer, size_t count, const char* format, va_list va);

size_t safe_write(int fd, char* buffer, size_t count) {
    size_t written = 0;
    size_t n;
    while (written < count) {
        if ((n = write(fd, buffer + written, count - written)) < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            } else {
                write(STDERR_FILENO, "error: could not write data\n", 28);
            }
        }
        written += n;
    }
    return written;
}

// Malloc-free printf.
void cr_printf(int fd, const char* restrict fmt, ...) {
    char buffer[PRINTF_BUF_SZ];

    va_list ap;
    va_start(ap, fmt);
    int chars = vsnprintf_(buffer, PRINTF_BUF_SZ, fmt, ap);
    va_end(ap);

    if (chars >= PRINTF_BUF_SZ) {
       (void) safe_write(STDERR_FILENO, "error: printf buffer not large enough\n", 38);
    }

    safe_write(fd, buffer, chars);
}

// Malloc-free snprintf.
void cr_snprintf(char* buffer, size_t count, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf_(buffer, count, fmt, ap);
    va_end(ap);
}

// printf.h requires this method to be implemented, even if we don't need it.
void _putchar(char character) {
    cr_printf(STDERR_FILENO, "error: _putchar shouldn't be called directly\n");
}