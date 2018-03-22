// ------------------------------------------------------------------------------------------------
// console/console.c
// ------------------------------------------------------------------------------------------------

#include "cconsole.h"
#include "cmd.h"
#include "vvga.h"
#include "lowmem.h"
#include "stdarg.h"
#include "sstring.h"
#include "crt.h"
#include "error.h"
#include "format.h"

#define     BLINK_BIT_POS           15
#define     BLINK_BIT               (1 << BLINK_BIT_POS)
#define     MAKE_IT_BLINK(atr)      (BLINK_BIT) | (atr)
#define     LOAD_BAR                77
#define     LOADING                 "LOADING"
#define     LOADING_START           71
#define     MINI_HV_BAR             "MINI HYPERVISOR"
#define     MAX_LENGTH_HV_BAR       CustomStrlen(MINI_HV_BAR)
#define     OFFSET_HV_BAR           (DWORD)((TEXT_COLS / 2) - ((MAX_LENGTH_HV_BAR - 1) / 2))
#define     MAX_INCREMENT_VALUE     100

static UINT s_col;
static UINT s_row;

static UINT s_cursor;
static char s_inputLine[TEXT_COLS];
static UINT s_lineIndex;

BOOLEAN gfirstRun            = TRUE;
BOOLEAN gBlinkOption         = FALSE;
volatile WORD gDefaultColour = DEFAULT_TEXT_ATTR;

BOOLEAN gWriteLoadBar   = FALSE;
BOOLEAN gFirstWrite     = TRUE;
DWORD   gOffsetLoadBar  = 0;
DWORD   gLoadBar        = 0;

BOOLEAN gWriteMiniHv          = FALSE;
DWORD   gOffsetMiniHvBar      = 0;
DWORD   gIncrementedMiniHvBar = 0;

BOOLEAN gFirstLoad = TRUE;

VOID WriteMiniHv(
    VOID
);

VOID
ConsoleInit(
    VOID
    )
{
    WriteMiniHv();
}


BOOLEAN SetDefaultCol(
    _In_ volatile WORD col
)
{
    switch (col)
    {
    case VGA_DEFAULT:  gDefaultColour = DEFAULT_TEXT_ATTR;      break;

    case VGA_WARNING:  gDefaultColour = DEFAULT_WARNING_ATR;    break;

    case VGA_ERROR:    gDefaultColour = DEFAULT_ERROR_ATTR;     break;

    case VGA_ACPI:      gDefaultColour = DEFAULT_ACPI_ATTR;     break;

    default:            gDefaultColour = DEFAULT_TEXT_ATTR;     break;
    }

    return TRUE;
}

BOOLEAN
ConsolePutChar(
    _In_ CHAR c
    )
{
    if (c == '\n')
    {
        // Advance to next line
        s_col = 0;
        ++s_row;
    }

    UINT offset = s_row * TEXT_COLS + s_col;
    if (offset >= (TEXT_ROWS - 1) * TEXT_COLS)
    {
        // Scroll text
        UINT i = 0;
        for (; i < (TEXT_ROWS - 2) * TEXT_COLS; i++)
        {
            VGA_TEXT_BASE[80 + i] = VGA_TEXT_BASE[TEXT_COLS * 2 + i];
        }

        for (; i < (TEXT_ROWS - 1) * TEXT_COLS; i++)
        {
            VGA_TEXT_BASE[i] = gDefaultColour | ' ';
        }

        --s_row;
        offset -= TEXT_COLS;
    }

    if (gWriteLoadBar)
    {
        VGA_TEXT_BASE[gOffsetLoadBar] = DEFAULT_MINIHV_ATTR1 | c;
        return TRUE;
    }

    if (gWriteMiniHv && gfirstRun)
    {
        VGA_TEXT_BASE[gOffsetMiniHvBar] = DEFAULT_MINIHV_ATTR1 | c;
        return TRUE;
    }
    else if (gWriteMiniHv)
    {
        if ((gOffsetMiniHvBar < OFFSET_HV_BAR) || (gOffsetMiniHvBar >= OFFSET_HV_BAR + MAX_LENGTH_HV_BAR))
        {
            VGA_TEXT_BASE[gOffsetMiniHvBar] = DEFAULT_MINIHV_LOADED | c;
        }
        else
        {
            VGA_TEXT_BASE[gOffsetMiniHvBar] = DEFAULT_MINIHV_LOADED_DARK | c;
        }
        return TRUE;
    }

    if (c == 0x09)
    {
        s_col = (s_col + 8) & ~(8 - 1);
    }
    else if (s_col < TEXT_COLS && c >= 32 && c <= 126)
    {
        // Print character
        if (s_row > 0)
        {
            VGA_TEXT_BASE[offset] = gDefaultColour | c;
        }

        ++s_col;
    }


    if (s_col >= 80)
    {
        s_col = 0;
        s_row++;
    }

    VVgaTextSetCursor(offset);
    return TRUE;
}

VOID
WriteCurrentFileDbg(
    _In_ CONST CHAR *Fmt,
    _In_opt_   ...
    )
{
    va_list     arg;
    CHAR        buff[MAX_PATH];
    size_t      length;

    va_start(arg, Fmt);
    length = vsnprintf(buff, MAX_PATH, Fmt, arg);
    va_end(arg);

    if (length == -1)
    {
        //
        // Clearly an error (don't halt it just for that)
        //
    }

}   

