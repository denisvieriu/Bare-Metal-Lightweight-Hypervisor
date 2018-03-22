#ifndef PIT_H
#define PIT_H

#include "minihv.h"

extern volatile DWORD g_pitTicks;

void PitInit();
void PitWait(DWORD ms);

#endif // PIT_H
