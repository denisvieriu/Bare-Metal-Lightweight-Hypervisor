#ifndef CRT_H
#define CRT_H

#include "minihv.h"
#include "stdarg.h"
#include "string.h"
#include "acpica.h"

#ifdef CRT_H
#define RtlZeroMemory(Destination,Length) CustomMemset((Destination),0,(Length))
#define ZeroMemory RtlZeroMemory
#endif

//
PCHAR
CustomItoa(
    _In_    QWORD value,
    _Out_   PCHAR str,
    _In_    INT32 base
    );

//
size_t CustomSnprintFunc(
    _Out_ PCHAR buffer,
    _In_  INT n,
    _In_  CHAR CONST * fmt,
    _In_  va_list arg
    );

//
size_t
CustomSnprintf(
    _Out_ PCHAR buffer,
    _In_  INT n,
    _In_  CONST CHAR* fmt,
    _In_  ...
    );

//
size_t
CustomPrintf(
    _In_  CONST PCHAR fmt,
    _In_  ...
    );

//
VOID
CustomMemcpy(
    _In_  PVOID dest,
    _In_  PVOID src,
    _In_  size_t n
    );

VOID*
CustomMemset(
    _In_  VOID *b,
    _In_  INT C,
    _In_  INT len);

#endif // !CRT_H
