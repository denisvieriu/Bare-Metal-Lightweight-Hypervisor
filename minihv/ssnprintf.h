#ifndef _SNPRINTF_H
#define _SNPRINTF_H

#include "minihv.h"
#include "acpica.h"

int Rpl_vsnprintf(char *str, size_t size, const char *format, va_list args);

#endif
