#ifndef CR_LOGGER_H
#define CR_LOGGER_H

// Custom print functions.
void cr_printf(int fd, const char* restrict fmt, ...);
void cr_snprintf(char* buffer, size_t count, const char* fmt, ...);

#endif