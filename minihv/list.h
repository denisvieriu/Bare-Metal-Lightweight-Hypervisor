#ifndef _LIST_H
#define _LIST_H

#include "minihv.h"

typedef struct DLL
{
    struct DLL *Prev;
    struct DLL *Next;
} DLL, *PDLL;

static inline VOID
LinkInit(
    DLL *x
    );

static inline VOID
LinkAfter(
    DLL *a,
    DLL *x
    );

static inline VOID
LinkBefore(
    DLL *a,
    DLL *x
    );

static inline
VOID LinkRemove(
    DLL *x
    );

static inline VOID
LinkMoveAfter(
    DLL *a,
    DLL *x
    );

static inline VOID
LinkMoveBefore(
    DLL *a,
    DLL *x
    );

static inline bool
ListIsEmpty(
    DLL *x
    );

#endif // _LIST_H
