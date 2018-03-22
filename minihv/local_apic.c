#include "local_apic.h"
#include "dbglog.h"
#include "sstring.h"
#include "gdt.h"
#include "minihv_status.h"

#define MS_IN_US 1000

static PDWORD m_lapicVa;

VOID
DummySleepUs(
    DWORD Us
)
{
    for (DWORD j = 0; j < Us; j++)
    {
        __inbyte(0x80);
    }
}

VOID
DummySleepMs(
    DWORD Ms
)
{
    DummySleepUs(Ms * MS_IN_US);
}

#pragma pack(push, 1)
// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

typedef union _ICR_LOW_REGISTER
{
    struct
    {
        DWORD           Vector : 8;
        DWORD           DeliveryMode : 3;
        DWORD           DestinationMode : 1;
        DWORD           DeliveryStatus : 1;
        DWORD           Reserved0 : 1;
        DWORD           Level : 1;
        DWORD           TriggerMode : 1;
        DWORD           Reserved1 : 2;
        DWORD           DestinationShorthand : 2;
        DWORD           Reserved2 : 12;
    };
    DWORD               Raw;
} ICR_LOW_REGISTER, *PICR_LOW_REGISTER;

typedef union _ICR_HIGH_REGISTER
{
    struct
    {
        DWORD           Reserved : 24;
        DWORD           Destination : 8;
    };
    DWORD               Raw;
} ICR_HIGH_REGISTER, *PICR_HIGH_REGISTER;
#pragma pack(pop)

typedef enum _LAPIC_DESTINATION_MODE
{
    ApicDestinationModePhysical,
    ApicDestinationModeLogical
} LAPIC_DESTINATION_MODE;

typedef enum _LAPIC_DESTINATION_SHORTHAND
{
    ApicDestinationShorthandNone,
    ApicDestinationShorthandSelf,
    ApicDestinationShorthandAll,
    ApicDestinationShorthandAllExcludingSelf
} LAPIC_DESTINATION_SHORTHAND;

typedef enum _APIC_DELIVERY_MODE
{
    ApicDeliveryModeFixed,
    ApicDeliveryModeLowest,
    ApicDeliveryModeSMI = 2,
    ApicDeliveryModeNMI = 4,
    ApicDeliveryModeINIT,
    ApicDeliveryModeSIPI,
    ApicDeliveryModeExtINT
} APIC_DELIVERY_MODE;

extern void tramp_end();
extern void tramp_start();
extern void switch_rip_2_va();
extern QWORD stack;
extern QWORD pml4TableAddress;
extern WORD apsTrSelector;
extern QWORD paToVa;


// CPUID with leaf 0x00
#define GET_ID_BSP(x)       x >> SHIFT_RIGHT_GET_ID // used to get BSP (cpuif with leaf 0x01)
#define GEN_INTEL           "GenuineIntel"
#define GEN_INTEL_LENGTH    14                      // Length of GenuineIntel
#define EBX_I               "Genu"
#define ECX_I               "ntel"
#define EDX_I               "inel"

// Local APIC Registers
#define LAPIC_ID                        0x0020  // Local APIC ID
#define LAPIC_VER                       0x0030  // Local APIC Version
#define LAPIC_TPR                       0x0080  // Task Priority
#define LAPIC_APR                       0x0090  // Arbitration Priority
#define LAPIC_PPR                       0x00a0  // Processor Priority
#define LAPIC_EOI                       0x00b0  // EOI
#define LAPIC_RRD                       0x00c0  // Remote Read
#define LAPIC_LDR                       0x00d0  // LOG_INFOical Destination
#define LAPIC_DFR                       0x00e0  // Destination Format
#define LAPIC_SVR                       0x00f0  // Spurious Interrupt Vector
#define LAPIC_ISR                       0x0100  // In-Service (8 registers)
#define LAPIC_TMR                       0x0180  // Trigger Mode (8 registers)
#define LAPIC_IRR                       0x0200  // Interrupt Request (8 registers)
#define LAPIC_ESR                       0x0280  // Error Status
#define LAPIC_ICRLO                     0x0300  // Interrupt Command
#define LAPIC_ICRHI                     0x0310  // Interrupt Command [63:32]
#define LAPIC_TIMER                     0x0320  // LVT Timer
#define LAPIC_THERMAL                   0x0330  // LVT Thermal Sensor
#define LAPIC_PERF                      0x0340  // LVT Performance Counter
#define LAPIC_LINT0                     0x0350  // LVT LINT0
#define LAPIC_LINT1                     0x0360  // LVT LINT1
#define LAPIC_ERROR                     0x0370  // LVT Error
#define LAPIC_TICR                      0x0380  // Initial Count (for Timer)
#define LAPIC_TCCR                      0x0390  // Current Count (for Timer)
#define LAPIC_TDCR                      0x03e0  // Divide Configuration (for Timer)

