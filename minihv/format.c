#include "format.h"

#pragma warning (disable : 4244)
#pragma warning (disable : 4146)
typedef enum _PADDING
{
    PAD_ZERO = 1,
    PAD_LEFT = 2,
}PADDING;

typedef struct _FORMATTER
{
    CHAR *p;
    CHAR *end;
    UINT flags;
    INT width;
} FORMATTER;

static bool
IsSpace(
    CHAR c
)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

static bool
IsDigit(
    CHAR c
    )
{
    return c >= '0' && c <= '9';
}

static VOID OutputChar(
    FORMATTER *f,
    CHAR c
)
{
    if (f->p < f->end)
    {
        *f->p++ = c;
    }
}

static VOID
OutputString(
    FORMATTER *f,
    CONST CHAR *s
    )
{
    INT width = f->width;
    CHAR padCHAR = f->flags & PAD_ZERO ? '0' : ' ';

    if (~f->flags & PAD_LEFT)
    {
        while (--width >= 0)
        {
            OutputChar(f, padCHAR);
        }
    }

    while (*s)
    {
        OutputChar(f, *s++);
    }

    while (--width >= 0)
    {
        OutputChar(f, padCHAR);
    }
}

static VOID
OutputDec(
    FORMATTER *f,
    unsigned long long n
    )
{
    CHAR buf[32];
    CHAR *end = buf + sizeof(buf) - 1;
    CHAR *s = end;
    *s = '\0';

    do
    {
        CHAR c = '0' + (n % 10);
        *--s = c;
        n /= 10;
    } while (n > 0);

    f->width -= end - s;
    OutputString(f, s);
}

static VOID
OutputHex(
    FORMATTER *f,
    CHAR type,
    unsigned long long n
    )
{
    CHAR buf[32];
    CHAR *end = buf + sizeof(buf) - 1;
    CHAR *s = end;
    *s = '\0';

    do
    {
        UINT digit = n & 0xf;
        CHAR c;
        if (digit < 10)
        {
            c = '0' + digit;
        }
        else if (type == 'x')
        {
            c = 'a' + digit - 10;
        }
        else
        {
            c = 'A' + digit - 10;
        }

        *--s = c;
        n >>= 4;
    } while (n > 0);

    f->width -= end - s;
    OutputString(f, s);
}

static VOID
OutputPointer(
    FORMATTER *f,
    VOID *p
    )
{
    unsigned long long n = (uintptr_t)p;
    OutputHex(f, 'x', n);
}

INT
vsnprintf(
    CHAR *str,
    size_t size,
    CONST CHAR *fmt,
    va_list args
    )
{
    FORMATTER f;
    f.p = str;
    f.end = str + size - 1;

    for (;;)
    {
        // Read next CHARacter
        CHAR c = *fmt++;
        if (!c)
        {
            break;
        }

        // Output non-format CHARacter
        if (c != '%')
        {
            OutputChar(&f, c);
            continue;
        }

        // Parse type specifier
        c = *fmt++;

        // Parse flags
        f.flags = 0;
        if (c == '-')
        {
            f.flags |= PAD_LEFT;
            c = *fmt++;
        }
        else if (c == '0')
        {
            f.flags |= PAD_ZERO;
            c = *fmt++;
        }

        // Parse width
        f.width = -1;
        if (IsDigit(c))
        {
            INT width = 0;
            do
            {
                width = width * 10 + c - '0';
                c = *fmt++;
            } while (IsDigit(c));

            f.width = width;
        }

        // Parse length modifier
        bool isLongLong = FALSE;

        if (c == 'l')
        {
            c = *fmt++;
            if (c == 'l')
            {
                c = *fmt++;
                isLongLong = TRUE;
            }
        }

        // Process type specifier
        CHAR type = c;
        switch (type)
        {
        case '%':
            OutputChar(&f, '%');
            break;

        case 'c':
            c = va_arg(args, INT);
            OutputChar(&f, c);
            break;

        case 's':
        {
            CHAR *s = va_arg(args, CHAR *);
            if (!s)
            {
                s = "(null)";
            }

            if (f.width > 0)
            {
                CHAR *p = s;
                while (*p)
                {
                    ++p;
                }

                f.width -= p - s;
            }

            OutputString(&f, s);
        }
        break;

        case 'd':
        {
            long long n;
            if (isLongLong)
            {
                n = va_arg(args, long long);
            }
            else
            {
                n = va_arg(args, INT);
            }

            if (n < 0)
            {
                OutputChar(&f, '-');
                n = -n;
            }

            OutputDec(&f, n);
        }
        break;

        case 'u':
        {
            unsigned long long n;
            if (isLongLong)
            {
                n = va_arg(args, unsigned long long);
            }
            else
            {
                n = va_arg(args, UINT);
            }

            OutputDec(&f, n);
        }
        break;

        case 'x':
        case 'X':
        {
            unsigned long long n;
            if (isLongLong)
            {
                n = va_arg(args, unsigned long long);
            }
            else
            {
                n = va_arg(args, UINT);
            }

            OutputHex(&f, type, n);
        }
        break;

        case 'p':
        {
            VOID *p = va_arg(args, VOID *);

            OutputChar(&f, '0');
            OutputChar(&f, 'x');
            OutputPointer(&f, p);
        }
        break;
        }
    }

    if (f.p < f.end + 1)
    {
        *f.p = '\0';
    }

    return f.p - str;
}

