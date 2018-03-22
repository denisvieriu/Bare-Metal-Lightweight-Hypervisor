#include "spinlock.h"
#include "memzero.h"
#include "console.h"

#define LOCK_FREE   0
#define LOCK_TAKEN  1



VOID
LockInit(
    _Out_ PLOCK pl
    )
{
    memzero((long*)pl, sizeof(pl));
    _InterlockedExchange((long *)pl, LOCK_IS_FREE);
}

VOID
Lock(
    _Out_ PLOCK pl
    )
{
    while (_InterlockedCompareExchange((long *)pl,
        LOCK_IS_TAKEN,
        LOCK_IS_FREE) == LOCK_IS_TAKEN)
    {
        //ConsoleLogWarning("Trying acquire lock!!!");
    }
    //ConsoleLogWarning("LOCK ACQUIRED");
}


VOID
Unlock(
    _Out_ PLOCK pl
    )
{
    _InterlockedExchange((long *)pl, LOCK_IS_FREE);
}