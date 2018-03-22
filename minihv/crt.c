#include "crt.h"
#include "print.h"
#include "format.h"
#include "ssnprintf.h"

PCHAR
CustomItoa(
    _In_  QWORD Value,
    _Out_ PCHAR Str,
    _In_  INT32 Base
)
{
	PCHAR	rc;
	PCHAR	ptr;
	PCHAR	low;
	// Check for supported base.
	if (Base < 2 || Base > 36)
	{
		*Str = '\0';
		return Str;
	}
	rc = ptr = Str;
	// Set '-' for negative decimals.
	if (Value < 0 && Base == 10)
	{
		*ptr++ = '-';
	}
	// Remember where the numbers start.
	low = ptr;
	// The actual conversion.
	do
	{
		// Modulo is negative for negative value. This trick makes abs() unnecessary.
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + Value % Base];
		Value /= Base;
	} while (Value);
	// Terminating the string.
	*ptr-- = '\0';
	// Invert the numbers.
	while (low < ptr)
	{
		CHAR tmp = *low;
		*low++ = *ptr;
		*ptr-- = tmp;
	}
	return rc;
}

#pragma warning(disable:4244)
size_t
CustomSnprintFunc(
    PCHAR buffer,
    INT n,
    CHAR CONST * fmt,
    va_list arg
)
{

	INT		INTTemp;
	CHAR	CHARTemp;
	PCHAR	stringTemp;

    CHAR	copyBuffer[MAX_PATH];
	CHAR	ch;
	size_t	length;

    QWORD QWORDTEMP;

    length    = 0;
	buffer[0] = '\0';
    n         = 0;

	while ((ch = *fmt++) != '\0')
	{
		copyBuffer[0] = '\0';
		if ('%' == ch)
		{
			switch (ch = *fmt++)
			{
				/* %% - prINT out a single %    */
			case '%':
				CustomStrcatC(buffer, '%');
				length++;
				break;

				/* %c: prINT out a CHARacter    */
			case 'c':
				CHARTemp = va_arg(arg, INT);
				CustomStrcatC(buffer, CHARTemp);
				length++;
				break;

				/* %s: prINT out a string       */
			case 's':
				stringTemp = va_arg(arg, PCHAR);
				CustomStrcat(buffer, stringTemp);
				length += CustomStrlen(stringTemp);
				break;

				/* %d: prINT out an INT         */
			case 'd':
				INTTemp = va_arg(arg, INT);
				CustomItoa(INTTemp, copyBuffer, 10);
				CustomStrcat(buffer, copyBuffer);
				length += CustomStrlen(copyBuffer);
				break;

				/* %x: prINT out an INT in hex  */
            case 'u':
                QWORDTEMP = va_arg(arg, QWORD);
                CustomItoa(QWORDTEMP, copyBuffer, 10);
                CustomStrcat(buffer, copyBuffer);
                length += CustomStrlen(copyBuffer);
                break;
			case 'x':
				INTTemp = va_arg(arg, INT);
				CustomItoa(INTTemp, copyBuffer, 16);
				CustomStrcat(buffer, copyBuffer);
				length += CustomStrlen(copyBuffer);
				break;
            case 'X':
                QWORDTEMP = va_arg(arg, QWORD);
                CustomItoa(QWORDTEMP, copyBuffer, 16);
                CustomStrcat(buffer, copyBuffer);
                length += CustomStrlen(copyBuffer);
                break;
            case 'p':
            {
                void *p = va_arg(arg, void *);
                CustomStrcatC(buffer, '0');
                CustomStrcatC(buffer, 'x');
                unsigned long long nnn = (uintptr_t)p;
                CustomItoa(nnn, copyBuffer, 16);
                CustomStrcat(buffer, copyBuffer);
                length += CustomStrlen(copyBuffer);
                break;
            }
			}
		}
		else
		{
			CustomStrcatC(buffer, ch);
			length++;
		}
	}
	return length;
}

size_t
CustomSnprintf(
    PCHAR Buffer,
    INT N,
    CONST CHAR* Fmt,
    ...
)
{
	va_list arg;
	size_t length;

	va_start(arg, Fmt);
	length = Rpl_vsnprintf(Buffer, N,  Fmt, arg);
	va_end(arg);

	return length;
}

size_t
CustomPrintf(
    CONST PCHAR Fmt,
    ...
)
{
	va_list     arg;
    size_t      length;
	CHAR        buffer[MAX_PATH];

	va_start(arg, Fmt);
	length = Rpl_vsnprintf(buffer, MAX_PATH, Fmt, arg);
	va_end(arg);

	Puts(buffer);
	return length;
}

VOID
CustomMemcpy(
    PVOID dest,
    PVOID src,
    size_t n
)
{
    // Typecast src and dest addresses to (PCHAR)
    PCHAR csrc  = (PCHAR)src;
    PCHAR cdest = (PCHAR)dest;

    // Copy contents of src[] to dest[]
    for (INT i = 0; i < n; i++)
        cdest[i] = csrc[i];
}



VOID*
CustomMemset(
    VOID *b,
    INT c,
    INT len)
{
    INT i;
    UCHAR* p = b;
    i = 0;
    while (len > 0)
    {
        *p = c;
        p++;
        len--;
    }
    return(b);
}

