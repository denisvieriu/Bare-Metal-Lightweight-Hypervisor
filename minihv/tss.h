#pragma once

#include "minihv.h"
#include "ntstatus.h"

#define PREDEFINED_TSS_SIZE         104

#define NO_OF_IST                   7

#define NO_OF_PRIVILLEGE_LEVELS     3

#pragma pack(push,1)
//warning C4214: nonstandard extension used : bit field types other than int
#pragma warning(disable:4201)
typedef struct _TSS
{
    DWORD           Reserved0;
    QWORD           Rsp[NO_OF_PRIVILLEGE_LEVELS];       // RSP0,1,2 (for Ring3 we don't have a stack)
    QWORD           Reserved1;                          // Probably because of IST[0] which needs to be NULL
    QWORD           IST[NO_OF_IST];                     // IST[1..7]
    QWORD           Reserved2;
    WORD            Reserved3;
    WORD            IOMapBaseAddress;
} TSS, *PTSS;

#pragma warning (disable : 4214)
typedef union _TSS_ENTRY_HIGH
{
    struct
    {
        QWORD BaseAddress : 32;
        QWORD Reserved0 : 8;
        QWORD Zero : 5;
        QWORD Reserved1 : 19;
    };
    QWORD Raw;
} TSS_ENTRY_HIGH, *PTSS_ENTRY_HIGH;

typedef union _TSS_ENTRY_LOW
{
    struct
    {
        QWORD SegmentLimit0 : 16;
        QWORD BaseAddress0 : 16;
        QWORD BaseAddress1 : 8;
        QWORD Type : 4;
        QWORD Zero0 : 1;
        QWORD Dpl : 2;
        QWORD P : 1;
        QWORD SegmentLimit1 : 4;
        QWORD Avl : 1;
        QWORD Zero1 : 1;
        QWORD Zero2 : 1;
        QWORD G : 1;
        QWORD BaseAddress2 : 8;
    };
    QWORD Raw;
} TSS_ENTRY_LOW, *PTSS_ENTRY_LOW;
#pragma warning(default:4201)
#pragma pack(pop)

NTSTATUS
TssInstallStack(
    PTSS        Tss,
    BYTE        StackIndex
);

VOID
TssInstallStacks(
    PTSS Tss
);
