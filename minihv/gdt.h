#pragma once

#include "minihv.h"
#include "cpu.h"

#define TR_FIRST_SELECTOR 0x18

#define TRAMP_16_LOCATION 0x02000
#define TRAMP_16_SIZE 0x01000

#define GDT_NULL_ENTRY_SIZE 8
#define GDT_CODE_ENTRY_SIZE 8
#define GDT_DATA_ENTRY_SIZE 8
#define GDT_TSS_ENTRY_SIZE 0x10

#define GDT_CODE64_SEL  0x08

#define TSS_SIZE 0x68 // 104 bytes

#define GDT_CODE64_DESCRIPTOR 0x002F9A000000FFFF
#define GDT_DATA64_DESCRIPTOR 0x00CF92000000FFFF

#define GDT_REAL_MODE_SELECTOR_LIMIT 0xFFFFF

#define CS_TYPE_EXECUTE_READ_ACCESSED 0xB
#define DS_TYPE_READ_WRITE_ACCESSED 0x3

#define TR_TYPE_16_BIT_BUSY_TSS 0x3
#define TR_TYPE_32_BIT_BUSY_TSS 0xB

#pragma pack(push)
#pragma pack(1)

typedef struct _GDT
{
    QWORD Reserved;
    QWORD Code64Selector;
    QWORD Data64Selector;
    QWORD TssDescriptorLow;
    QWORD TssDescriptorHigh;
} GDT, *PGDT;

typedef struct _GDTR
{
    WORD GdtLength;
    PQWORD Gdt;
} GDTR, *PGDTR;
#pragma pack(pop)

QWORD
GdtComputeTssDescriptorLowValue(
    QWORD TssAddress
);

QWORD
GdtComputeTssDescriptorHighValue(
    QWORD TssAddress
);

VOID
GdtSetupFinalGdt(
    CPU Cpus[8]
);

PQWORD
GdtGetGdtrBaseAddress(
    VOID
);

WORD
GdtGetGdtrLimit(
    VOID
);


