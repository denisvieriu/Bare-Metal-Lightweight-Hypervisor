#ifndef _SLEEP_H
#define _SLEEP_H

#define SEC                 60000
#define DEFAULT_SLEEP_BLINK 30000
#define REAL_SEC            SEC / 4


#include "minihv.h"

VOID
Sleep(
    DWORD MicroSecs                    /// ...
);

#endif
