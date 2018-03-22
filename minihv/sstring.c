#include "sstring.h"

#pragma warning(disable:4244)
PVOID
mMmemset(void *s, int c, size_t n)
{
    BYTE *p = (BYTE *)s;
    BYTE *end = p + n;

    while (p != end)
    {
        *p++ = c;
    }

    return s;
}

void *mMmemcpy(void *dst, const void *src, size_t n)
{
    BYTE *p = (BYTE *)src;
    BYTE *q = (BYTE *)dst;
    BYTE *end = p + n;

    while (p != end)
    {
        *q++ = *p++;
    }

    return dst;
}

void *mmemmove(void *dst, const void *src, size_t n)
{
    BYTE *p = (BYTE *)src;
    BYTE *q = (BYTE *)dst;
    BYTE *end = p + n;

    if (q > p && q < end)
    {
        p = end;
        q += n;

        while (p != src)
        {
            *--q = *--p;
        }
    }
    else
    {
        while (p != end)
        {
            *q++ = *p++;
        }
    }

    return dst;
}

void *mmemchr(const void *buf, int c, size_t n)
{
    BYTE *p = (BYTE *)buf;
    BYTE *end = p + n;

    while (p != end)
    {
        if (*p == c)
        {
            return p;
        }

        ++p;
    }

    return 0;
}

int mMmemcmp(const void *s1, const void *s2, size_t n)
{
    const BYTE *byte1 = (const BYTE *)s1;
    const BYTE *byte2 = (const BYTE *)s2;
    while ((*byte1 == *byte2) && (n > 0))
    {
        ++byte1;
        ++byte2;
        --n;
    }

    if (n == 0)
    {
        return 0;
    }
    return *byte1 - *byte2;
}

UINT sMstrlen(const char *str)
{
    const char *s = str;
    while (*s++)
        ;

    return (UINT)(s - str - 1);
}

#pragma warning(disable:4706)
char *sMstrcpy(char *dst, const char *src)
{
    char c;
    char *p = dst;

    while ((c = *src++))
    {
        *p++ = c;
    }

    *p = '\0';
    return dst;
}

char *sstrncpy(char *dst, const char *src, size_t n)
{
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        dst[i] = src[i];
    }

    for (; i < n; i++)
    {
        dst[i] = '\0';
    }

    return dst;
}

int sMstrcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2)
    {
        if (*s1 == '\0')
        {
            return 0;
        }

        ++s1;
        ++s2;
    }

    return *s1 - *s2;
}

#pragma warning(disable:4706)
PCHAR sMstrcpy_safe(char *dst, const char *src, size_t dstSize)
{
    char *p = dst;
    char *end = dst + dstSize - 1;
    char c;

    while ((c = *src++) && dst < end)
    {
        *p++ = c;
    }

    if (p < end + 1)
    {
        *p = '\0';
    }

    return dst;
}
