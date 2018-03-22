#ifndef VGA_H
#define VGA_H

#include "minihv.h"

// Colors from 0x0 to 0xF
typedef enum _COLOR
{
    COL_BLACK,
    COL_BLUE,
    COL_GREEN,
    COL_CYAN,
    COL_RED,
    COL_MAGENTA,
    COL_BROWN,
    COL_LIGHT_GREY,
    COL_DARK_GREY,
    COL_LIGHT_BLUE,
    COL_LIGHT_GREEN,
    COL_LIGHT_CYAN,
    COL_LIGHT_RED,
    COL_LIGHT_MAGENTA,
    COL_LIGHT_BROWN,
    COL_WHITE
}COLOR;

// Text attributes
#define TEXT_ATTR(fgcolor, bgcolor) ((bgcolor << 12) | fgcolor << 8)


VOID VgaTextInit();
VOID VgaTextClear();
VOID VgaTextSetCursor(WORD cursorLocation);

#endif
