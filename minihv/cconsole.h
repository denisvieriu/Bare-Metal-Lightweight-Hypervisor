#ifndef _CCONSOLE_H
#define _CCONSOLE_H

#include "minihv.h"

typedef enum _WRITES
{
    SET_LOAD_BAR,
    SET_MINIHV_BAR
}WRITES;

typedef enum _SEVERITY_ENUM
{
    SEV_DEFAULT,
    SEV_HIGHLIGHT

}SEVERITY_ENUM;

typedef struct _SEVERITY
{
    SEVERITY_ENUM        Var;
    CONST unsigned char Keyword[6];

}SEVERITY, *PSEVERITY;

VOID
ConsoleInit(
    VOID
);

BOOLEAN
ConsolePutChar(
    _In_ CHAR c
    );

BOOLEAN
CConsolePrint(
    _In_ CONST CHAR *fmt,
    _In_opt_   ...
);

VOID
WriteCurrentFileDbg(
    _In_ CONST CHAR *fmt,
    _In_opt_   ...
);

VOID
WriteBlinkingText(
    _In_        CHAR toPrint,
    _In_        DWORD nrTimes,
    _In_opt_    DWORD sleepTime,
    _In_opt_    BOOLEAN blinkActive,
    _In_opt_    BOOLEAN difCol
);

BOOLEAN
SetDefaultCol(
    _In_        volatile WORD col
);

BOOLEAN
IncrementLoaded(
    _In_ DWORD Value,
    _In_ WRITES Opt
);

DWORD
RandRdtsc(
    _In_ DWORD minV,
    _In_ DWORD maxV
    );


#endif // !_CCONSOLE_H
