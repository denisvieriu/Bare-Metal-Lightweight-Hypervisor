#include "minihv.h"
#include "map.h"
#include "dbglog.h"
#include "memzero.h"
#include "console.h"
#include "pci.h"
#include "ept.h"
#include "mtrr.h"
#include "vmx.h"
#include "cconsole.h"
typedef struct _PML4_ENTRY
{
    QWORD Present : 1;
    QWORD ReadWrite : 1;
    QWORD UserSupervisor : 1;
    QWORD PWT : 1;
    QWORD PCD : 1;
    QWORD Accessed : 1;
    QWORD Ignored0 : 1;
    QWORD Reserved : 1;  // Must be 0
    QWORD Ignored1 : 4;
    QWORD PhysicalAddress : 40;
    QWORD Ignored2 : 11;
    QWORD XD : 1;
} PML4_ENTRY, *PPML4_ENTRY;

typedef struct _PDPT_ENTRY
{
    QWORD Present : 1;
    QWORD ReadWrite : 1;
    QWORD UserSupervisor : 1;
    QWORD PWT : 1;
    QWORD PCD : 1;
    QWORD Accessed : 1;
    QWORD Ignored0 : 1;
    QWORD PageSize : 1;  // Must be 0
    QWORD Ignored1 : 4;
    QWORD PhysicalAddress : 40;
    QWORD Ignored2 : 11;
    QWORD XD : 1;
} PDPT_ENTRY, *PPDPT_ENTRY;

typedef struct _PD_ENTRY
{
    QWORD Present : 1;
    QWORD ReadWrite : 1;
    QWORD UserSupervisor : 1;
    QWORD PWT : 1;
    QWORD PCD : 1;
    QWORD Accessed : 1;
    QWORD Ignored0 : 1;
    QWORD PageSize : 1;  // Must be 0
    QWORD Ignored1 : 4;
    QWORD PhysicalAddress : 40;
    QWORD Ignored2 : 11;
    QWORD XD : 1;
} PD_ENTRY, *PPD_ENTRY;

typedef struct _PD_ENTRY_2MB_PAGE
{
    QWORD Present : 1;
    QWORD ReadWrite : 1;
    QWORD UserSupervisor : 1;
    QWORD PWT : 1;
    QWORD PCD : 1;
    QWORD Accessed : 1;
    QWORD Dirty : 1;
    QWORD PageSize : 1;  // Must be 0
    QWORD Global : 1;
    QWORD Ignored1 : 3;
    QWORD Pat : 1;
    QWORD Reserved : 8;
    QWORD PhysicalAddress : 31;
    QWORD Ignored2 : 7;
    QWORD ProtectionKey : 4;
    QWORD XD : 1;
} PD_ENTRY_2MB_PAGE, *PPD_ENTRY_2MB_PAGE;

typedef struct _PT_ENTRY
{
    QWORD Present : 1;
    QWORD ReadWrite : 1;
    QWORD UserSupervisor : 1;
    QWORD PWT : 1;
    QWORD PCD : 1;
    QWORD Accessed : 1;
    QWORD Dirty : 1;
    QWORD Pat : 1;
    QWORD Global : 1;
    QWORD Ignored0 : 3;
    QWORD PhysicalAddress : 40;
    QWORD Ignored2 : 7;
    QWORD ProtectionKey : 4;
    QWORD XD : 1;
} PT_ENTRY, *PPT_ENTRY;

extern unsigned int gTempE820EntriesExt;

static PE820_MEMORY_ENTRY m_e820MemoryEntriesBuffer = (PE820_MEMORY_ENTRY)((char *)&gTempE820EntriesExt + 4);

static
QWORD
MmuGetMaxE820Entry(
    VOID
)
{
    QWORD maxAddress;

    if (gTempE820EntriesExt < 1)
    {
        return 0;
    }

    maxAddress = m_e820MemoryEntriesBuffer[0].BaseAddress + m_e820MemoryEntriesBuffer[0].Length;

    for (DWORD i = 1; i < gTempE820EntriesExt; i++)
    {
        if (maxAddress < m_e820MemoryEntriesBuffer[i].BaseAddress + m_e820MemoryEntriesBuffer[i].Length)
        {
            maxAddress = m_e820MemoryEntriesBuffer[i].BaseAddress + m_e820MemoryEntriesBuffer[i].Length;
        }
    }

    return maxAddress;
}


