
#pragma once

#include "minihv.h"

// ------------------------------------------------------------------------------------------------
// VGA Text Mode
#define TEXT_ROWS                       50
#define TEXT_COLS                       80

// Colors
#define TEXT_BLACK                      0
#define TEXT_BLUE                       1
#define TEXT_GREEN                      2
#define TEXT_CYAN                       3
#define TEXT_RED                        4
#define TEXT_MAGENTA                    5
#define TEXT_BROWN                      6
#define TEXT_LIGHT_GRAY                 7
#define TEXT_DARK_GRAY                  8
#define TEXT_LIGHT_BLUE                 9
#define TEXT_LIGHT_GREEN                10
#define TEXT_LIGHT_CYAN                 11
#define TEXT_LIGHT_RED                  12
#define TEXT_LIGHT_MAGENTA              13
#define TEXT_YELLOW                     14
#define TEXT_WHITE                      15

#pragma warning(disable : 4005)
#define TEXT_ATTR(fgcolor, bgcolor) \
    ((bgcolor << 12) | (fgcolor << 8))

#define DEFAULT_TEXT_ATTR               TEXT_ATTR(TEXT_LIGHT_GRAY,      TEXT_BLACK)
#define DEFAULT_ERROR_ATTR              TEXT_ATTR(TEXT_RED,             TEXT_BLACK)
#define DEFAULT_WARNING_ATR             TEXT_ATTR(TEXT_YELLOW,          TEXT_BLACK)
#define DEFAULT_ACPI_ATTR               TEXT_ATTR(TEXT_DARK_GRAY,       TEXT_BLACK)

#define DEFAULT_MINIHV_ATTR1            TEXT_ATTR(TEXT_WHITE,           TEXT_RED)
#define DEFAULT_MINIHV_ATTR2            TEXT_ATTR(TEXT_LIGHT_RED,       TEXT_WHITE)

#define DEFAULT_MINIHV_LOADED           TEXT_ATTR(TEXT_WHITE,          TEXT_LIGHT_GREEN)
#define DEFAULT_MINIHV_LOADED_DARK      DEFAULT_TEXT_ATTR

VOID
VVgaTextInit(
    VOID
    );

VOID
VVgaTextClear(
    VOID
    );

VOID
VVgaTextSetCursor(
    _In_ UINT offset
);
