#include "video_memory_print.h"
#include "acpica.h"

volatile PWORD video = (PWORD)0xB8000;

// starting cursor position
volatile BYTE x = 0;
volatile BYTE y = 0;

BYTE cursor = 0;



BOOLEAN
monitorPut(
    CHAR chr
)
{
	BYTE	bColour;		 // background color
	BYTE	attributeByte;
	WORD	attribute;
	volatile PWORD  location;

	bColour = COL_BLACK;   // black background

	attributeByte = (bColour << 4) | (COL_WHITE);  // lower bytes : foreground
												        // upper bytes : background
	attribute = attributeByte << 8;

    if (chr == 0x08 && x)
    {
        x--;
    }

    // Handle a tab by increasing the cursor's X, but only to a point
    // where it is divisible by 8.
    else if (chr == 0x09)
    {
        x = (x + 8) & ~(8 - 1);
    }

    // Handle carriage return
    else if (chr == '\r')
    {
        x = 0;
    }

    // Handle newline by moving cursor back to left and increasing the row
    else if (chr == '\n')
    {
        x = 0;
        y++;
    }
    // Handle any other printable character.
    else if (chr >= ' ')
    {
        location = video + (y * 80 + x);
        *location = chr | attribute;
        x++;
    }
	if (x >= 80)
	{
		x = 0;
		y++;
	}
	scroll();

    moveCursor();
	return TRUE;
}

VOID
monitorClear(
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
		video[idx] = attribute;
	}

	x = 0;
	y = 0;

    moveCursor();
}

static VOID
scroll(
    VOID
)
{

	// Get a space character with the default colour attributes.
	BYTE attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
	WORD blank = 0x20 /* space */ | (attributeByte << 8);

	// Row 25 is the end, this means we need to scroll up
	if (y >= 25)
	{
		// Move the current text chunk that makes up the screen
		// back in the buffer by a line
		int i;
		for (i = 0 * 80; i < 24 * 80; i++)
		{
			video[i] = video[i + 80];
		}

		// The last line should now be blank. Do this by writing
		// 80 spaces to it.
		for (i = 24 * 80; i < 25 * 80; i++)
		{
			video[i] = blank;
		}
		// The cursor should now be on the last line.
		y = 24;
	}
}



BOOLEAN
monitorWrite(
    CONST CHAR *fmt,
    ...
)

{
    char buf[1024];
    va_list args;

    va_start(args, fmt);
    CustomSnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    char c;
    char *s = buf;
    while ((c = *s++))
    {
        monitorPut(c);
    }

    return TRUE;
}

static VOID moveCursor()
{
    // The screen is 80 characters wide...
    WORD cursorLocation = y * 80 + x;
    __outbyte(0x3D4, 14);                // Tell the VGA board we are setting the high cursor byte.
    __outbyte(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
    __outbyte(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
    __outbyte(0x3D5, cursorLocation);      // Send the low cursor byte.
}