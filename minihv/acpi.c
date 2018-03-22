#include "print.h"
#include "acpica.h"
#include "dbglog.h"
#include "local_apic.h"
#include "acpi.h"
#include "ntstatus.h"


#define ACPI_MAX_INIT_TABLES (16)
static ACPI_TABLE_DESC      Tables[ACPI_MAX_INIT_TABLES];
DWORD                gNrOfCpus;

ACPI_STATUS
AcpiInit(
    VOID
)
{
    ACPI_STATUS result;


    LOG_ACPI("Calling AcpiInitializetables");
    result = AcpiInitializeTables(Tables, ACPI_MAX_INIT_TABLES, TRUE);
    if (result != AE_OK)
    {
        LOG_ERROR("AcpiInitializeTables() failed 0x%08x\n", result);
        return AE_ERROR;
    }

    LOG_INFO("AcpiInit succeeded");
    return (AE_OK);
}

DWORD GetNrOfCpus(
    VOID
    )
{
    return (gNrOfCpus) ? (gNrOfCpus) : 0;
}


ACPI_STATUS
AcpiGetProcessors(
    _Out_ PCPU cpu
)
{
    ACPI_STATUS             lastACPIStatus;
    ACPI_TABLE_HEADER       *pMadtTable;
    ACPI_MADT_LOCAL_APIC    *pLocalApic;
    ACPI_SUBTABLE_HEADER    *pSubtableHeader;
    DWORD                   lengthBytesOfTable;

    pMadtTable = NULL;

    LOG_ACPI("Calling AcpiGetTable...");
    lastACPIStatus = AcpiGetTable(ACPI_SIG_MADT, 1, &pMadtTable);

    if (ACPI_FAILURE(lastACPIStatus))
    {
        LOG_ACPI_ERROR("AcpiGetTable failed, status: 0x%08x", lastACPIStatus);
        return lastACPIStatus;
    }
    LOG_INFO("AcpiGetTable(ACPI_SIG_MDT) succeeded..");

    lengthBytesOfTable = sizeof(ACPI_TABLE_MADT);
    LOG_INFO("Length in bytes of a single MADT entry table: %d", lengthBytesOfTable);
    LOG_INFO("Total length in bytes of all MADT entries: %d",    pMadtTable->Length);

    LOG_INFO("Checking for CPUs");
    while (lengthBytesOfTable < pMadtTable->Length)
    {
        pSubtableHeader = (ACPI_SUBTABLE_HEADER *)((PBYTE)pMadtTable + lengthBytesOfTable);
        //LOG_INFO("Checking for LOCAL APIC...");
        if (pSubtableHeader->Type == ACPI_MADT_TYPE_LOCAL_APIC)
        {
            //LOG_ACPI("LOCAL APIC found");
            pLocalApic = (ACPI_MADT_LOCAL_APIC *)pSubtableHeader;

            //LOG_ACPI("Processor ID : 0x%08x", pLocalApic->ProcessorId);
            //LOG_ACPI("Apic ID      : 0x%08x", pLocalApic->Id);

            cpu[gNrOfCpus].ProcessorId = pLocalApic->ProcessorId;
            cpu[gNrOfCpus].ApicId = pLocalApic->Id;

            (IsBsp(pLocalApic->Id)) ? (cpu[gNrOfCpus].Bsp = TRUE) : (cpu[gNrOfCpus].Bsp = FALSE);

            gNrOfCpus++;
        }

        if (pSubtableHeader->Length == 0)
        {
            break;
        }

        lengthBytesOfTable += pSubtableHeader->Length;
    }
    return AE_OK;
}



ACPI_STATUS
AcpiGetPciEcam(
    PQWORD BaseAddress,
    PBYTE StartBusNumber,
    PBYTE EndBusNumber
)
{
    ACPI_STATUS             status;
    ACPI_TABLE_HEADER       *pAcpiTableMcfg;
    ACPI_MCFG_ALLOCATION    *pMcfgTable;
    DWORD                   offsetInTable = sizeof(ACPI_TABLE_MCFG);

    if (BaseAddress == NULL)
    {
        LOG("Invalid first parameter");
        return AE_ERROR;
    }

    if (StartBusNumber == NULL)
    {
        LOG("Invalid second parameter");
        return AE_ERROR;
    }

    if (EndBusNumber == NULL)
    {
        LOG("Invalid third parameter");
        return AE_ERROR;
    }

    status = AcpiGetTable(ACPI_SIG_MCFG, 1, &pAcpiTableMcfg);
    if (ACPI_FAILURE(status))
    {
        LOG("AcpiGetTable failed with %X", status);
        return (status);
    }

    pMcfgTable = (ACPI_MCFG_ALLOCATION *)((PBYTE)pAcpiTableMcfg + offsetInTable);

    LOG("Address %X", (QWORD)pMcfgTable->Address);
    LOG("PciSegment %X", (QWORD)pMcfgTable->PciSegment);
    LOG("StartBusNumber %X", (QWORD)pMcfgTable->StartBusNumber);
    LOG("EndBusNumber %X", (QWORD)pMcfgTable->EndBusNumber);

    *BaseAddress = pMcfgTable->Address;
    *StartBusNumber = pMcfgTable->StartBusNumber;
    *EndBusNumber = pMcfgTable->EndBusNumber;

    return AE_OK;
}