VOID
MmuInit(
    VOID
)
{
    MmuPrintE820MemoryEntries();

    QWORD BaseAddress;
    QWORD Length;

    if (!MmuCheckIfChunkExists(KERNEL_ADDRESS, 200 * MB, &BaseAddress, &Length))
    {
        __halt();
    }
    // clean PML4 table
    memzero((PBYTE)PA2VA(PML4_ADDRESS), TABLE_SIZE);

    MmuMapToVaInternal(KERNEL_ADDRESS, KERNEL_RESERVED_MEMORY, KERNEL_VA_START, FALSE);
    MmuMapToVaInternal(VIDEO_MEMORY_ADDRESS, PAGE_SIZE * 2, VIDEO_MEMORY_ADDRESS, FALSE);

    __writecr3(PML4_ADDRESS);

    //LOG_INFO("Initialized MMU");
}

static
BOOLEAN
MmuIsAvailableChunk(
    DWORD Type
)
{
    return Type == 1;
}

VOID
MmuPrintE820MemoryEntries(
    VOID
)
{
    LOG_INFO("Number of reported memory entries: %X:%u\n\n", &gTempE820EntriesExt, gTempE820EntriesExt);

    for (DWORD i = 0; i < gTempE820EntriesExt; i++)
    {
        LOG_INFO("BaseAddress: %X", m_e820MemoryEntriesBuffer[i].BaseAddress);
        LOG_INFO("Length: %X", m_e820MemoryEntriesBuffer[i].Length);
        LOG_INFO("MaxAddress: %X", m_e820MemoryEntriesBuffer[i].BaseAddress + m_e820MemoryEntriesBuffer[i].Length - 1);
        LOG_INFO("Type: %X\n", (QWORD)m_e820MemoryEntriesBuffer[i].Type);
    }
}

BOOLEAN
MmuCheckIfChunkExists(
    DWORD Base,
    DWORD Limit,
    PQWORD FoundBase,
    PQWORD FoundLimit
)
{
    for (DWORD i = 0; i < gTempE820EntriesExt; i++)
    {
        if (m_e820MemoryEntriesBuffer[i].BaseAddress <= Base &&
            m_e820MemoryEntriesBuffer[i].BaseAddress + m_e820MemoryEntriesBuffer[i].Length >= Base + Limit &&
            MmuIsAvailableChunk(m_e820MemoryEntriesBuffer[i].Type))
        {
            *FoundBase = m_e820MemoryEntriesBuffer[i].BaseAddress;
            *FoundLimit = m_e820MemoryEntriesBuffer[i].Length;

            LOG_INFO("Found available RAM;  BASEADRESS: [0%08x] | Length: [0%08x]", m_e820MemoryEntriesBuffer[i].BaseAddress, m_e820MemoryEntriesBuffer[i].Length);
            return TRUE;
        }
    }

    *FoundBase = 0;
    *FoundLimit = 0;

    return FALSE;
}

static
QWORD
MmuRetrieveNextTablePhysicalAddress(
    VOID
)
{
    static QWORD currentPdptTablePa = PML4_ADDRESS;

    currentPdptTablePa += TABLE_SIZE;

    return currentPdptTablePa;
}