INT
snprintf(
    CHAR *str,
    size_t size,
    CONST CHAR *fmt,
    ...
    )
{
    va_list args;

    va_start(args, fmt);
    INT len = vsnprintf(str, size, fmt, args);
    va_end(args);

    return len;
}

INT
vsscanf(
    CONST CHAR *str,
    CONST CHAR *fmt,
    va_list args
    )
{
    INT count = 0;

    for (;;)
    {
        // Read next CHARacter
        CHAR c = *fmt++;
        if (!c)
        {
            break;
        }
        if (IsSpace(c))
        {
            // Whitespace
            while (IsSpace(*str))
            {
                ++str;
            }
        }
        else if (c != '%')
        {
        match_literal:
            // Non-format CHARacter
            if (*str == '\0')
            {
                goto end_of_input;
            }

            if (*str != c)
            {
                goto match_failure;
            }

            ++str;
        }
        else
        {
            // Parse type specifider
            c = *fmt++;

            // Process type specifier
            CHAR type = c;
            switch (type)
            {
            case '%':
                goto match_literal;

            case 'd':
            {
                INT sign = 1;

                c = *str++;
                if (c == '\0')
                    goto end_of_input;

                if (c == '-')
                {
                    sign = -1;
                    c = *str++;
                }

                INT n = 0;
                while (IsDigit(c))
                {
                    n = n * 10 + c - '0';
                    c = *str++;
                }

                n *= sign;
                --str;

                INT *result = va_arg(args, INT *);
                *result = n;
                ++count;
            }
            break;
            }
        }
    }

match_failure:
    return count;

end_of_input:
    return count ? count : -1;
}

INT sscanf(
    CONST CHAR *str,
    CONST CHAR *fmt,
    ...
    )
{
    va_list args;

    va_start(args, fmt);
    INT count = vsscanf(str, fmt, args);
    va_end(args);

    return count;
}

UINT
myStrtoul(
    CONST CHAR *nptr,
    CHAR **endptr,
    INT base
    )
{
    CONST CHAR *pCurrentCHAR = nptr;

    // Skip whitespace
    while (IsSpace(*pCurrentCHAR))
    {
        ++pCurrentCHAR;
    }

    // Optionally there maybe a sign.
    bool neg = FALSE;
    if (*pCurrentCHAR == '-')
    {
        neg = TRUE;
        ++pCurrentCHAR;
    }
    else if (*pCurrentCHAR == '+')
    {
        ++pCurrentCHAR;
    }

    if (base == 0)
    {
        // detect base;
        if (*pCurrentCHAR == '0')
        {
            ++pCurrentCHAR;
            if (*pCurrentCHAR == 'x')
            {
                base = 16;
                ++pCurrentCHAR;
            }
            else
            {
                base = 8;
            }
        }
        else
        {
            base = 10;
        }
    }
    else if (base == 16)
    {
        if (*pCurrentCHAR == '0' && *(pCurrentCHAR + 1) == 'x')
        {
            pCurrentCHAR += 2;
        }
    }
    // Don't really need to skip leading 0 for oct.


    // I've not worried about limit error handling
    UINT result = 0;
    bool done = FALSE;
    while (!done)
    {
        CHAR currentCHAR = *pCurrentCHAR;
        INT  currentValue = 0;
        if (currentCHAR >= '0' && currentCHAR <= '9')
        {
            currentValue = currentCHAR - '0';
        }
        else if (currentCHAR >= 'a' && currentCHAR <= 'z')
        {
            currentValue = currentCHAR - 'a' + 10;
        }
        else if (currentCHAR >= 'A' && currentCHAR <= 'Z')
        {
            currentValue = currentCHAR - 'A' + 10;
        }
        else
        {
            done = TRUE;
        }

        if (!done)
        {
            if (currentValue >= base)
            {
                done = TRUE;
            }
            else
            {
                ++pCurrentCHAR;
                result *= base;
                result += currentValue;
            }
        }
    }

    if (neg)
    {
        result = -result;
    }

    if (endptr != 0)
    {
        *endptr = (CHAR *)pCurrentCHAR;
    }

    return result;
}
