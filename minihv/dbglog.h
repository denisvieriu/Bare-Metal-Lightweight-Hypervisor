#ifndef DBGLOG_H
#define DBGLOG_H

#include "minihv.h"
#include "stdarg.h"
#include "crt.h"
#include "print.h"
#include "error.h"

#define EMPTY   "          "
#define INFO    "[INFO]    "
#define WARNING "[WARNING] "
#define ERROR   "[ERROR]   "
#define ACPI    "[ACPI]    "
#define MSR     "[MSR]     "
#define PCI     "[PCI]     "


#define HALT_THE_PC    "[HALTING]"

VOID
DbgDebugLog(
    _In_    PCHAR                           PFileName,
    _In_    DWORD                           DwLine,
    _In_    volatile WORD                   LogType,
    _In_    PCHAR                           PFormat,
    _In_    ...
);

VOID
MemDump(
    CONST VOID *Start,
    CONST VOID* End
);

#define StandardLogLock(LogType, Type, Format, ...)                             \
{                                                                               \
    do                                                                          \
    {                                                                           \
        PrintSpinlockAquaire();                                                 \
        DbgDebugLog(__FILE__, __LINE__, LogType, Type Format, __VA_ARGS__);     \
        PrintSpinlockRelease();                                                 \
    } while (0);                                                                \
}

#define LOG(Format, ...)                StandardLogLock(VGA_DEFAULT,    EMPTY,    Format, __VA_ARGS__);

#define LOG_INFO(Format, ...)           StandardLogLock(VGA_DEFAULT,    INFO,     Format, __VA_ARGS__);

#define LOG_WARNING(Format, ...)        StandardLogLock(VGA_DEFAULT,    WARNING,  Format, __VA_ARGS__);

#define LOG_ERROR(Format, ...)          StandardLogLock(VGA_ERROR,      ERROR,    Format, __VA_ARGS__);

#define LOG_ACPI(Format, ...)           StandardLogLock(VGA_DEFAULT,    ACPI,     Format, __VA_ARGS__);

#define LOG_ACPI_ERROR(Format, ...)     StandardLogLock(VGA_ERROR,      ACPI,     Format, __VA_ARGS__);

#define LOG_MSR(Format, ...)            StandardLogLock(VGA_DEFAULT,    MSR,      Format, __VA_ARGS__);

#define LOG_PCI(Format, ...)            StandardLogLock(VGA_PCI,        PCI,      Format, __VA_ARGS__);

#define DEF_PRINTF(Format, ...)         StandardLogLock(VGA_NO_NEWLINE, EMPTY,    Format, __VA_ARGS__);

#define LOG_WARN_HLT(Format, ...)       {StandardLogLock(VGA_WARNING,    WARNING,  Format, __VA_ARGS__); __halt();}


#define LOG_IN_BASE2(Number, NumberSize)                                                                    \
{                                                                                                           \
    do{                                                                                                     \
        BYTE  crtBit;                                                                                       \
        INT   crtStrIdx;                                                                                    \
        QWORD bitToGet;                                                                                     \
        CHAR  nrInBase2[SIZEOF_BYTE_TO_BITS(sizeof(QWORD))];                                                \
        crtStrIdx = 0;                                                                                      \
        for (crtBit = 0; crtBit < SIZEOF_BYTE_TO_BITS(NumberSize); crtBit++)                                \
        {                                                                                                   \
            bitToGet = GET_MASK_OF_1_BIT_SET(crtBit);                                                       \
            (bitToGet & Number) ? (nrInBase2[crtStrIdx++] = '1') : (nrInBase2[crtStrIdx++] = '0');          \
        }                                                                                                   \
        /* To do : Reverse the string*/                                                                     \
        /* HvReverse(nrInBase2, crtStrIdx]; */                                                              \
        nrInBase2[crtStrIdx] = '\0';                                                                        \
        LOG_INFO("%s", nrInBase2);                                                                          \
    } while (0);                                                                                            \
}

#endif // DBGLOG_H
