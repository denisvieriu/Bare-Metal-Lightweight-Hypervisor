#pragma once

#include "minihv.h"

#pragma pack(push,1)
//warning C4214: nonstandard extension used : bit field types other than int
#pragma warning(disable:4201)
#pragma warning(disable:4214)
typedef union _IDT_ENTRY_LOW
{
    struct
    {
        QWORD   OffsetLow : 16;
        QWORD   SegmentSelector : 16;
        QWORD   Ist : 3;
        QWORD   Zero1 : 5;
        QWORD   Type : 4;
        QWORD   Zero2 : 1;
        QWORD   Dpl : 2;
        QWORD   P : 1;
        QWORD   OffsetMiddle : 16;
    };
    QWORD Raw;
} IDT_ENTRY_LOW, *PIDT_ENTRY_LOW;

typedef union _IDT_ENTRY_HIGH
{
    struct
    {
        QWORD   OffsetHigh : 32;
        QWORD   Reserved : 32;
    };
    QWORD Raw;
} IDT_ENTRY_HIGH, *PIDT_ENTRY_HIGH;

typedef struct _IDT_ENTRY
{
    IDT_ENTRY_LOW IdtEntryLow;
    IDT_ENTRY_HIGH IdtEntryHigh;
} IDT_ENTRY, *PIDT_ENTRY;

typedef struct _IDT_PTR
{
    WORD limit;
    QWORD base;
} IDT_PTR, *PIDT_PTR;

typedef struct _IDT_ENTRY_REAL_MODE
{
    DWORD Offset : 16;
    DWORD SegmentSelector : 16;
} IDT_ENTRY_REAL_MODE, *PIDT_ENTRY_REAL_MODE;
#pragma warning(default:4201)
#pragma pack(pop)

typedef enum _IST
{
    CURRENT_STACK_IST_INDEX,
    DOUBLE_FAULT_IST_INDEX,
    NMI_IST_INDEX
} IST;

void
IdtSetGate(
    BYTE    IdtIndex,
    QWORD   Offset,
    WORD    SegmentSelector,
    BYTE    Ist,
    BYTE    Type
);

void
IdtInit(
    void
);

void
IdtLoad(
    void
);

QWORD
IdtGetIdtrBaseAddress(
    VOID
);