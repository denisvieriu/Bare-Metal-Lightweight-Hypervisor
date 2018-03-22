#include "mtrr.h"
#include "map.h"
#include "cpu.h"
#include "vmx.h"
#include "print.h"

#define CPUID_FEATRUE_INFORMATION 0x01
#define CPUID_FEATURE_INFORMATION_MTRR_SUPPORTED 0x12

#define CPUID_EDX 0x03

#define MSR_IA32_MTRR_DEF_TYPE_E_FLAG_INDEX 0x0B
#define MSR_IA32_MTRR_DEF_TYPE_TYPE_FIELD_INDEX 0x00

#define VCNT_INDEX 0
#define FIX_INDEX 8
#define FE_INDEX 10

static BYTE m_mtrrDefaultMemoryType;
static BYTE m_mtrrVcnt;

VOID
MtrrInit(
    VOID
)
{
    MtrrGetCapabilities(NULL, &m_mtrrVcnt, NULL, &m_mtrrDefaultMemoryType);
}

VOID
MtrrGetCapabilities(
    BOOLEAN* MtrrSupported,
    PBYTE VariableRangeRegistersCount,
    BOOLEAN* FixedRangeRegistersSupported,
    PBYTE DefaultMemoryType
)
{
    if (MtrrSupported != NULL)
    {
        *MtrrSupported = TRUE;
    }

    if (!CpuidGetFeature(CPUID_FEATRUE_INFORMATION, CPUID_EDX, CPUID_FEATURE_INFORMATION_MTRR_SUPPORTED, BIT_MASK))
    {
        if (MtrrSupported != NULL)
        {
            *MtrrSupported = FALSE;
        }
        goto get_variable_range_registers;
    }

    if (!ReadMsr(MSR_IA32_MTRR_DEF_TYPE, MSR_IA32_MTRR_DEF_TYPE_E_FLAG_INDEX, BIT_MASK))
    {
        if (MtrrSupported != NULL)
        {
            *MtrrSupported = FALSE;
        }
    }

get_variable_range_registers:

    *VariableRangeRegistersCount = (BYTE)ReadMsr(MSR_IA32_MTRRCAP, VCNT_INDEX, MAX_BYTE);

    if (FixedRangeRegistersSupported != NULL)
    {
        *FixedRangeRegistersSupported = TRUE;
    }

    if (!ReadMsr(MSR_IA32_MTRRCAP, FIX_INDEX, BIT_MASK))
    {
        if (FixedRangeRegistersSupported != NULL)
        {
            *FixedRangeRegistersSupported = FALSE;
        }
        goto get_default_mem_type;
    }

    if (!ReadMsr(MSR_IA32_MTRR_DEF_TYPE, FE_INDEX, BIT_MASK))
    {
        if (FixedRangeRegistersSupported != NULL)
        {
            *FixedRangeRegistersSupported = FALSE;
        }
    }

get_default_mem_type:

    *DefaultMemoryType = (BYTE)ReadMsr(MSR_IA32_MTRR_DEF_TYPE, MSR_IA32_MTRR_DEF_TYPE_TYPE_FIELD_INDEX, MAX_BYTE);
}

#define MTRR_64K 0x10000
#define MTRR_16K 0x4000

#define MSR_IA32_MTRR_FIX64K_00000 0x250
#define MSR_IA32_MTRR_FIX16K_80000 0x258
#define MSR_IA32_MTRR_FIX16K_A0000 0x259
#define MSR_IA32_MTRR_FIX4K_C0000  0x268
#define MSR_IA32_MTRR_FIX4K_C8000  0x269
#define MSR_IA32_MTRR_FIX4K_D0000  0x26A
#define MSR_IA32_MTRR_FIX4K_D8000  0x26B
#define MSR_IA32_MTRR_FIX4K_E0000  0x26C
#define MSR_IA32_MTRR_FIX4K_E8000  0x26D
#define MSR_IA32_MTRR_FIX4K_F0000  0x26E
#define MSR_IA32_MTRR_FIX4K_F8000  0x26F

static BYTE m_mtrrFixedRangeShift[8] = { 0, 8, 16, 24, 32, 40, 48, 56 };

typedef enum _MEMORY_TYPE
{
    UC,
    WC,
    WT = 0x4,
    WP,
    WB
} MEMORY_TYPE, *PMEMORY_TYPE;

char *
MemoryTypeValueToString(
    DWORD Value
)
{
    switch (Value)
    {
    case UC: return "UC";
    case WC: return "WC";
    case WT: return "WT";
    case WP: return "WP";
    case WB: return "WB";
    default: return "invalid type";
    }
}

static
BYTE
MtrrGetFixed64KPageMemoryType(
    QWORD BaseAddress
)
{
    BYTE fixedMsrValue;
    DWORD shiftIndex = (BaseAddress >> 16) & MAX_NIBBLE;

    fixedMsrValue = (BYTE)ReadMsr(MSR_IA32_MTRR_FIX64K_00000, m_mtrrFixedRangeShift[shiftIndex], MAX_NIBBLE);

    //LOG("Found fixed 64K mtrr for address %X of type %s", BaseAddress, MemoryTypeValueToString(fixedMsrValue));

    return fixedMsrValue;
}