#define APIC_GLOBAL_ENABLE              (1 << 11)

QWORD
GetLapicBaseAdress(
    VOID
    )
{
    return __readmsr(IA32_APIC_BASE) & ~FOUR_LEVEL_PG_PAGE_OFFSET_MASK;
}

NTSTATUS
LapicInit(
    VOID
    )
{

    NTSTATUS lastNTStatus = STATUS_SUCCESS;
    LOG_INFO("Checking for GenuineIntel");

    lastNTStatus = CheckGenuineIntel();
    if (!NT_SUCCESS(lastNTStatus))
    {
        LOG_INFO("CheckGenuineIntel failed: [0x%08x]", lastNTStatus);
        return lastNTStatus;
    }


    volatile DWORD* pSvr;
    QWORD lapicAdress = GetLapicBaseAdress();
    QWORD enable = lapicAdress;
    LOG_INFO("Enabling local apic");
    LOG_INFO("Setting byte of IA32_APIC_BASE[11] to 1");

    m_lapicVa = (PDWORD)MmuMapToVa(lapicAdress, PAGE_SIZE, NULL, TRUE);

    enable = __readmsr(IA32_APIC_BASE);
    enable = enable | (APIC_GLOBAL_ENABLE);
    __writemsr(IA32_APIC_BASE, enable);

    pSvr = (PDWORD)((PBYTE)m_lapicVa + 0xF0);
    *pSvr |= 0x100;

    return lastNTStatus;
}

VOID
LapicSendIpi(
    _In_     BYTE ApicId,
    _In_     APIC_DELIVERY_MODE DeliveryMode,
    _In_opt_ PBYTE Vector
    )
{
    ICR_LOW_REGISTER icrLowRegister = { 0 };
    ICR_HIGH_REGISTER icrHighRegister = { 0 };

    volatile DWORD *pLapicIcrLow = (PDWORD)((PBYTE)m_lapicVa + 0x300);
    volatile DWORD *pLapicIcrHigh = (PDWORD)((PBYTE)m_lapicVa + 0x310);

    icrLowRegister.Level = 1;
    icrLowRegister.DestinationShorthand = ApicDestinationShorthandNone;
    icrLowRegister.DestinationMode = ApicDestinationModePhysical;
    icrLowRegister.DeliveryMode = DeliveryMode;

    if (Vector != NULL)
    {
        icrLowRegister.Vector = *Vector;
    }

    icrHighRegister.Destination = ApicId;

    pLapicIcrLow = &icrLowRegister.Raw;
    pLapicIcrHigh = &icrHighRegister.Raw;
}

VOID
LapicSignalAllAPs(
    _In_ PCPU    Cpu,
    _In_ BYTE    SipiVector
)
{
    PQWORD pStack = (PQWORD)(TRAMP_16_LOCATION + ((QWORD)&stack) - ((QWORD)tramp_start)); // points to the low memory PA which will store the address of the stack for the APs; every AP needs a different stack
    PWORD pApsTrSelector = (PWORD)(TRAMP_16_LOCATION + ((QWORD)&apsTrSelector) - ((QWORD)tramp_start)); // points to the low memory PA which will store the selector for a TSS; every AP needs a different TSS selector
    WORD currentTrSelector = TR_FIRST_SELECTOR;

    for (DWORD i = 0; i < GetNrOfCpus(); i++)
    {
        Cpu[i].TrSelector = currentTrSelector;

        if (Cpu[i].Bsp)
        {
            LOG_INFO("BSP will not be signaled");

            Cpu[i].StackTop = (PVOID)BSP_TOS;
            Cpu[i].StackSize = 2 * PAGE_SIZE;
        }
        else
        {
            LOG_INFO("Signaling AP %d with ApicId %d", Cpu[i].ProcessorId, Cpu[i].ApicId);

            *pStack = (QWORD)MmuAllocVa(CPU_STACK_SIZE);
            MmuUnmapFromVa(*pStack, PAGE_SIZE); // unmap last stack page for out-of-stack error detection
            *pStack += CPU_STACK_SIZE - 0x08; // set stack to higher value -> it will decrease
            Cpu[i].StackTop = (PVOID)*pStack;

            Cpu[i].StackSize = CPU_STACK_SIZE;

            *pApsTrSelector = currentTrSelector;

            LapicSendIpi(Cpu[i].ApicId, 5, NULL);
            DummySleepMs(10);

            LapicSendIpi(Cpu[i].ApicId, 6, &SipiVector);
            DummySleepUs(200);

            LapicSendIpi(Cpu[i].ApicId, 6, &SipiVector);
            DummySleepUs(200);
        }

        currentTrSelector += GDT_TSS_ENTRY_SIZE;
    }
}