VOID
MmuMapToVaInternal(
    QWORD PhysicalAddress,
    QWORD Size,
    QWORD VirtualAddress,
    BOOLEAN Uncacheable
)
{
    PPML4_ENTRY pml4Table;
    PPDPT_ENTRY pdptTable;
    PPD_ENTRY pdTable;
    PPT_ENTRY pt;
    QWORD pml4Index;
    QWORD pdptIndex;
    QWORD pdIndex;
    QWORD ptIndex;

    for (QWORD Offset = 0; Offset < Size; Offset += PAGE_SIZE)
    {
        pml4Index = PML4_OFFSET(VirtualAddress);
        pdptIndex = PDPT_OFFSET(VirtualAddress);
        pdIndex = PD_OFFSET(VirtualAddress);
        ptIndex = PT_OFFSET(VirtualAddress);

        pml4Table = (PPML4_ENTRY)PA2VA(PML4_ADDRESS);
        pml4Table = &pml4Table[pml4Index];
        if (pml4Table->Present == 0) // we need to allocate a new pml4 entry in the pml4 table
        {
            memzero((PBYTE)pml4Table, TABLE_ENTRY_SIZE);

            pml4Table->Present = 1;
            pml4Table->ReadWrite = 1;
            QWORD tempPhysicalAddress = MmuRetrieveNextTablePhysicalAddress();
            pml4Table->PhysicalAddress = tempPhysicalAddress >> PHYSICAL_ADDRESS_SHIFT;
            memzero((PBYTE)PA2VA(tempPhysicalAddress), TABLE_SIZE);
        }

        pdptTable = (PPDPT_ENTRY)PA2VA(&(((PPDPT_ENTRY)(pml4Table->PhysicalAddress << PHYSICAL_ADDRESS_SHIFT))[pdptIndex]));
        if (pdptTable->Present == 0) // we need to allocate a new pdpt entry in the pdpt table
        {
            memzero((PBYTE)pdptTable, TABLE_ENTRY_SIZE);

            pdptTable->Present = 1;
            pdptTable->ReadWrite = 1;
            QWORD tempPhysicalAddress = MmuRetrieveNextTablePhysicalAddress();
            pdptTable->PhysicalAddress = tempPhysicalAddress >> PHYSICAL_ADDRESS_SHIFT;
            memzero((PBYTE)PA2VA(tempPhysicalAddress), TABLE_SIZE);
        }

        pdTable = (PPD_ENTRY)PA2VA(&(((PPD_ENTRY)(pdptTable->PhysicalAddress << PHYSICAL_ADDRESS_SHIFT))[pdIndex]));
        if (pdTable->Present == 0) // we need to allocate a new pd entry in the pd table
        {
            memzero((PBYTE)pdTable, TABLE_ENTRY_SIZE);

            pdTable->Present = 1;
            pdTable->ReadWrite = 1;
            QWORD tempPhysicalAddress = MmuRetrieveNextTablePhysicalAddress();
            pdTable->PhysicalAddress = tempPhysicalAddress >> PHYSICAL_ADDRESS_SHIFT;
            memzero((PBYTE)PA2VA(tempPhysicalAddress), TABLE_SIZE);
        }

        pt = (PPT_ENTRY)PA2VA(&(((PPT_ENTRY)(pdTable->PhysicalAddress << PHYSICAL_ADDRESS_SHIFT))[ptIndex]));
        if (pt->Present == 0) // 4KB page is not mapped
        {
            memzero((PBYTE)pt, TABLE_ENTRY_SIZE);

            pt->Present = 1;
            pt->ReadWrite = 1;
            pt->PCD = Uncacheable;
            pt->PWT = Uncacheable;
            pt->PhysicalAddress = PhysicalAddress >> PHYSICAL_ADDRESS_SHIFT;
        }

        PhysicalAddress += PAGE_SIZE;
        VirtualAddress += PAGE_SIZE;
    }
}

static
VOID
MmuUnmapFromVaInternal(
    QWORD VirtualAddress,
    QWORD Size
)
{
    PPML4_ENTRY pml4Table;
    PPDPT_ENTRY pdptTable;
    PPD_ENTRY pdTable;
    PPT_ENTRY pt;
    QWORD pml4Index;
    QWORD pdptIndex;
    QWORD pdIndex;
    QWORD ptIndex;

    for (QWORD Offset = 0; Offset < Size; Offset += PAGE_SIZE)
    {
        pml4Index = PML4_OFFSET(VirtualAddress);
        pdptIndex = PDPT_OFFSET(VirtualAddress);
        pdIndex = PD_OFFSET(VirtualAddress);
        ptIndex = PT_OFFSET(VirtualAddress);

        pml4Table = (PPML4_ENTRY)PA2VA(PML4_ADDRESS);
        pml4Table = &pml4Table[pml4Index];
        if (!pml4Table->Present)
        {
            continue;
        }

        pdptTable = (PPDPT_ENTRY)PA2VA(&((PPDPT_ENTRY)(pml4Table->PhysicalAddress << PHYSICAL_ADDRESS_SHIFT))[pdptIndex]);
        if (!pdptTable->Present)
        {
            continue;
        }

        pdTable = (PPD_ENTRY)PA2VA(&((PPD_ENTRY)(pdptTable->PhysicalAddress << PHYSICAL_ADDRESS_SHIFT))[pdIndex]);
        if (!pdTable->Present)
        {
            continue;
        }

        pt = (PPT_ENTRY)PA2VA(&((PPT_ENTRY)(pdTable->PhysicalAddress << PHYSICAL_ADDRESS_SHIFT))[ptIndex]);
        if (!pt->Present)
        {
            continue;
        }

        pt->Present = 0;

        VirtualAddress += PAGE_SIZE;
    }
}

