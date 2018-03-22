#ifndef CONSOLE_H
#define CONSOLE_H

#include "minihv.h"
#include "stdarg.h"
#include "crt.h"
#include "print.h"
#include "format.h"

VOID
ConsolePrint(
    /* _In_z_ */    PCHAR                           PFileName,
    /* _In_   */    DWORD                           DwLine,
    /* _In_   */    PCHAR                           PFormat,
    /* _In_   */    ...
);

/// Should be used to log informations/traces only
/// \warning It accepts ONLY STRING LITERALS as first parameter
#define ConsoleLog(Format, ...)               ConsolePrint(__FILE__, __LINE__, Format, __VA_ARGS__)

/// Should be used to log informations/traces only
/// \warning It accepts ONLY STRING LITERALS as first parameter
#define ConsoleLogInfo(Format, ...)           ConsolePrint(__FILE__, __LINE__, "[INFO] " Format, __VA_ARGS__)

/// Should be used to log warnings only
/// \warning It accepts ONLY STRING LITERALS as first parameter
#define ConsoleLogWarning(Format, ...)        ConsolePrint(__FILE__, __LINE__, "[WARNING] " Format, __VA_ARGS__)

/// Should be used to log errors only
/// \warning It accepts ONLY STRING LITERALS as first parameter
#define ConsoleLogError(Format, ...)          ConsolePrint(__FILE__, __LINE__, "[ERROR] " Format, __VA_ARGS__)

VOID SetNewLine(BOOLEAN var);

#endif // CONSOLE_H
