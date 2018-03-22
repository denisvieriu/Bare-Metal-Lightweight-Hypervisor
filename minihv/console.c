#include "console.h"
#include "ssnprintf.h"

BOOLEAN gNewLineEnabled = TRUE;

VOID SetNewLine(BOOLEAN var)
{
    InitOnlyCons();
    gNewLineEnabled = var;

    if (var == TRUE)
    {
        CustomPrintf("\n");
    }
}

VOID
ConsolePrint(
    PCHAR                           PFileName,
    DWORD                           DwLine,
    PCHAR                           PFormat,
    ...
    )
{

    InitOnlyCons();

    va_list arg;
    size_t length;
    CHAR buffer[MAX_PATH];
    va_start(arg, PFormat);
    length = Rpl_vsnprintf(buffer, MAX_PATH, PFormat, arg);
    va_end(arg);

    CustomPrintf("%-14s,  %4d:  ", PFileName, DwLine);

    Puts(buffer);

    if (gNewLineEnabled)
        CustomPrintf("\n");


}
