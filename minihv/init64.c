#include "minihv.h"
#include "print.h"
#include "string.h"
#include "crt.h"
#include "pci.h"

#include "dbglog.h"

#include "map.h"
#include "acpica.h"
#include "cmd.c"
#include "vvga.h"
#include "cconsole.h"
#include "acpica.h"
#include "local_apic.h"
#include "cpu.h"
#include "mtrr.h"
#include "ept.h"
#include "pcci.h"
#include "idt.h"

#include "vmx.h"
#include "vmx_common.h"

#define POINT_ASC '.'

CPU gCpu[NR_OF_CPUS];


DWORD m_numberOfCpusInVmxMode = 0;

int
ApInit64(
    VOID
)
{
    return 0;
}

int BasicInit(VOID)
{
    LOG_ERROR("WTNOT WRK");
    return 0;
}

BYTE
LapicGetCpuApicId(
    VOID
)
{
    int cpuInfo[4];
    BYTE cpuApicId;

    __cpuid(cpuInfo, 1);

    cpuApicId = cpuInfo[1] >> 24;

    return cpuApicId;
}

BOOLEAN
LapicCpuIsCurrent(
    BYTE LapicId
)
{
    return LapicGetCpuApicId() == LapicId;
}

VOID
InitCurrentProcessorVmx(
    VOID
)
{
    for (BYTE i = 0; i < GetNrOfCpus(); i++)
    {
        LOG_WARNING("init %d", i)
        if (LapicCpuIsCurrent(gCpu[i].ApicId))
        {
            VmxInit(&gCpu[i]);
            break;
        }
    }
}

int Init64(void)
{
    //
    // let's try to do a RDTSC demo...
    //
#pragma warning(suppress:4127)  // conditional expression is constant
    ACPI_STATUS lastACPIStatus;
    NTSTATUS    lastNTStatus;

//    DWORD idx;

    VVgaTextInit();
    ConsoleInit();
    InitSerOrVid();
    MmuInit();


    lastACPIStatus = AcpiInit();
    if (!ACPI_SUCCESS(lastACPIStatus))
    {
        LOG_ERROR("AcpiInit failed, status: [0x%08x]", lastACPIStatus);
        __halt();
    }
    else
    {
        LOG_ACPI("Calling AcpiGetProcessors...");
        lastACPIStatus = AcpiGetProcessors(gCpu);
        if (!ACPI_SUCCESS(lastACPIStatus))
        {
            LOG_ERROR("AcpiGetProcessors failed, status: [0x%08x]", lastACPIStatus);
            __halt();
        }
        LOG_INFO("AcpiGetProcessors succeeded");
    }
    IdtInit();

    /*
    char *s = "abc";
    int c = 5 / (3 - sMstrlen(s));
    c;*/

    lastNTStatus = LapicInit();
    if (!NT_SUCCESS(lastNTStatus))
    {
        LOG_ERROR("LapicInit failed, status: [0x%08x]", lastNTStatus);
        __halt();
    }
    LOG_INFO("Successfuly initialized local apic");

    LapicSetupTrampCode(gCpu);
    LapicSignalAllAPs(gCpu, 0x02);
    MtrrInit();
    EptInitialization();
    MmuMapMemoryEntriesInEpt();
    PciEcamRestrictAccessOnReservedSerialPort();
    VmxConfigureGuestCode();
    VmxPatchRealModeInt15hE820();

    
    //
    // This function should be used intead VmxInit (additional validations)
    //
    //MhvVmxInit(gCpu); // throw #UD
        
    lastNTStatus = VmxInit(gCpu);
    if (!NT_SUCCESS(lastNTStatus))
    {
        LOG_ERROR("VmxInit failed");
        __halt();
    }

    while (m_numberOfCpusInVmxMode != GetNrOfCpus() - 1)
    {
        LOG_WARNING("Not all cpus IN VMX MODE (yet)")
    }
    InitCurrentProcessorVmx();

    //if (MhvPresentHypervisor())
    //{
    //    LOG_ERROR("Mini Hypervisor is not present!");
    //    __halt();
    //}
    //else
    //{
    //    Sleep(SEC);
    //    LOG_INFO("Mini Hypervisor is present!");
    //}



    //LOG_WARN_HLT("");
    return 0;
}

VOID
IncrementNumberOfCpusInVmxMode(
    VOID
    )
{
    
    LOG_INFO("Current CPU: [%d]", m_numberOfCpusInVmxMode);
   
    m_numberOfCpusInVmxMode++;
    LOG_INFO("CPU contor incr. successfully");

}

VOID
DecrementNumberOfCpusInVmxMode(
    VOID
    )
{
    // Will be used only in case of fail (vmx_launch fails)
    m_numberOfCpusInVmxMode--;
}