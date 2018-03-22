#ifndef _LOCAL_LAPIC_H
#define _LOCAL_LAPIC_H

#include "minihv.h"
#include "cpu.h"
#include "map.h"

#define DEF_CPU_INFO        4
#define SHIFT_RIGHT_GET_ID  24
#define IA32_APIC_BASE      0x1B

typedef enum _REGISTERS
{
    EAX,
    EBX,
    ECX,
    EDX
}REGISTERS;



NTSTATUS
CheckGenuineIntel(
    VOID
);

BYTE
GetInitialApicId(
    VOID
);

BOOLEAN
IsBsp(
    _In_ BYTE apic
);

NTSTATUS
LapicInit(
    VOID
);

VOID
LapicSetupTrampCode(
    CPU Cpus[8]
);



VOID
SignalAP(
    _In_ PCPU cpu
    );

VOID
SendInit(
    VOID
);

VOID
SendStartUpInit(
    VOID
);


VOID
LapicSignalAllAPs(
    PCPU    Cpu,
    BYTE    SipiVector
);
#endif
