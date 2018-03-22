#include "memzero.h"
#include "assert.h"

void *memzero(void *mem, size_t n)
{
    size_t i, j;
    unsigned long long *q;
    unsigned long long qzero = 0ULL;
    unsigned char *b;
    unsigned char bzero = 0U;

    assert(mem != NULL);
    assert(n > 0);

    i = 0;
    if (n - i >= sizeof(qzero))
    {
        q = mem;
        q += i;
        q[0] = qzero;
        for (j = 1; j < (n - i) / sizeof(qzero); j++)
        {
            q[j] = q[j - 1];
        }
        i += j * sizeof(qzero);
    }

    if (i >= n)
    {
        return mem;
    }

    b = mem;
    b += i;
    b[0] = bzero;
    for (j = 1; j < n - i; j++)
    {
        b[j] = b[j - 1];
    }
    return mem;
}