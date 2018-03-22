#include "sleep.h"

VOID
Sleep(
    DWORD MicroSecs
    )
{
    {
        while (MicroSecs > 0)
        {
            __outbyte(0x80, 0);     // approx 1 us, using DMA controller
            MicroSecs--;
        }
    }
}
