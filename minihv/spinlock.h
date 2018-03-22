#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include "minihv.h"

typedef unsigned LOCK, *PLOCK;

typedef enum _LOCK
{
    LOCK_IS_FREE,
    LOCK_IS_TAKEN
}LOCK;

VOID
LockInit(
    _Out_ PLOCK pl
);

VOID
Lock(
    _Out_ PLOCK pl
);

VOID
Unlock(
    _Out_ PLOCK pl
);
#endif // _SPINLOCK_H
