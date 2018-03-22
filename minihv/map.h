#ifndef _MEMORY_MAP_H_
#define _MEMORY_MAP_H_

#include "minihv.h"

#define TB 0x0000010000000000
#define MB 0x100000

#define KERNEL_ADDRESS  (32 * MB)

#define VIRTUAL_TO_PHYSICAL_OFFSET (1 * TB - KERNEL_ADDRESS)

#define PA2VA(addr) ((QWORD)(addr) + VIRTUAL_TO_PHYSICAL_OFFSET)
#define VA2PA(addr) ((QWORD)(addr) - VIRTUAL_TO_PHYSICAL_OFFSET)

#define KERNEL_VA_START PA2VA(KERNEL_ADDRESS)

#define PAGE_SIZE 0x1000 // 4KB page
#define TABLE_ENTRY_SIZE 0x8
#define TABLE_NO_OF_ENTRIES 0x200
#define TABLE_SIZE (TABLE_ENTRY_SIZE * TABLE_NO_OF_ENTRIES)

#define FOUR_LEVEL_PG_OFFSET_MASK       0x1FF // 9 bits are used to index different entries from the paging tables
#define FOUR_LEVEL_PG_PAGE_OFFSET_MASK  0xFFF // 12 bits used to offset a byte in a 4KB page

#define PML4_OFFSET(x)  ((x >> 39) & FOUR_LEVEL_PG_OFFSET_MASK)
#define PDPT_OFFSET(x)  ((x >> 30) & FOUR_LEVEL_PG_OFFSET_MASK)
#define PD_OFFSET(x)    ((x >> 21) & FOUR_LEVEL_PG_OFFSET_MASK)
#define PT_OFFSET(x)    ((x >> 12) & FOUR_LEVEL_PG_OFFSET_MASK)
#define PG_OFFSET(x)    (x & FOUR_LEVEL_PG_PAGE_OFFSET_MASK)

#define PHYSICAL_ADDRESS_SHIFT 12

#define VIDEO_MEMORY_ADDRESS 0x00000000000B8000ULL

#define PML4_ADDRESS    (KERNEL_ADDRESS + (10 * MB)) // 42MB (1TB + 10MB VA) is where we keep our paging tables

#define KERNEL_RESERVED_MEMORY  (200 * MB)

#define HEAP_ADDRESS (KERNEL_ADDRESS + 100 * MB) // 132MB is where we keep our dynamically allocated memory

#define STACK_NUMBER_OF_PAGES 4
#define CPU_STACK_SIZE (STACK_NUMBER_OF_PAGES * PAGE_SIZE)

#define AddressOffset(addr,alig)            ((QWORD)(addr)&((alig)-1))
#define AlignAddressUpper(addr,alig)        (((QWORD)(addr)+(alig)-1)&(~((QWORD)(alig)-1)))
#define AlignAddressLower(addr,alig)        ((QWORD)(addr)&~((alig)-1))

#define GUEST_MBR_LOADER_LOCATION_PA 0x1000
#define GUEST_RESERVED_MEMORY 0x1000
#define GUEST_MBR_LOADER_SIZE 0x200
#define GUEST_STACK_PA GUEST_MBR_LOADER_LOCATION_PA + GUEST_MBR_LOADER_SIZE

#define PATCHED_INT15H_OFFSET GUEST_MBR_LOADER_LOCATION_PA + GUEST_MBR_LOADER_SIZE

typedef struct _E820_MEMORY_ENTRY
{
    QWORD BaseAddress;
    QWORD Length;
    DWORD Type;
    DWORD Reserved;
} E820_MEMORY_ENTRY, *PE820_MEMORY_ENTRY;

VOID
MmuInit(
    VOID
);

BOOLEAN
MmuCheckIfChunkExists(
    DWORD Base,
    DWORD Limit,
    PQWORD FoundBase,
    PQWORD FoundLimit
);

VOID
MmuPrintE820MemoryEntries(
    VOID
);

PVOID
MmuMapToVa(
    QWORD PhysicalAddress,
    QWORD Size,
    PQWORD AlignedSize,
    BOOLEAN Uncacheable
);

VOID
MmuUnmapFromVa(
    QWORD VirtualAddress,
    QWORD Size
);

PVOID
MmuAllocVa(
    QWORD Length
);

VOID
MmuMapToVaInternal(
    QWORD PhysicalAddress,
    QWORD Size,
    QWORD VirtualAddress,
    BOOLEAN Uncacheable
);

PVOID
MmuAllocAlignedVa(
    QWORD Length
);

VOID
MmuMapMemoryEntriesInEpt(
    VOID
);


VOID
MmuCreateModifiedE820MemoryMap(
    VOID
);


VOID
MmuPrintModifiedE820MemoryEntries(
    VOID
);

QWORD
MmuGetEntryPhysicalAddressFromPageTable(
    QWORD VirtualAddress,
    QWORD Pml4Address
);


VOID
MmuCreateModifiedE820MemoryMap(
    VOID
);

PE820_MEMORY_ENTRY
MmuGetModifiedE820MemoryEntry(
    DWORD Index,
    BOOLEAN* LastEntry
);

#endif // _MEMORY_MAP_H_