VOID
LapicSetupTrampCode(
    _In_ CPU Cpus[8]
    )
{
    PVOID trampCodeVa;
    QWORD codeSize;

    LOG_INFO("Setting up tramp code");

    // address of the final paging tables
    pml4TableAddress = (QWORD)PML4_ADDRESS;

    // map APs tramp code
    trampCodeVa = MmuMapToVa(TRAMP_16_LOCATION, TRAMP_16_SIZE, NULL, FALSE);
    MmuMapToVaInternal(TRAMP_16_LOCATION, TRAMP_16_SIZE, TRAMP_16_LOCATION, FALSE);

    codeSize = (QWORD)tramp_end - (QWORD)tramp_start;

    // VA of the tramp code, as generated by the compiler
    paToVa = (QWORD)switch_rip_2_va;

    // setup final GDTs for APs
    GdtSetupFinalGdt(Cpus);

    //nonstandard extension, function/data pointer conversion in expression
#pragma warning(suppress:4152)
    CustomMemcpy(trampCodeVa, tramp_start, codeSize);

    MmuUnmapFromVa((QWORD)trampCodeVa, TRAMP_16_SIZE);
}



BYTE
GetInitialApicId(
    VOID
    )
{
    INT cpuInfo[DEF_CPU_INFO];

    //DebugLOG_INFOInfo("Calling __cpuid with leaf 0x01");
    __cpuid(cpuInfo, 0x01);

    //DebugLOG_INFOInfo("EAX: [0x%08x]", cpuInfo[EAX]);
    //DebugLOG_INFOInfo("EBX: [0x%08x]", cpuInfo[EBX]);
    //DebugLOG_INFOInfo("ECX: [0x%08x]", cpuInfo[ECX]);
    //DebugLOG_INFOInfo("EDX: [0x%08x]", cpuInfo[EDX]);

    return (BYTE)(cpuInfo[EBX] >> SHIFT_RIGHT_GET_ID);
}



VOID
GetVendor(
    _In_  PCHAR p,
    _Out_ PCHAR str
    )
{
    sstrncpy(str + 0, p + 0, CustomStrlen(EBX_I));
    sstrncpy(str + CustomStrlen(EBX_I), p + CustomStrlen(EBX_I) + CustomStrlen(ECX_I), CustomStrlen(EDX_I));
    sstrncpy(str + CustomStrlen(EBX_I) + CustomStrlen(ECX_I), p + CustomStrlen(EBX_I), CustomStrlen(ECX_I));
}

NTSTATUS
CheckGenuineIntel(
    VOID
    )
{
    CHAR    *p;
    INT     cpuInfo[DEF_CPU_INFO];
    CHAR    vendor[GEN_INTEL_LENGTH];

    CustomMemset(vendor, 0, sizeof(vendor));
    LOG_INFO("Calling __cpuid with leaf 0");
    __cpuid(cpuInfo, 0x00);

    p = (CHAR *)&cpuInfo[EBX];

    GetVendor(p, vendor);
    LOG_INFO(">>%s", vendor);

    LOG_INFO("Checking if vendor is \"GenuineIntel\"");
    if (!sMstrcmp(vendor, GEN_INTEL))
    {
        LOG_INFO("Vendor successfully verified");
        return STATUS_SUCCESS;
    }
    //LOG_ERROR("%d", sMstrcmp(vendor, GEN_INTEL));
    //LOG_ERROR("%s %d", vendor, CustomStrlen(vendor));
    //LOG_ERROR("%s %d", GEN_INTEL, CustomStrlen(GEN_INTEL));
    LOG_ERROR("Vendor not \"GenuineIntel\"");
    return STATUS_NOT_GENUINE_INTEL;
}

BOOLEAN
IsBsp(
    _In_ BYTE apic
    )
{
    return apic == GetInitialApicId();
}

//VOID
//SignalAP(
//    PCPU cpu
//)
//{
//    DWORD idx;
//    //// INIT - SIPI - SIPI
//    DWORD nrOfCpus = GetNrOfCpus();
//
//    for (idx = 0; idx < nrOfCpus; idx++)
//    {
//        //signalAP();
//        //
//    }
//}
