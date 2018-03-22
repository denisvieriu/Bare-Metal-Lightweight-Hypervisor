#include "list.h"

static inline VOID
LinkInit(
    DLL *x
    )
{
    x->Prev = x;
    x->Next = x;
}

static inline VOID
LinkAfter(
    DLL *a,
    DLL *x
    )
{
    DLL *p = a;
    DLL *n = a->Next;
    n->Prev = x;
    x->Next = n;
    x->Prev = p;
    p->Next = x;
}

static inline VOID
LinkBefore(
    DLL *a,
    DLL *x
    )
{
    DLL *p = a->Prev;
    DLL *n = a;
    n->Prev = x;
    x->Next = n;
    x->Prev = p;
    p->Next = x;
}

static inline VOID
LinkRemove(
    DLL *x
    )
{
    DLL *p = x->Prev;
    DLL *n = x->Next;
    n->Prev = p;
    p->Next = n;
    x->Next = 0;
    x->Prev = 0;
}

static inline VOID
LinkMoveAfter(
    DLL *a,
    DLL *x
    )
{
    DLL *p = x->Prev;
    DLL *n = x->Next;
    n->Prev = p;
    p->Next = n;

    p = a;
    n = a->Next;
    n->Prev = x;
    x->Next = n;
    x->Prev = p;
    p->Next = x;
}

static inline VOID
LinkMoveBefore(
    DLL *a,
    DLL *x
    )
{
    DLL *p = x->Prev;
    DLL *n = x->Next;
    n->Prev = p;
    p->Next = n;

    p = a->Prev;
    n = a;
    n->Prev = x;
    x->Next = n;
    x->Prev = p;
    p->Next = x;
}

static inline bool
ListIsEmpty(
    DLL *x
    )
{
    return x->Next == x;
}

#define LinkData(link,T,m) \
    (T *)((char *)(link) - (unsigned long)(&(((T*)0)->m)))

#define ListForEach(it, list, m) \
    for (it = LinkData((list).Next, typeof(*it), m); \
        &it->m != &(list); \
        it = LinkData(it->m.Next, typeof(*it), m))

#define ListForEachSafe(it, n, list, m) \
    for (it = LinkData((list).Next, typeof(*it), m), \
        n = LinkData(it->m.Next, typeof(*it), m); \
        &it->m != &(list); \
        it = n, \
        n = LinkData(n->m.Next, typeof(*it), m))