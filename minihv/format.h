#ifndef _FORMAT_H
#define _FORMAT_H

#include "acpica.h"
#include "minihv.h"

int vsscanf(const char *str, const char *fmt, va_list args);
int sscanf(const char *str, const char *fmt, ...);

int vsnprintf(char *str, size_t size, const char *fmt, va_list args);
int snprintf(char *str, size_t size, const char *fmt, ...);

UINT myStrtoul(const char *nptr, char **endptr, int base);

#endif
