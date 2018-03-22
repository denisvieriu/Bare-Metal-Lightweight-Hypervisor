#ifndef STRING_H
#define STRING_H

#include "minihv.h"

//
PCHAR
CustomStrcpy(
    PCHAR dest,
    CONST CHAR* src
);

//
PCHAR
CustomStrcat(
    PCHAR dest,
    CONST CHAR* src
);

PCHAR
CustomStrncat(
    PCHAR dest,
    CONST PCHAR src,
    size_t n
);

//
size_t
CustomStrlen(
    CONST char *str
);

//
char*
CustomStrcatC(
    char *dest,
    CONST char src
);




#endif // !STRING_H
