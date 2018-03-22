#ifndef _EPT_H
#define _EPT_H

#include "map.h"
#include "minihv.h"

#define EPT_TABLE_ENTRY_SIZE 0x8
#define EPT_TABLE_NUMBER_OF_ENTRIES 0x200
#define EPT_TABLE_SIZE (EPT_TABLE_ENTRY_SIZE * EPT_TABLE_NUMBER_OF_ENTRIES)

#define EPT_ENTRY_PRESENT(x) ((x)->ReadAccess || (x)->WriteAccess || (x)->ExecuteAccessSupervisorMode)

#define EPT_PML4_PA (PML4_ADDRESS + 10 * MB)

#define EPT_PHYSICAL_ADDRESS_SHIFT 12

#define EPT_PAGE_SIZE 0x1000

#define EPT_FOUR_LEVEL_PG_OFFSET_MASK       0x1FF // 9 bits are used to index different entries from the paging tables
#define EPT_FOUR_LEVEL_PG_PAGE_OFFSET_MASK  0xFFF // 12 bits used to offset a byte in a 4KB page

#define EPT_PML4_OFFSET(x)  ((x >> 39) & EPT_FOUR_LEVEL_PG_OFFSET_MASK)
#define EPT_PDPT_OFFSET(x)  ((x >> 30) & EPT_FOUR_LEVEL_PG_OFFSET_MASK)
#define EPT_PD_OFFSET(x)    ((x >> 21) & EPT_FOUR_LEVEL_PG_OFFSET_MASK)
#define EPT_PT_OFFSET(x)    ((x >> 12) & EPT_FOUR_LEVEL_PG_OFFSET_MASK)
#define EPT_PG_OFFSET(x)    (x & EPT_FOUR_LEVEL_PG_PAGE_OFFSET_MASK)

#define EPT_PAGE_WALK 4

#pragma warning (disable : 4214)
#pragma pack(push, 1)
typedef struct _EPT_PML4_ENTRY
{
    QWORD ReadAccess : 1;
    QWORD WriteAccess : 1;
    QWORD ExecuteAccessSupervisorMode : 1;
    QWORD Reserved0 : 5;
    QWORD AccessedFlag : 1;
    QWORD Ignored1 : 1;
    QWORD ExecuteAccessUserMode : 1;
    QWORD Ignored2 : 1;
    QWORD PhysicalAddress : 40;
    QWORD Ignored3 : 12;
} EPT_PML4_ENTRY, *PEPT_PML4_ENTRY;

typedef struct _EPT_PDPT_ENTRY
{
    QWORD ReadAccess : 1;
    QWORD WriteAccess : 1;
    QWORD ExecuteAccessSupervisorMode : 1;
    QWORD Reserved0 : 5;
    QWORD AccessedFlag : 1;
    QWORD Ignored1 : 1;
    QWORD ExecuteAccessUserMode : 1;
    QWORD Ignored2 : 1;
    QWORD PhysicalAddress : 40;
    QWORD Ignored3 : 12;
} EPT_PDPT_ENTRY, *PEPT_PDPT_ENTRY;

typedef struct _EPT_PD_ENTRY
{
    QWORD ReadAccess : 1;
    QWORD WriteAccess : 1;
    QWORD ExecuteAccessSupervisorMode : 1;
    QWORD Reserved0 : 5;
    QWORD AccessedFlag : 1;
    QWORD Ignored1 : 1;
    QWORD ExecuteAccessUserMode : 1;
    QWORD Ignored2 : 1;
    QWORD PhysicalAddress : 40;
    QWORD Ignored3 : 12;
} EPT_PD_ENTRY, *PEPT_PD_ENTRY;

typedef struct _EPT_PT_ENTRY
{
    QWORD ReadAccess : 1;
    QWORD WriteAccess : 1;
    QWORD ExecuteAccessSupervisorMode : 1;
    QWORD MemoryType : 3;
    QWORD IgnorePatMemoryType : 1;
    QWORD Ignored1 : 1;
    QWORD AccessedFlag : 1;
    QWORD DirtyFlag : 1;
    QWORD ExecuteAccessUserMode : 1;
    QWORD Ignored2 : 1;
    QWORD PhysicalAddress : 40;
    QWORD Ignored3 : 11;
    QWORD SuppressVe : 1;
} EPT_PT_ENTRY, *PEPT_PT_ENTRY;
#pragma pack(pop)

VOID
EptMapGuestPaToPa(
    QWORD GuestPhysicalAddress,
    QWORD Size,
    QWORD PhysicalAddress
);


VOID
EptInitialization(
    VOID
);

QWORD
EptGetEntryValue(
    QWORD GuestPhysicalAddress
);

QWORD
EptSetEntryValue(
    QWORD GuestPhysicalAddress,
    QWORD Value
);



#endif
