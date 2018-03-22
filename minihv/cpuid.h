#ifndef _CPUID_H
#define _CPUID_H

#include "minihv.h"

#define IS_BIT_SET(nr, n) ((nr & (1ULL << n)) ? (TRUE) : (FALSE))

#define SUPPORTED       " supported"
#define UNSUPPORTED     " unsupported"

typedef struct _CPUID
{
    INT CpuidEax;
    INT CpuidEbc;
    INT CpuidEcx;
    INT CpuidEdx;
}CPUID, *PCPUID;

typedef enum _ECX_REGISTER_CPUID0x01
{
    SSE3,
    PCLMULQDQ,
    DTES64,
    MONITOR,
    DS_CPL,
    VMX,
    SMX,
    EIST,
    TM2,
    SSSE3,
    CNXT_ID,
    SDBG,
    FMA,
    CMPXCHG16B,
    xTPR_Update_Control,
    PDCM,
    RESERVED,
    PCID,
    DCA,
    SSE4_1,
    SSE4_2,
    x2APCIC,
    MOVBE,
    POPCNT,
    TSC_Deadline,
    AESNI,
    XSAVE,
    OSXSAVE,
    AVX,
    F16C,
    RDRAND,
    NOT_USED
}_ECX_REGISTER_CPUID0x01;

BOOLEAN
Check_ECX(
    _In_ DWORD ECX,
    _In_ BYTE  bit
);

#endif // _CPUID_H
