#ifndef _STRING_H
#define _STRING_H

#include "minihv.h"

PVOID
mMmemset(
    void *s,
    int c,
    size_t n
    );

PVOID
mMmemcpy(
    void *dst,
    const void *src,
    size_t n
    );

PVOID
mmemmove(
    void *dst,
    const void *src,
    size_t n
    );

PVOID
mmemchr(
    const void *buf,
    int c,
    size_t n
    );

INT
mMmemcmp(
    const void *s1,
    const void *s2,
    size_t n
    );

UINT
sMstrlen(
    const char *str
    );

PCHAR
sMstrcpy(
    char *dst,
    const char *src
    );

PCHAR
sstrncpy(
    char *dst,
    const char *src,
    size_t n
    );

INT
sMstrcmp(
    const char *s1,
    const char *s2
    );

PCHAR
sMstrcpy_safe(
    char *dst,
    const char *src,
    size_t dstSize
    );

#endif
