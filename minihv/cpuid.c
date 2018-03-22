#include "cpuid.h"
#include "dbglog.h"

#define LOG_ECX(isBitSet, print) { if (isBitSet)                          \
                                        { LOG_INFO(print SUPPORTED);  }   \
                                     else                                   \
                                        { LOG_INFO(print UNSUPPORTED);}   \
                                     break;                                 \
                                  }
BOOLEAN
Check_ECX(
    _In_ DWORD ECX,
    _In_ BYTE  bit
    )
{
    BOOLEAN bitSet;

    bitSet = IS_BIT_SET(ECX, bit);
    switch (bit)
    {
    case SSE3:                  LOG_ECX(bitSet, "SSE3");
    case PCLMULQDQ:             LOG_ECX(bitSet, "PCLMULQDQ");
    case DTES64:                LOG_ECX(bitSet, "DTES64");
    case MONITOR:               LOG_ECX(bitSet, "MONITOR");
    case DS_CPL:                LOG_ECX(bitSet, "DS_CPL");
    case VMX:                   LOG_ECX(bitSet, "VMX");
    case SMX:                   LOG_ECX(bitSet, "SMX");
    case EIST:                  LOG_ECX(bitSet, "EIST");
    case TM2:                   LOG_ECX(bitSet, "TM2");
    case SSSE3:                 LOG_ECX(bitSet, "SSSE3");
    case CNXT_ID:               LOG_ECX(bitSet, "CNXT_ID");
    case SDBG:                  LOG_ECX(bitSet, "SDBG");
    case FMA:                   LOG_ECX(bitSet, "FMA");
    case CMPXCHG16B:            LOG_ECX(bitSet, "CMPXCHG16B");
    case xTPR_Update_Control:   LOG_ECX(bitSet, "xTPR_Update_Control");
    case PDCM:                  LOG_ECX(bitSet, "PDCM");
    case RESERVED:              LOG_ECX(bitSet, "RESERVED BIT");
    case PCID:                  LOG_ECX(bitSet, "PCID");
    case DCA:                   LOG_ECX(bitSet, "DCA");
    case SSE4_1:                LOG_ECX(bitSet, "SSE4_1");
    case SSE4_2:                LOG_ECX(bitSet, "SSE4_2");
    case x2APCIC:               LOG_ECX(bitSet, "x2APCIC");
    default:
        break;
    }

    return bitSet;
}