#define MTRR_16K_MSR_INDEX_SHIFT 0x10

static DWORD m_mtrrFixed16KRangeMsrMap[4] =
{
    MSR_IA32_MTRR_FIX16K_80000, MSR_IA32_MTRR_FIX16K_80000, MSR_IA32_MTRR_FIX16K_A0000, MSR_IA32_MTRR_FIX16K_A0000
};

static
BYTE
MtrrGetFixed16KPageMemoryType(
    QWORD BaseAddress
)
{
    BYTE fixedMsrValue;
    BYTE msrIndex = (BYTE)(((BaseAddress & ~MAX_WORD) >> MTRR_16K_MSR_INDEX_SHIFT) - 0x8);
    BYTE shiftIndex = ((((BaseAddress >> 12) & MAX_NIBBLE) >> 2) + 4 * ((BaseAddress >> 16) & 1));

    fixedMsrValue = (BYTE)ReadMsr(m_mtrrFixed16KRangeMsrMap[msrIndex], m_mtrrFixedRangeShift[shiftIndex], MAX_NIBBLE);

    //LOG("Found fixed 16K mtrr for address %X of type %s", BaseAddress, MemoryTypeValueToString(fixedMsrValue));

    return fixedMsrValue;
}

static DWORD m_mtrrFixed4KRangeMsrMap[8] =
{
    MSR_IA32_MTRR_FIX4K_C0000, MSR_IA32_MTRR_FIX4K_C8000, MSR_IA32_MTRR_FIX4K_D0000, MSR_IA32_MTRR_FIX4K_D8000,
    MSR_IA32_MTRR_FIX4K_E0000, MSR_IA32_MTRR_FIX4K_E8000, MSR_IA32_MTRR_FIX4K_F0000, MSR_IA32_MTRR_FIX4K_F8000
};

static
BYTE
MtrrGetFixed4KPageMemoryType(
    QWORD BaseAddress
)
{
    BYTE fixedMsrValue;
    BYTE msrIndex = (BYTE)(2 * (((BaseAddress & ~MAX_WORD) - MTRR_FIXED_4K_RANGE_LOW) >> MTRR_16K_MSR_INDEX_SHIFT) + (((BaseAddress >> 12) & MAX_NIBBLE) >> 3));
    DWORD shiftIndex = (((BaseAddress >> 12) & MAX_NIBBLE) & 7);

    fixedMsrValue = (BYTE)ReadMsr(m_mtrrFixed4KRangeMsrMap[msrIndex], m_mtrrFixedRangeShift[shiftIndex], MAX_NIBBLE);

    //LOG("Found fixed 4K mtrr for address %X of type %s", BaseAddress, MemoryTypeValueToString(fixedMsrValue));

    return fixedMsrValue;
}

#define IN_RANGE(x, s, bl, bh) (((x) >= (bl)) && (((x) + (s) - 1) <= bh))

#define MSR_MTRR_VARIABLE_RANGES_VALID_BIT_INDEX 11
#define MSR_MTRR_VARIABLE_RANGES_MASK_INDEX 12
#define MSR_MTRR_VARIABLE_RANGES_BASE_INDEX 12
#define MSR_MTRR_VARIABLE_RANGES_TYPE_INDEX 0

#define MSR_IA32_MTRR_PHYSBASE0 0x200
#define MSR_IA32_MTRR_PHYSMASK0 0x201
#define MSR_IA32_MTRR_PHYSBASE1 0x202
#define MSR_IA32_MTRR_PHYSMASK1 0x203
#define MSR_IA32_MTRR_PHYSBASE2 0x204
#define MSR_IA32_MTRR_PHYSMASK2 0x205
#define MSR_IA32_MTRR_PHYSBASE3 0x206
#define MSR_IA32_MTRR_PHYSMASK3 0x207
#define MSR_IA32_MTRR_PHYSBASE4 0x208
#define MSR_IA32_MTRR_PHYSMASK4 0x209
#define MSR_IA32_MTRR_PHYSBASE5 0x20A
#define MSR_IA32_MTRR_PHYSMASK5 0x20B
#define MSR_IA32_MTRR_PHYSBASE6 0x20C
#define MSR_IA32_MTRR_PHYSMASK6 0x20D
#define MSR_IA32_MTRR_PHYSBASE7 0x20E
#define MSR_IA32_MTRR_PHYSMASK7 0x20F
#define MSR_IA32_MTRR_PHYSBASE8 0x210
#define MSR_IA32_MTRR_PHYSMASK8 0x211
#define MSR_IA32_MTRR_PHYSBASE9 0x212
#define MSR_IA32_MTRR_PHYSMASK9 0x213