PVOID
MmuMapToVa(
    QWORD PhysicalAddress,
    QWORD Size,
    PQWORD AlignedSize,
    BOOLEAN Uncacheable
)
{
    QWORD va;
    DWORD alignmentDifference;
    //IncrementLoaded(1, SET_MINIHV_BAR);
    //IncrementLoaded(1, SET_LOAD_BAR);
    alignmentDifference = (DWORD)AddressOffset(PhysicalAddress, PAGE_SIZE);
    PhysicalAddress = (QWORD)AlignAddressLower(PhysicalAddress, PAGE_SIZE);
    Size = AlignAddressUpper(Size + alignmentDifference, PAGE_SIZE);

    if (AlignedSize != NULL)
    {
        *AlignedSize = Size;
    }

    va = PA2VA(PhysicalAddress);

    //LOG_WARNING("Mapping PhysicalAddress %X of Size %X to VirtualAddress %X", PhysicalAddress, Size, va);
    //Sleep(SEC * 50);

    MmuMapToVaInternal(PhysicalAddress, Size, (QWORD)va, Uncacheable);

    return (PVOID)(va + alignmentDifference);
}

VOID
MmuUnmapFromVa(
    QWORD VirtualAddress,
    QWORD Size
)
{
    DWORD alignmentDifference;

    alignmentDifference = (DWORD)AddressOffset(VirtualAddress, PAGE_SIZE);
    VirtualAddress = AlignAddressLower(VirtualAddress, PAGE_SIZE);
    Size = AlignAddressUpper(Size + alignmentDifference, PAGE_SIZE);

    //LOG_WARNING("Unmapping VirtualAddress %X of Size %X", VirtualAddress, Size);
    //Sleep(SEC * 100);
    //IncrementLoaded(1, SET_MINIHV_BAR);
    //IncrementLoaded(1, SET_LOAD_BAR);
    MmuUnmapFromVaInternal(VirtualAddress, Size);
}

VOID
MmuGetNextAvailableVa(
    QWORD Length,
    PQWORD NextVa,
    BOOLEAN Aligned
)
{
    static QWORD nextAvailableVa = PA2VA(HEAP_ADDRESS);
    DWORD alignmentDifference;

    if (Aligned)
    {
        alignmentDifference = (DWORD)AddressOffset(nextAvailableVa, PAGE_SIZE);
        Length = AlignAddressUpper(Length + alignmentDifference, PAGE_SIZE);

        nextAvailableVa += (PAGE_SIZE - alignmentDifference);
    }

    *NextVa = nextAvailableVa;

    nextAvailableVa += Length;
}

PVOID
MmuAllocAlignedVa(
    QWORD Length
)
{
    QWORD nextVa;

    MmuGetNextAvailableVa(Length, &nextVa, TRUE);

    return (PVOID)nextVa;
}


PVOID
MmuAllocVa(
    QWORD Length
)
{
    QWORD nextVa;

    MmuGetNextAvailableVa(Length, &nextVa, TRUE);

    return (PVOID)nextVa;
}


VOID
MmuMapMemoryEntriesInEpt(
    VOID
)
{
    QWORD physicalAddressWidth = GetCpuMaxPhysicalAddress();
    QWORD maxPhysicalAddress = 1ULL << physicalAddressWidth;

    LOG("physicalAddressWidth %X", physicalAddressWidth);
    LOG("maxPhysicalAddress %X", maxPhysicalAddress);

    EptMapGuestPaToPa(0, KERNEL_ADDRESS, 0);
    EptMapGuestPaToPa(KERNEL_ADDRESS + KERNEL_RESERVED_MEMORY, MmuGetMaxE820Entry(), KERNEL_ADDRESS + KERNEL_RESERVED_MEMORY);

    LOG("Done mapping in EPT");
}

