#include "gdt.h"
#include "cpu.h"
#include "map.h"
#include "print.h"
#include "tss.h"
#include "acpi.h"

extern WORD apsGdtrLength;
extern QWORD apsGdtrAddress;

PQWORD m_gdt;

static GDTR m_gdtr;

VOID
GdtLoadGdtr(
    QWORD GdtAddress
)
{
    _lgdt(GdtAddress);
}

QWORD
GdtComputeTssDescriptorHighValue(
    QWORD TssAddress
)
{
    TSS_ENTRY_HIGH tssEntry = { 0 };

    tssEntry.BaseAddress = TssAddress >> 32;

    return tssEntry.Raw;
}

QWORD
GdtComputeTssDescriptorLowValue(
    QWORD TssAddress
)
{
    TSS_ENTRY_LOW tssEntry = { 0 };

    tssEntry.SegmentLimit0 = (TSS_SIZE - 1) & 0xFFFF;
    tssEntry.BaseAddress0 = TssAddress & 0xFFFF;
    tssEntry.BaseAddress1 = (TssAddress >> 16) & 0xFF;
    tssEntry.Type = 0x09;
    tssEntry.P = 1;
    tssEntry.BaseAddress2 = (TssAddress >> 24) & 0xFF;

    return tssEntry.Raw;
}

/// TODO: change TSS from the PCPU structure to PTSS
VOID
GdtSetupFinalGdt(
    CPU Cpus[8]
)
{
    DWORD numberOfCpus = GetNrOfCpus();

    m_gdtr.GdtLength = GDT_NULL_ENTRY_SIZE + GDT_CODE_ENTRY_SIZE + GDT_DATA_ENTRY_SIZE + (WORD)(numberOfCpus)* GDT_TSS_ENTRY_SIZE;
    m_gdt = MmuAllocVa(m_gdtr.GdtLength);
    m_gdtr.Gdt = m_gdt;

    m_gdt[0] = 0; // null descriptor
    m_gdt[1] = GDT_CODE64_DESCRIPTOR; // code descriptor
    m_gdt[2] = GDT_DATA64_DESCRIPTOR; // data descriptor

                                      // tss descriptors; here we assume that the first PCPU is the BSP
    for (DWORD i = 0; i < numberOfCpus; i++)
    {
        m_gdt[3 + 2 * i] = GdtComputeTssDescriptorLowValue((QWORD)(&Cpus[i].Tss));;
        m_gdt[3 + 2 * i + 1] = GdtComputeTssDescriptorHighValue((QWORD)(&Cpus[i].Tss));

        TssInstallStacks(&Cpus[i].Tss);
    }

    // final values of the APs GDT
    apsGdtrLength = m_gdtr.GdtLength;
    apsGdtrAddress = (QWORD)m_gdt;

    // final values of the BSPs GDT
    GdtLoadGdtr((QWORD)&m_gdtr);
}

PQWORD
GdtGetGdtrBaseAddress(
    VOID
)
{
    return m_gdtr.Gdt;
}

WORD
GdtGetGdtrLimit(
    VOID
)
{
    return m_gdtr.GdtLength;
}