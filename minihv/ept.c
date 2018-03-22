#include "ept.h"
#include "map.h"
#include "string.h"
#include "print.h"
#include "vmx.h"
#include "mtrr.h"
#include "memzero.h"

VOID
EptInitialization(
    VOID
)
{
    memzero((PBYTE)PA2VA(EPT_PML4_PA), EPT_TABLE_SIZE);
}


static
QWORD
EptGetNextTableAddress(
    VOID
)
{
    static QWORD currentTableAddress = EPT_PML4_PA;

    currentTableAddress += EPT_TABLE_SIZE;

    return currentTableAddress;
}

VOID
EptMapGuestPaToPa(
    QWORD GuestPhysicalAddress,
    QWORD Size,
    QWORD PhysicalAddress
)
{
    PEPT_PML4_ENTRY pEptPml4Entry = NULL;
    PEPT_PDPT_ENTRY pEptPdptEntry = NULL;
    PEPT_PD_ENTRY pEptPdEntry = NULL;
    PEPT_PT_ENTRY pEptPtEntry = NULL;

    PEPT_PML4_ENTRY pEptPml4EntryReal = VmxGetPml4Pa();

    WORD pml4Index;
    WORD pdptIndex;
    WORD pdIndex;
    WORD ptIndex;

    WORD pml4IndexOld = MAX_WORD;
    WORD pdptIndexOld = MAX_WORD;
    WORD pdIndexOld = MAX_WORD;

    for (QWORD Offset = 0; Offset < Size; Offset += EPT_PAGE_SIZE)
    {
        pml4Index = EPT_PML4_OFFSET(GuestPhysicalAddress);
        pdptIndex = EPT_PDPT_OFFSET(GuestPhysicalAddress);
        pdIndex = EPT_PD_OFFSET(GuestPhysicalAddress);
        ptIndex = EPT_PT_OFFSET(GuestPhysicalAddress);

        if (pdIndex == pdIndexOld)
        {
            goto map_pt;
        }

        if (pdptIndex == pdptIndexOld)
        {
            goto map_pd;
        }

        if (pml4Index == pml4IndexOld)
        {
            goto map_pdpt;
        }

        pEptPml4Entry = &(((PEPT_PML4_ENTRY)PA2VA(pEptPml4EntryReal))[pml4Index]);
        if (!EPT_ENTRY_PRESENT(pEptPml4Entry))
        {
            memzero((PBYTE)pEptPml4Entry, EPT_TABLE_ENTRY_SIZE);

            pEptPml4Entry->ReadAccess = 1;
            pEptPml4Entry->WriteAccess = 1;
            pEptPml4Entry->ExecuteAccessSupervisorMode = 1;

            pEptPml4Entry->PhysicalAddress = EptGetNextTableAddress() >> EPT_PHYSICAL_ADDRESS_SHIFT;

            memzero((PBYTE)PA2VA(pEptPml4Entry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT), EPT_TABLE_SIZE);
        }

    map_pdpt:

        pEptPdptEntry = (PEPT_PDPT_ENTRY)(&((PEPT_PML4_ENTRY)PA2VA(pEptPml4Entry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT))[pdptIndex]);
        if (!EPT_ENTRY_PRESENT(pEptPdptEntry))
        {
            memzero((PBYTE)pEptPdptEntry, EPT_TABLE_ENTRY_SIZE);

            pEptPdptEntry->ReadAccess = 1;
            pEptPdptEntry->WriteAccess = 1;
            pEptPdptEntry->ExecuteAccessSupervisorMode = 1;

            pEptPdptEntry->PhysicalAddress = EptGetNextTableAddress() >> EPT_PHYSICAL_ADDRESS_SHIFT;

            memzero((PBYTE)PA2VA(pEptPdptEntry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT), EPT_TABLE_SIZE);
        }

    map_pd:

        pEptPdEntry = (PEPT_PD_ENTRY)(&((PEPT_PDPT_ENTRY)PA2VA(pEptPdptEntry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT))[pdIndex]);
        if (!EPT_ENTRY_PRESENT(pEptPdEntry))
        {
            memzero((PBYTE)pEptPdEntry, EPT_TABLE_ENTRY_SIZE);

            pEptPdEntry->ReadAccess = 1;
            pEptPdEntry->WriteAccess = 1;
            pEptPdEntry->ExecuteAccessSupervisorMode = 1;

            pEptPdEntry->PhysicalAddress = EptGetNextTableAddress() >> EPT_PHYSICAL_ADDRESS_SHIFT;

            memzero((PBYTE)PA2VA(pEptPdEntry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT), EPT_TABLE_SIZE);
        }

    map_pt:

        pEptPtEntry = (PEPT_PT_ENTRY)(&((PEPT_PD_ENTRY)PA2VA(pEptPdEntry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT))[ptIndex]);
        if (!EPT_ENTRY_PRESENT(pEptPtEntry))
        {
            memzero((PBYTE)pEptPtEntry, EPT_TABLE_ENTRY_SIZE);

            pEptPtEntry->ReadAccess = 1;
            pEptPtEntry->WriteAccess = 1;
            pEptPtEntry->ExecuteAccessSupervisorMode = 1;

            pEptPtEntry->IgnorePatMemoryType = 0;

            pEptPtEntry->MemoryType = MtrrGetPageMemoryType(GuestPhysicalAddress);

            pEptPtEntry->PhysicalAddress = PhysicalAddress >> EPT_PHYSICAL_ADDRESS_SHIFT;
        }

        PhysicalAddress += EPT_PAGE_SIZE;
        GuestPhysicalAddress += EPT_PAGE_SIZE;

        pml4IndexOld = pml4Index;
        pdptIndexOld = pdptIndex;
        pdIndexOld = pdIndex;
    }

}