QWORD
MmuGetEntryPhysicalAddressFromPageTable(
    QWORD VirtualAddress,
    QWORD Pml4Address
)
{
    PPML4_ENTRY pPml4Entry = (PPML4_ENTRY)MmuMapToVa(Pml4Address & ~FOUR_LEVEL_PG_PAGE_OFFSET_MASK, PAGE_SIZE, NULL, FALSE);
    PPDPT_ENTRY pPdptEntry = NULL;
    PPD_ENTRY pPdEntry = NULL;
    PPT_ENTRY pPtEntry = NULL;

    WORD pml4Index;
    WORD pdptIndex;
    WORD pdIndex;
    WORD ptIndex;

    QWORD entryValue;
    QWORD pageOffset;

    pml4Index = PML4_OFFSET(VirtualAddress);
    pdptIndex = PDPT_OFFSET(VirtualAddress);
    pdIndex = PD_OFFSET(VirtualAddress);
    ptIndex = PT_OFFSET(VirtualAddress);

    if (!pPml4Entry[pml4Index].Present)
    {
        MmuUnmapFromVa((QWORD)pPml4Entry, PAGE_SIZE);
        return 0;
    }

    pPdptEntry = MmuMapToVa(pPml4Entry[pml4Index].PhysicalAddress << PHYSICAL_ADDRESS_SHIFT, PAGE_SIZE, NULL, FALSE);
    MmuUnmapFromVa((QWORD)pPml4Entry, PAGE_SIZE);

    if (!pPdptEntry[pdptIndex].Present)
    {
        MmuUnmapFromVa((QWORD)pPdptEntry, PAGE_SIZE);
        return 0;
    }

    pPdEntry = MmuMapToVa(pPdptEntry[pdptIndex].PhysicalAddress << PHYSICAL_ADDRESS_SHIFT, PAGE_SIZE, NULL, FALSE);
    MmuUnmapFromVa((QWORD)pPdptEntry, PAGE_SIZE);

    if (!pPdEntry[pdIndex].Present)
    {
        MmuUnmapFromVa((QWORD)pPdEntry, PAGE_SIZE);
        return 0;
    }

    if (pPdEntry[pdIndex].PageSize == 1)
    {
        PPD_ENTRY_2MB_PAGE pPdEntry2Mb = (PPD_ENTRY_2MB_PAGE)pPdEntry;

        pageOffset = VirtualAddress & 0x1FFFFF;
        entryValue = (pPdEntry2Mb[pdIndex].PhysicalAddress << 21) + pageOffset;
        MmuUnmapFromVa((QWORD)pPdEntry, PAGE_SIZE);

        return entryValue;
    }

    pPtEntry = MmuMapToVa(pPdEntry[pdIndex].PhysicalAddress << PHYSICAL_ADDRESS_SHIFT, PAGE_SIZE, NULL, FALSE);
    MmuUnmapFromVa((QWORD)pPdEntry, PAGE_SIZE);

    if (!pPtEntry[ptIndex].Present)
    {
        MmuUnmapFromVa((QWORD)pPtEntry, PAGE_SIZE);
        return 0;
    }

    pageOffset = VirtualAddress & FOUR_LEVEL_PG_PAGE_OFFSET_MASK;
    entryValue = (pPtEntry[ptIndex].PhysicalAddress << PHYSICAL_ADDRESS_SHIFT) + pageOffset;

    MmuUnmapFromVa((QWORD)pPtEntry, PAGE_SIZE);

    return entryValue;
}



typedef enum _E820_MEMORY_TYPE
{
    E820MemoryTypeUsable = 1,
    E820MemoryTypeUnusable,
    E820MemoryTypeACPIReclaimable,
    E820MemoryTypeACPINVS,
    E820MemoryTypeBadMemory
} E820_MEMORY_TYPE, *PE820_MEMORY_TYPE;

static PE820_MEMORY_ENTRY m_modifiedE820MemoryMap;
static DWORD m_numberOfModifiedE820MemoryEntries;

VOID
MmuPrintModifiedE820MemoryEntries(
    VOID
)
{
    for (DWORD idxEntries = 0; idxEntries < m_numberOfModifiedE820MemoryEntries; idxEntries++)
    {
        DEF_PRINTF("Modified E820 Memory Map:\n"
            "Index:         %d      \n"
            "Base address:  [0x%08X]\n"
            "Length:        [0x%08X]\n"
            "Reserved:      [0x%08x]\n"
            "Type:          [0x%08x]\n",
            idxEntries,
            m_modifiedE820MemoryMap[idxEntries].BaseAddress, 
            m_modifiedE820MemoryMap[idxEntries].Length, 
            m_modifiedE820MemoryMap[idxEntries].Reserved, 
            m_modifiedE820MemoryMap[idxEntries].Type);
    }
}


