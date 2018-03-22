#ifndef PCI_H
#define PCI_H

#include "minihv.h"

VOID PciInit(
    VOID
);

BOOLEAN
SerialPut(
    CHAR chr
);

BOOLEAN
SerialWrite(
    CONST CHAR *fmt, ...
);
#endif // PCI_H
