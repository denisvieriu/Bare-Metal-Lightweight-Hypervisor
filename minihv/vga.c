#include "vga.h"



extern BYTE x;
extern BYTE y;
extern PWORD video;


VOID VgaTextInit()
{
    VgaTextClear();
    VgaTextSetCursor(0);
}

VOID VgaTextClear()
{
    BYTE	attributeByte;
    WORD	attribute;
    WORD	idx;

    attributeByte = (0 << 4) | (15 & 0x0F);
    attribute = 0x20 | (attributeByte << 8); // 0x20 - space character

    for (idx = 0; idx < 80 * 25; idx++)
    {
        video[idx] = attribute;
    }

    x = 0;
    y = 0;
    VgaTextSetCursor(0);
}

VOID VgaTextSetCursor(WORD cursorLocation)
{
    __outbyte(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
    __outbyte(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
    __outbyte(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
    __outbyte(0x3D5, cursorLocation);      // Send the low cursor byte.
}