PE820_MEMORY_ENTRY
MmuGetModifiedE820MemoryEntry(
    DWORD Index,
    BOOLEAN* LastEntry
)
{
    if (LastEntry == NULL)
    {
        return NULL;
    }

    if (Index >= m_numberOfModifiedE820MemoryEntries)
    {
        return NULL;
    }
    else if (Index == m_numberOfModifiedE820MemoryEntries - 1)
    {
        *LastEntry = TRUE;
    }
    else
    {
        *LastEntry = FALSE;
    }

    return &m_modifiedE820MemoryMap[Index];
}

VOID
MmuCreateModifiedE820MemoryMap(
    VOID
)
{
    m_modifiedE820MemoryMap = MmuAllocVa(PAGE_SIZE);
    DWORD currentModifiedMemoryMapEntryIndex = 0;

    for (DWORD i = 0; i < gTempE820EntriesExt; i++)
    {
        if (m_e820MemoryEntriesBuffer[i].BaseAddress <= KERNEL_ADDRESS &&
            m_e820MemoryEntriesBuffer[i].BaseAddress + m_e820MemoryEntriesBuffer[i].Length > KERNEL_ADDRESS + KERNEL_RESERVED_MEMORY)
        {
            LOG("Found E820 entry which contains the kernel address space");

            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].BaseAddress = m_e820MemoryEntriesBuffer[i].BaseAddress;
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Length = KERNEL_ADDRESS - m_e820MemoryEntriesBuffer[i].BaseAddress;
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Type = m_e820MemoryEntriesBuffer[i].Type;

            currentModifiedMemoryMapEntryIndex++;

            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].BaseAddress = KERNEL_ADDRESS;
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Length = KERNEL_RESERVED_MEMORY;
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Type = E820MemoryTypeUnusable;

            currentModifiedMemoryMapEntryIndex++;

            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].BaseAddress = KERNEL_ADDRESS + KERNEL_RESERVED_MEMORY;
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Length = m_e820MemoryEntriesBuffer[i].BaseAddress + m_e820MemoryEntriesBuffer[i].Length - (KERNEL_ADDRESS + KERNEL_RESERVED_MEMORY);
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Type = m_e820MemoryEntriesBuffer[i].Type;
        }
        else if (m_e820MemoryEntriesBuffer[i].BaseAddress <= GUEST_MBR_LOADER_LOCATION_PA &&
            m_e820MemoryEntriesBuffer[i].BaseAddress + m_e820MemoryEntriesBuffer[i].Length > GUEST_MBR_LOADER_LOCATION_PA + GUEST_RESERVED_MEMORY)
        {
            LOG("Found E820 entry which contains the guest code");

            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].BaseAddress = m_e820MemoryEntriesBuffer[i].BaseAddress;
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Length = GUEST_MBR_LOADER_LOCATION_PA - m_e820MemoryEntriesBuffer[i].BaseAddress;
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Type = m_e820MemoryEntriesBuffer[i].Type;

            currentModifiedMemoryMapEntryIndex++;

            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].BaseAddress = GUEST_MBR_LOADER_LOCATION_PA;
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Length = GUEST_RESERVED_MEMORY;
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Type = E820MemoryTypeUnusable;

            currentModifiedMemoryMapEntryIndex++;

            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].BaseAddress = GUEST_MBR_LOADER_LOCATION_PA + GUEST_RESERVED_MEMORY;
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Length = m_e820MemoryEntriesBuffer[i].BaseAddress + m_e820MemoryEntriesBuffer[i].Length - (GUEST_MBR_LOADER_LOCATION_PA + GUEST_RESERVED_MEMORY);
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex].Type = m_e820MemoryEntriesBuffer[i].Type;
        }
        else
        {
            m_modifiedE820MemoryMap[currentModifiedMemoryMapEntryIndex] = m_e820MemoryEntriesBuffer[i];
        }

        currentModifiedMemoryMapEntryIndex++;
    }

    m_numberOfModifiedE820MemoryEntries = currentModifiedMemoryMapEntryIndex;
}