QWORD
EptGetEntryValue(
    QWORD GuestPhysicalAddress
)
{
    PEPT_PML4_ENTRY pEptPml4Entry = NULL;
    PEPT_PDPT_ENTRY pEptPdptEntry = NULL;
    PEPT_PD_ENTRY pEptPdEntry = NULL;
    PEPT_PT_ENTRY pEptPtEntry = NULL;

    PEPT_PML4_ENTRY pEptPml4EntryReal = VmxGetPml4Pa();

    WORD pml4Index;
    WORD pdptIndex;
    WORD pdIndex;
    WORD ptIndex;

    QWORD entryValue;

    pml4Index = EPT_PML4_OFFSET(GuestPhysicalAddress);
    pdptIndex = EPT_PDPT_OFFSET(GuestPhysicalAddress);
    pdIndex = EPT_PD_OFFSET(GuestPhysicalAddress);
    ptIndex = EPT_PT_OFFSET(GuestPhysicalAddress);

    pEptPml4Entry = &(((PEPT_PML4_ENTRY)PA2VA(pEptPml4EntryReal))[pml4Index]);
    if (!EPT_ENTRY_PRESENT(pEptPml4Entry))
    {
        return 0;
    }

    pEptPdptEntry = (PEPT_PDPT_ENTRY)(&((PEPT_PML4_ENTRY)PA2VA(pEptPml4Entry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT))[pdptIndex]);
    if (!EPT_ENTRY_PRESENT(pEptPdptEntry))
    {
        return 0;
    }

    pEptPdEntry = (PEPT_PD_ENTRY)(&((PEPT_PDPT_ENTRY)PA2VA(pEptPdptEntry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT))[pdIndex]);
    if (!EPT_ENTRY_PRESENT(pEptPdEntry))
    {
        return 0;
    }

    pEptPtEntry = (PEPT_PT_ENTRY)(&((PEPT_PD_ENTRY)PA2VA(pEptPdEntry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT))[ptIndex]);
    if (!EPT_ENTRY_PRESENT(pEptPtEntry))
    {
        return 0;
    }

    memcpy(&entryValue, pEptPtEntry, EPT_TABLE_ENTRY_SIZE);
    return entryValue;
}

QWORD
EptSetEntryValue(
    QWORD GuestPhysicalAddress,
    QWORD Value
)
{
    PEPT_PML4_ENTRY pEptPml4Entry = NULL;
    PEPT_PDPT_ENTRY pEptPdptEntry = NULL;
    PEPT_PD_ENTRY pEptPdEntry = NULL;
    PEPT_PT_ENTRY pEptPtEntry = NULL;

    PEPT_PML4_ENTRY pEptPml4EntryReal = VmxGetPml4Pa();

    WORD pml4Index;
    WORD pdptIndex;
    WORD pdIndex;
    WORD ptIndex;

    pml4Index = EPT_PML4_OFFSET(GuestPhysicalAddress);
    pdptIndex = EPT_PDPT_OFFSET(GuestPhysicalAddress);
    pdIndex = EPT_PD_OFFSET(GuestPhysicalAddress);
    ptIndex = EPT_PT_OFFSET(GuestPhysicalAddress);

    pEptPml4Entry = &(((PEPT_PML4_ENTRY)PA2VA(pEptPml4EntryReal))[pml4Index]);
    if (!EPT_ENTRY_PRESENT(pEptPml4Entry))
    {
        return 0;
    }

    pEptPdptEntry = (PEPT_PDPT_ENTRY)(&((PEPT_PML4_ENTRY)PA2VA(pEptPml4Entry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT))[pdptIndex]);
    if (!EPT_ENTRY_PRESENT(pEptPdptEntry))
    {
        return 0;
    }

    pEptPdEntry = (PEPT_PD_ENTRY)(&((PEPT_PDPT_ENTRY)PA2VA(pEptPdptEntry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT))[pdIndex]);
    if (!EPT_ENTRY_PRESENT(pEptPdEntry))
    {
        return 0;
    }

    pEptPtEntry = (PEPT_PT_ENTRY)(&((PEPT_PD_ENTRY)PA2VA(pEptPdEntry->PhysicalAddress << EPT_PHYSICAL_ADDRESS_SHIFT))[ptIndex]);
    if (!EPT_ENTRY_PRESENT(pEptPtEntry))
    {
        return 0;
    }

    memcpy(pEptPtEntry, &Value, EPT_TABLE_ENTRY_SIZE);

    return Value;
}