static WORD m_mtrrVariableRangesMsrs[] =
{
    // 8 variable ranges
    /*
    Register IA32_MTRRCAP is MSR[FE]
        There are 8 variable ranges, split into a base and mask:
       -> IA32_MTRR_PHYSBASE{x=0..7}, in MSR[200+x*2]
       -> IA32_MTRR_PHYSMASK{x=0..7}, in MSR[201+x*2]
        There are 88 fixed ranges, covering the bottom 1MB
        IA32_MTRR_FIX64K_00000, in MSR[250] - 8 regions of 64 KB each (512 KB total)
        IA32_MTRR_FIX16K_80000, in MSR[258] - 8 regions of 16 KB each (128 KB total)
        IA32_MTRR_FIX16K_A0000, in MSR[259] - 8 regions of 16 KB each (128 KB total)
        IA32_MTRR_FIX4K_C0000, in MSR[268] - 8 regions of 4 KB each (32 KB total)
        IA32_MTRR_FIX4K_C8000, in MSR[269] - 8 regions of 4 KB each (32 KB total)
        IA32_MTRR_FIX4K_D0000, in MSR[26A] - 8 regions of 4 KB each (32 KB total)
        IA32_MTRR_FIX4K_D8000, in MSR[26B] - 8 regions of 4 KB each (32 KB total)
        IA32_MTRR_FIX4K_E0000, in MSR[26C] - 8 regions of 4 KB each (32 KB total)
        IA32_MTRR_FIX4K_E8000, in MSR[26D] - 8 regions of 4 KB each (32 KB total)
        IA32_MTRR_FIX4K_F0000, in MSR[26E] - 8 regions of 4 KB each (32 KB total)
        IA32_MTRR_FIX4K_F8000, in MSR[26F] - 8 regions of 4 KB each (32 KB total)
    */
    MSR_IA32_MTRR_PHYSBASE0, MSR_IA32_MTRR_PHYSMASK0,
    MSR_IA32_MTRR_PHYSBASE1, MSR_IA32_MTRR_PHYSMASK1,
    MSR_IA32_MTRR_PHYSBASE2, MSR_IA32_MTRR_PHYSMASK2,
    MSR_IA32_MTRR_PHYSBASE3, MSR_IA32_MTRR_PHYSMASK3,
    MSR_IA32_MTRR_PHYSBASE4, MSR_IA32_MTRR_PHYSMASK4,
    MSR_IA32_MTRR_PHYSBASE5, MSR_IA32_MTRR_PHYSMASK5,
    MSR_IA32_MTRR_PHYSBASE6, MSR_IA32_MTRR_PHYSMASK6,
    MSR_IA32_MTRR_PHYSBASE7, MSR_IA32_MTRR_PHYSMASK7,
    MSR_IA32_MTRR_PHYSBASE8, MSR_IA32_MTRR_PHYSMASK8,
    MSR_IA32_MTRR_PHYSBASE9, MSR_IA32_MTRR_PHYSMASK9
};

BYTE
MtrrGetPageMemoryType(
    QWORD BaseAddress
)
{
    if (IN_RANGE(BaseAddress, PAGE_SIZE, MTRR_FIXED_RANGE_LOW, MTRR_FIXED_RANGE_HIGH))
    {
        if (IN_RANGE(BaseAddress, PAGE_SIZE, MTRR_FIXED_64K_RANGE_LOW, MTRR_FIXED_64K_RANGE_HIGH))
        {
            return MtrrGetFixed64KPageMemoryType(BaseAddress);
        }
        else if (IN_RANGE(BaseAddress, PAGE_SIZE, MTRR_FIXED_16K_RANGE_LOW, MTRR_FIXED_16K_RANGE_HIGH))
        {
            return MtrrGetFixed16KPageMemoryType(BaseAddress);
        }
        else
        {
            return MtrrGetFixed4KPageMemoryType(BaseAddress);
        }
    }
    else
    {
        for (BYTE i = 0; i < m_mtrrVcnt; i++)
        {
            QWORD physAddrMsrValue = ReadMsr(m_mtrrVariableRangesMsrs[2 * i], 0, MAX_QWORD);
            QWORD physMaskMsrValue = ReadMsr(m_mtrrVariableRangesMsrs[2 * i + 1], 0, MAX_QWORD);
            BYTE mtrrValue;

            if (!GET_BITS(physMaskMsrValue, MSR_MTRR_VARIABLE_RANGES_VALID_BIT_INDEX, 1))
            {
                continue;
            }

            QWORD physBase = GET_BITS(physAddrMsrValue, MSR_MTRR_VARIABLE_RANGES_BASE_INDEX, 0xFFFFFFF) << 12;
            QWORD physMask = GET_BITS(physMaskMsrValue, MSR_MTRR_VARIABLE_RANGES_MASK_INDEX, 0xFFFFFFF) << 12;

            if ((BaseAddress & physMask) == (physBase & physMask))
            {
                mtrrValue = (BYTE)GET_BITS(physAddrMsrValue, MSR_MTRR_VARIABLE_RANGES_TYPE_INDEX, MAX_BYTE);

                //LOG("Found variable range mtrr for address %X of type %s", BaseAddress, MemoryTypeValueToString(mtrrValue));

                return mtrrValue;
            }
        }
    }

    //LOG("Returning default type %s for address %x", MemoryTypeValueToString(m_mtrrDefaultMemoryType), BaseAddress);

    return m_mtrrDefaultMemoryType;
}
