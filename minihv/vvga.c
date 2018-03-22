#include "vvga.h"
#include "lowmem.h"

VOID
VVgaTextInit(
    VOID
    )
{
    VVgaTextClear();
    VVgaTextSetCursor(0);
}

VOID
VVgaTextClear(
    VOID
    )
{

    BYTE	attributeByte;
    WORD	attribute;
    WORD	idx;

    attributeByte = (0 << 4) | (15 & 0x0F);
    attribute = 0x20 | (attributeByte << 8); // 0x20 - space character
    for (idx = 0; idx < 80 * 25; idx++)
    {
        VGA_TEXT_BASE[idx] = DEFAULT_TEXT_ATTR;
    }

    VVgaTextSetCursor(0);
}

VOID
VVgaTextSetCursor(
    UINT offset
    )
{
    __outbyte(0x3d4, 0x0e);
    __outbyte(0x3d5, offset >> 8);
    __outbyte(0x3d4, 0x0f);
    __outbyte(0x3d5, offset);
}
