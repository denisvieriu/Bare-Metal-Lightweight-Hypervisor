#ifndef video_H
#define video_H

#include "minihv.h"
#include "vga.h"


// Prints / Writes a signle character to the sceen
BOOLEAN
monitorPut(
    CHAR c
    );

// Prints / Writes an entire NULL terminated string
BOOLEAN
monitorWrite(
    CONST CHAR *fmt,
    ...
    );

// Clears the screen by filling it up with spaces
VOID
monitorClear(
    VOID
    );

//
static VOID
scroll(
    VOID
    );

static VOID moveCursor();



#endif // !video_H
