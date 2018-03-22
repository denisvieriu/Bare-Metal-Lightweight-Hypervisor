#ifndef _ACPICA_H
#define _ACPICA_H

#include "minihv.h"
#include "acpi.h"
#include "cpu.h"

ACPI_STATUS
AcpiInit(
    VOID
);

ACPI_STATUS
AcpiGetProcessors(
   _Out_ PCPU cpu
);

DWORD GetNrOfCpus(
    VOID
);


ACPI_STATUS
AcpiGetPciEcam(
    PQWORD BaseAddress,
    PBYTE StartBusNumber,
    PBYTE EndBusNumber
);

#endif // _ACPICA_H

