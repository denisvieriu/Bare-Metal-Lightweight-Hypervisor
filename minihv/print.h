#ifndef PRINT_H
#define PRINT_H

#include "pci.h"
#include "cconsole.h"
#include "video_memory_print.h"

static
BOOLEAN(*printCharacter)(CHAR);

static
BOOLEAN(*printString)(const char *fmt, ...);

BOOLEAN
putc(
    CHAR c
);

BOOLEAN
Puts(
    PCHAR str
);

VOID
Init(
    VOID
    );

VOID InitSerOrVid(
    VOID
    );

VOID
InitOnlyCons(
    VOID
    );


VOID
PrintSpinlockAquaire(
    VOID
    );

VOID
PrintSpinlockRelease(
    VOID
    );

VOID
AquireLock(
    VOID
);

VOID
ReleaseLock(
    VOID
);


#endif // PRINT_H