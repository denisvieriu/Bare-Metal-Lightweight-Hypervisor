#include "dbglog.h"
#include "console.h"
#include "ssnprintf.h"


VOID
DbgDebugLog(
    _In_ PCHAR PFileName,
    _In_ DWORD DwLine,
    _In_ volatile WORD LogType,
    _In_ PCHAR PFormat,
    ...)
{
    va_list arg;
    size_t  length;
    CHAR    buffer[MAX_PATH];

    va_start(arg, PFormat);
    length = Rpl_vsnprintf(buffer, MAX_PATH, PFormat, arg);
    va_end(arg);

    if (SERIAL_LOG == TRUE)
    {
        Init();

        SetDefaultCol(LogType);

        CustomPrintf("%-16s,  %4d:  ", PFileName, DwLine);

        Puts(buffer);

        if (LogType != VGA_NO_NEWLINE)
        {
            CustomPrintf("\n");
        }

        if (LogType == VGA_ERROR || LogType == VGA_ACPI)
        {
            SetNewLine(FALSE);
        }
    }

    if (VGA_LOG == TRUE)
    {
        ConsolePrint(PFileName, DwLine, "%s", buffer);
    }

    if (VGA_LOG && LogType == VGA_ERROR)
    {
        WriteBlinkingText('.', 6, DEFAULT_SLEEP_BLINK/3, FALSE, FALSE);
        SetNewLine(TRUE);
    }
    else if (VGA_LOG && LogType == VGA_ACPI)
    {
        WriteBlinkingText('.', 6, DEFAULT_SLEEP_BLINK/5, FALSE, FALSE);
        SetNewLine(TRUE);
    }
}


static DWORD
MemChar(
    DWORD Val
)
{
    return (Val >= 0x20 && Val < 0x80) ? Val : '.';
}

VOID
MemDump(
    CONST VOID *Start,
    CONST VOID* End
)
{
    LOG_INFO("Memdump from 0x%016x to 0x%016x\n", Start, End);

    BYTE *p = (BYTE *)Start;

    while (p < (BYTE *)End)
    {
        DWORD offset = (QWORD)p & 0xFFFF;
        LOG_INFO("%04x:  "
            "%02x %02x %02x %02x  "
            "%02x %02x %02x %02x  "
            "%02x %02x %02x %02x  "
            "%02x %02x %02x %02x  "
            "%c%c%c%c%c%c%c%c"
            "%c%c%c%c%c%c%c%c\n",
            offset,
            p[0x0], p[0x1], p[0x2], p[0x3],
            p[0x4], p[0x5], p[0x6], p[0x7],
            p[0x8], p[0x9], p[0xa], p[0xb],
            p[0xc], p[0xd], p[0xe], p[0xf],
            MemChar(p[0x0]), MemChar(p[0x1]),
            MemChar(p[0x2]), MemChar(p[0x3]),
            MemChar(p[0x4]), MemChar(p[0x5]),
            MemChar(p[0x6]), MemChar(p[0x7]),
            MemChar(p[0x8]), MemChar(p[0x9]),
            MemChar(p[0xa]), MemChar(p[0xb]),
            MemChar(p[0xc]), MemChar(p[0xd]),
            MemChar(p[0xe]), MemChar(p[0xf]));

        p += 16;
    }
}

