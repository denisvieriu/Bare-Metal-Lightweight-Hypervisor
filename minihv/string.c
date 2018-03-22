#include "string.h"

PCHAR
CustomStrcpy(
    PCHAR dest,
    CONST CHAR* src)
{
	char *temp = dest;
	while ((*dest++ = *src++) != '\0')
		;
	return temp;
}


PCHAR CustomStrcat(
    PCHAR dest,
    CONST CHAR* src
)
{
	char *rdest = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
	return rdest;
}

PCHAR
CustomStrncat(
    PCHAR dest,
    CONST PCHAR src,
    size_t n
)
{
    size_t dest_len = CustomStrlen(dest);
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[dest_len + i] = src[i];
    dest[dest_len + i] = '\0';

    return dest;
}


#pragma warning(push)
#pragma warning( disable : 4305 )
#pragma warning( disable : 4702 )
static CONST unsigned long mask01 = 0x0101010101010101;
static CONST unsigned long mask80 = 0x8080808080808080;

#define	LONGPTR_MASK (sizeof(long) - 1)

/*
* Helper macro to return string length if we caught the zero
* byte.
*/
#define testbyte(x)				    \
	do {					        \
		if (p[x] == '\0')		    \
		    return (p - str + x);	\
	} while (0)

size_t
CustomStrlen(CONST char *str)
{
    CONST CHAR *p;
    CONST unsigned long *lp;
    long va, vb;
    /*
	 * Before trying the hard (unaligned byte-by-byte access) way
	 * to figure out whether there is a nul character, try to see
	 * if there is a nul character is within this accessible word
	 * first.
	 *
	 * p and (p & ~LONGPTR_MASK) must be equally accessible since
	 * they always fall in the same memory page, as long as page
	 * boundaries is integral multiple of word size.
	*/
    lp = (CONST unsigned long *)((INT64)str & ~LONGPTR_MASK);
    va = (*lp - mask01);
    vb = ((~*lp) & mask80);
    lp++;
    if (va & vb)
        /* Check if we have \0 in the first part */
        for (p = str; p < (CONST char *)lp; p++)
            if (*p == '\0')
                return (p - str);

    /* Scan the rest of the string using word sized operation */
    for (; ; lp++)
    {
        va = (*lp - mask01);
        vb = ((~*lp) & mask80);
        if (va & vb)
        {
            p = (CONST char*)(lp);
            testbyte(0);
            testbyte(1);
            testbyte(2);
            testbyte(3);
            /*testbyte(4);
            testbyte(5);
            testbyte(6);
            testbyte(7);*/

        }
    }

    /* NOTREACHED */
    return (0);
}
#pragma warning(pop)

char *CustomStrcatC(char * dest, CONST char src)
{
	char *rdest = dest;

	while (*dest != '\0')
		dest++;
	*dest++ = src;
	*dest = '\0';
	return rdest;
}