VOID
ApplyKeywordSettings(
    BOOLEAN Param
)
{
    //gEnabled;
}

VOID
SetToWriteLoadBar(
    _In_ BOOLEAN SetWrite,
    _In_ WRITES  Opt
    )
{
    switch (Opt)
    {
        case SET_LOAD_BAR:
        {
            (SetWrite) ? (gWriteLoadBar = TRUE) : (gWriteLoadBar = FALSE);
            break;
        }
        case SET_MINIHV_BAR:
        {
            (SetWrite) ? (gWriteMiniHv = TRUE) : (gWriteMiniHv = FALSE);
            break;
        }
        default:
        {
            break;
        }
    }
}

BOOLEAN
IncrementLoaded(
    _In_ DWORD  Value,       //
    _In_ WRITES Opt          //
    )
{
    /*if (VGA_LOG)
    {
        switch (Opt)
        {
        case SET_LOAD_BAR:
        {
            if (gFirstLoad)
            {
                CConsolePrint(LOADING);
                gFirstLoad = FALSE;
            }


            CHAR str[MAX_INCREMENT_VALUE];
            if (gLoadBar + Value <= MAX_INCREMENT_VALUE)
            {
                gLoadBar += Value;
            }
            CustomItoa(gLoadBar, str, 10);
            CustomStrcat(str, "%%");
            SetToWriteLoadBar(TRUE, SET_LOAD_BAR);
            CConsolePrint(str);
            SetToWriteLoadBar(FALSE, SET_LOAD_BAR);
            return TRUE;
        }
        case SET_MINIHV_BAR:
        {
            DWORD idx;
            CHAR  chr;
            DWORD offsetCopy;

            if (gIncrementedMiniHvBar + Value < TEXT_COLS)
            {
                if (gfirstRun)
                {
                    gOffsetMiniHvBar = 0;
                    gfirstRun = FALSE;
                }
                offsetCopy = gOffsetMiniHvBar;
                SetToWriteLoadBar(TRUE, SET_MINIHV_BAR);
                for (idx = offsetCopy; idx < Value + offsetCopy; idx++)
                {
                    if (idx < OFFSET_HV_BAR || idx >= OFFSET_HV_BAR + MAX_LENGTH_HV_BAR)
                    {
                        chr = ' ';
                    }
                    else
                    {
                        chr = MINI_HV_BAR[idx - OFFSET_HV_BAR];
                    }
                    ConsolePutChar(chr);
                    gOffsetMiniHvBar++;
                }
                SetToWriteLoadBar(FALSE, SET_MINIHV_BAR);

                gIncrementedMiniHvBar += Value;
            }
            return TRUE;
        }
        default:
            return FALSE;
        }
    }
    */
    return FALSE;
}

DWORD
RandRdtsc(
    _In_ DWORD minV,
    _In_ DWORD maxV
)
{
    return 0;
}


VOID
WriteBlinkingText(
    _In_        CHAR toPrint,
    _In_        DWORD nrTimes,
    _In_opt_    DWORD sleepTime,
    _In_opt_    BOOLEAN blinkActive,
    _In_opt_    BOOLEAN difCol
)
{
    volatile WORD copyDefaultColor;

    copyDefaultColor = gDefaultColour;
    gBlinkOption     = TRUE;

    if (blinkActive)
    {
        gDefaultColour = MAKE_IT_BLINK(gDefaultColour);
    }

    if (sleepTime == 0)
    {
        sleepTime = DEFAULT_SLEEP_BLINK;
    }

    for (DWORD i = 0; i < nrTimes; i++)
    {
        ConsolePutChar(toPrint);
        //IncrementLoaded(1, SET_MINIHV_BAR);
        Sleep(sleepTime);
    }

    gDefaultColour = copyDefaultColor;
    gBlinkOption = FALSE;
}

BOOLEAN
CConsolePrint(
    CONST CHAR *fmt,
    ...
    )
{
    va_list     arg;
    size_t      length;
    CHAR        buffer[MAX_PATH];

    va_start(arg, fmt);
    length = vsnprintf(buffer, MAX_PATH, fmt, arg);
    va_end(arg);

    char c;
    char *s = buffer;
    if (gWriteLoadBar == TRUE)
    {
        gOffsetLoadBar = LOAD_BAR;
    }

    while ((c = *s++))
    {
        ConsolePutChar(c);

        if (gWriteLoadBar)
        {
            gOffsetLoadBar++;
        }
        if (gWriteMiniHv)
        {
            gOffsetMiniHvBar++;
        }
    }
    return TRUE;
}


VOID
WriteMiniHv(VOID)
{
    if (VGA_LOG == TRUE)
    {

        gOffsetMiniHvBar = OFFSET_HV_BAR;
        SetToWriteLoadBar(TRUE, SET_MINIHV_BAR);
        CConsolePrint(MINI_HV_BAR);
        SetToWriteLoadBar(FALSE, SET_MINIHV_BAR);
        IncrementLoaded(0, SET_LOAD_BAR);

        s_row = 1;
    }
}

UINT
ConsoleGetCursor()
{
    return s_cursor;
}

