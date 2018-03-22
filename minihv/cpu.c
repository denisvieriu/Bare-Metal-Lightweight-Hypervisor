#include "cpu.h"
#include "dbglog.h"

extern DWORD gNrOfCpus;

VOID DumpCpuInfo(PCPU cpu)
{
    DWORD idx;
    for (idx = 0; idx < gNrOfCpus; idx++)
    {
        LOG_INFO("Processor ID : [0x%08x]", cpu[idx].ProcessorId);
        LOG_INFO("Apic ID :      [0x%08x]", cpu[idx].ApicId);
    }

}
