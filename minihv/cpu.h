#ifndef _CPU_H
#define _CPU_H

#define NR_OF_CPUS  8

#include "minihv.h"
#include "tss.h"

#define BIT_MASK 0x01

#define GET_BITS(x, i, m) (((x) >> (i)) & (m))

#define MSR_IA32_FEATURE_CONTROL   0x3A

#define MSR_IA32_SYSENTER_ESP       0x175
#define MSR_IA32_SYSENTER_EIP       0x176

#ifndef CONTROL_BITS
#define CONTROL_BITS
#define MSR_IA32_VMX_BASIC         0x480
#define MSR_IA32_VMX_PINBASED_CTLS 0x481
#define MSR_IA32_VMX_PROCBASED_CTLS 0x482
#define MSR_IA32_VMX_EXIT_CTLS 0x483
#define MSR_IA32_VMX_ENTRY_CTLS 0x484
#define MSR_IA32_VMX_MISC       0x485
#define MSR_IA32_VMX_CR0_FIXED0    0x486
#define MSR_IA32_VMX_CR0_FIXED1    0x487
#define MSR_IA32_VMX_CR4_FIXED0    0x488
#define MSR_IA32_VMX_CR4_FIXED1    0x489
#define MSR_IA32_VMX_PROCBASED_CTLS2 0x48B
#define MSR_IA32_VMX_EPT_VPID_CAP 0x48C
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS 0x48D
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS 0x48E
#define MSR_IA32_VMX_TRUE_EXIT_CTLS 0x48F
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS 0x490
#endif


#define MSR_IA32_MTRRCAP 0xFE

#define MSR_IA32_MTRR_DEF_TYPE 0x2FF

#define MSR_IA32_EFER 0xC000'0080

#define MSR_IA32_FEATURE_CONTROL_LOCK_BIT 0

#define MSR_IA32_FEATURE_CONTROL_VMXON_IN_SMX 1
#define MSR_IA32_FEATURE_CONTROL_VMXON_OUTSIDE_SMX 2

#define MSR_ALLOWED1_SETTINGS_SHIFT 32

#define MSR_IA32_VMX_EPT_VPID_CAP_WB_BIT_INDEX 14

#define MSR_EFER_LMA_BIT_INDEX 10

#define CR0_PG ((QWORD)1 << 31)

#define WRITE_BACK_VALUE 6

#define RFLAGS_RESERVED1_BIT_INDEX 0x1
#define RFLAGS_IF_BIT_INDEX 0x9
#define RFLAGS_CF_BIT_INDEX 0x0

#define BSP_STACK_OFFSET 0xA000
#define BSP_TOS (KERNEL_VA_START + BSP_STACK_OFFSET)

#define VECTOR_15H 0x15

typedef union _LARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        INT32 HighPart;
    }Union;
    QWORD QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _CAP_REPORT
{
    BOOLEAN TrueMsrSupport;
    BOOLEAN VmExitReportsInsOuts;
    BOOLEAN EptPageWalkLengthFour;
    BOOLEAN EptPgStructeWriteBack;
    BOOLEAN EptPdeTwoMbPage;
    BOOLEAN EptPdpteOneGbPage;
    BOOLEAN VmExitAdvReport;
    BOOLEAN DirtyFlagAccess;
    BOOLEAN AccessRightsVmcsEnum;
}CAP_REPORT, *PCAP_REPORT;


typedef struct _CPU
{
    DWORD         ProcessorId;
    BYTE          ApicId;
    BOOLEAN       Bsp;
    TSS           Tss;

    PVOID         StackTop;
    DWORD         StackSize;

    WORD          TrSelector;

    PVOID         Vcpu;

    PVOID         VmxonRegionVa;
    PVOID         VmxonRegionPa;
    LARGE_INTEGER MsrData[17];
    
    CAP_REPORT    CapabReport;
}CPU, *PCPU;

VOID DumpCpuInfo(
    _In_ PCPU cpu
);

#define XCR_NUMBER_OF_SUPPORTED_VALUES 1

#define CPUID_MAX_PHYSICAL_ADDRESS_MASK 0xFF

#define REAL_MODE_IDT_BASE 0

#define INT_15H_E820 0xE820
#define INT_15H_E820_SIGNATURE 0x534D4150 // signature 'SMAP'

__forceinline
BYTE
GetCpuMaxPhysicalAddress(
    VOID
)
{
    int cpuInfo[4];
    BYTE maxPhysicalAddress;

    __cpuid(cpuInfo, 0x80000008);

    //
    // CPUID.80000008H:EAX[7:0] -> supported, then the maximum physical address number supported should come frome this field
    //

    maxPhysicalAddress = cpuInfo[0] & CPUID_MAX_PHYSICAL_ADDRESS_MASK;

    return maxPhysicalAddress;
}


__forceinline
BOOLEAN
XcrSupportedValue(
    QWORD Ecx
)
{
    static DWORD xcrSupportedValues[XCR_NUMBER_OF_SUPPORTED_VALUES] = { 0 };

    for (DWORD i = 0; i < XCR_NUMBER_OF_SUPPORTED_VALUES; i++)
    {
        if (Ecx == xcrSupportedValues[i])
        {
            return TRUE;
        }
    }

    return FALSE;
}

__forceinline
QWORD
CpuidGetFeature(
    DWORD EaxValue,
    BYTE FeatureInformationRegister,
    BYTE Index,
    BYTE Mask
)
{
    int cpuInfo[4];
    QWORD value;

    __cpuid(cpuInfo, EaxValue);

    value = (cpuInfo[FeatureInformationRegister] >> Index) & Mask;

    return value;
}

typedef enum
{
    RegisterRax,
    RegisterRcx,
    RegisterRdx,
    RegisterRbx,
    RegisterRsp,
    RegisterRbp,
    RegisterRsi,
    RegisterRdi,
    RegisterR8,
    RegisterR9,
    RegisterR10,
    RegisterR11,
    RegisterR12,
    RegisterR13,
    RegisterR14,
    RegisterR15
} GeneralPurposeRegisterIndexes;

typedef struct _CONTEXT
{
    QWORD GuestRsp;
    QWORD GuestRip;
    QWORD GuestRflags;
}CONTEXT, *PCONTEXT;


#pragma pack(push,8)
typedef struct _PROCESSOR_STATE
{
    QWORD       RegisterValues[RegisterR15 + 1];

    QWORD       Rip;

    QWORD       Rflags;

    DWORD       ExitReason;

    CONTEXT     GuestContext;

} PROCESSOR_STATE, *PPROCESSOR_STATE;
#pragma pack(pop)


typedef union _IA32_FEATURE_CONTROL_MSR
{
    QWORD All;
    struct
    {
        QWORD Lock : 1;                // [0]
        QWORD EnableSMX : 1;           // [1]
        QWORD EnableVmxon : 1;         // [2]
        QWORD Reserved2 : 5;           // [3-7]
        QWORD EnableLocalSENTER : 7;   // [8-14]
        QWORD EnableGlobalSENTER : 1;  // [15]
        QWORD Reserved3a : 16;         //
        QWORD Reserved3b : 32;         // [16-63]
    } Fields;
} IA32_FEATURE_CONTROL_MSR, *PIA32_FEATURE_CONTROL_MSR;

#endif // _CPU_H
