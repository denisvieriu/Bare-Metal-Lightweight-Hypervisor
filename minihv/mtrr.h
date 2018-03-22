#pragma once

#include "minihv.h"
#include "map.h"

#define MTRR_FIXED_RANGE_LOW 0x00000
#define MTRR_FIXED_RANGE_HIGH 0xFFFFF

#define MTRR_FIXED_64K_RANGE_LOW 0x00000
#define MTRR_FIXED_64K_RANGE_HIGH 0x7FFFF

#define MTRR_FIXED_16K_RANGE_LOW 0x80000
#define MTRR_FIXED_16K_RANGE_HIGH 0xBFFFF

#define MTRR_FIXED_4K_RANGE_LOW 0xC0000
#define MTRR_FIXED_4K_RANGE_HIGH 0xFFFFF

BYTE
MtrrGetPageMemoryType(
    QWORD BaseAddress
);

VOID
MtrrGetCapabilities(
    BOOLEAN* MtrrSupported,
    PBYTE VariableRangeRegistersCount,
    BOOLEAN* FixedRangeRegistersSupported,
    PBYTE DefaultMemoryType
);

VOID
MtrrInit(
    VOID
);