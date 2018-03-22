#include "print.h"
#include "spinlock.h"

static LOCK gPrintSpinLock;
static LOCK gAquireLock;

extern DWORD gPort;

BOOLEAN
putc(
    _In_ CHAR c
)
{
    return (*printCharacter)(c);
}

BOOLEAN
Puts(
    _In_ PCHAR str
)
{
    return (*printString)(str);
}

VOID
Init(
    VOID
)
{
    if (gPort == 0)
    {
        printCharacter = &ConsolePutChar;
        printString = &CConsolePrint;
    }
    else
    {
        printCharacter = &SerialPut;
        printString = &SerialWrite;
    }
}

VOID
InitOnlyCons(
    VOID
)
{
    printCharacter = &ConsolePutChar;
    printString = &CConsolePrint;
}

VOID
InitOnlySer(
    VOID
)
{
    printCharacter = &SerialPut;
    printString = &SerialWrite;
}

VOID InitSerOrVid(
    VOID
)
{
    LockInit(&gAquireLock);
    LockInit(&gPrintSpinLock);

    Init();
    PciInit();
    Init();
}

VOID
PrintSpinlockAquaire(
    VOID
)
{
    Lock(&gPrintSpinLock);
}


VOID
PrintSpinlockRelease(
    VOID
)
{
    Unlock(&gPrintSpinLock);
}

VOID
AquireLock(
    VOID
)
{
    Lock(&gAquireLock);
}

VOID
ReleaseLock(
    VOID
)
{
    Unlock(&gAquireLock);
}
