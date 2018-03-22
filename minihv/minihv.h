#ifndef _MINIHV_H_
#define _MINIHV_H_

#include "sal.h"
#include "ntstatus.h"

#define InterlockedIncrement _InterlockedIncrement
#define InterlockedDecrement _InterlockedDecrement

#ifdef __cplusplus
extern "C++"
{
    template <typename _CountofType, size_t _SizeOfArray>
    char(*__countof_helper(_UNALIGNED _CountofType(&_Array)[_SizeOfArray]))[_SizeOfArray];

#define __crt_countof(_Array) (sizeof(*__countof_helper(_Array)) + 0)
}
#else
#define __crt_countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

#ifndef _countof
#define _countof __crt_countof
#endif

#define DECLSPEC_ALIGN(x)   __declspec(align(x))
#define DECLSPEC_NORETURN   __declspec(noreturn)
#define FORCEINLINE         __forceinline
#define C_ASSERT(x)         static_assert(x, "Error")
#define UNREFERENCED_PARAMETER(x)   (x)

#define typeof __typeof__
#define bool   _Bool

#ifndef CONST
#define CONST               const
#endif

#ifdef _WIN64
typedef unsigned __int64 size_t;
#else
typedef unsigned int     size_t;
#endif

#ifndef _UINTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
#ifdef _WIN64
typedef unsigned __int64  uintptr_t;
#else
typedef unsigned int uintptr_t;
#endif
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#ifndef FORCEINLINE
#if (_MSC_VER >= 1200)
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __inline
#endif
#endif

#if defined(_M_MRX000) && !(defined(MIDL_PASS) || defined(RC_INVOKED)) && defined(ENABLE_RESTRICTED)
#define RESTRICTED_POINTER __restrict
#else
#define RESTRICTED_POINTER
#endif

#define MAX_PATH 255
#define MIN_PATH -999

#ifndef VOID
#define VOID void
typedef unsigned char UCHAR;
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#if !defined(MIDL_PASS)
typedef int INT;
#endif
#endif


#ifdef FALSE
#undef FALSE
#endif
#define FALSE                           (1 == 0)

#ifdef TRUE
#undef TRUE
#endif
#define TRUE                            (1 == 1)

#ifndef NULL
#define NULL                            (void *) 0
#endif

#define MAX_NIBBLE                  0xFU
#define MAX_BYTE                    0xFFU
#define MAX_WORD                    0xFFFFU
#define MAX_DWORD                   0xFFFFFFFFUL
#define MAX_QWORD                   0xFFFFFFFFFFFFFFFFULL

#define SIZEOF_BYTE_IN_BITS         0x8ULL
#define SIZEOF_BYTE_TO_BITS(x)      (x * SIZEOF_BYTE_IN_BITS)

#define GET_MASK_OF_1_BIT_SET(n)                  (1ULL << n)         // returns a mask with only one bit 1 set: 000010000
#define GET_MASK_OF_0_BIT_SET(n)                  (~SET_BIT(n))       // returns a mask with only one bit 0 set: 111110111


typedef CHAR *PCHAR, *LPCH, *PCH;
typedef CONST CHAR *LPCCH, *PCCH;

typedef unsigned int        UINT;

typedef void *PVOID;

//
// standard types - define them with explicit length
//
typedef unsigned __int8     BYTE, *PBYTE;
typedef unsigned __int16    WORD, *PWORD;
typedef unsigned __int32    DWORD, *PDWORD;
typedef unsigned __int64    QWORD, *PQWORD;
typedef signed __int8       INT8;
typedef signed __int16      INT16;
typedef signed __int32      INT32;
typedef signed __int64      INT64;


typedef int                 errno_t;
typedef unsigned short      wint_t;
typedef unsigned short      wctype_t;
typedef long                __time32_t;
typedef __int64             __time64_t;


#ifndef _CRT_NO_TIME_T
#ifdef _USE_32BIT_TIME_T
typedef __time32_t time_t;
#else
typedef __time64_t time_t;
#endif
#endif

VOID
IncrementNumberOfCpusInVmxMode(
    VOID
);

VOID
DecrementNumberOfCpusInVmxMode(
    VOID
);


#define VGA_LOG     TRUE
#define SERIAL_LOG  TRUE

//
typedef BYTE                BOOLEAN;
//

//
// special TRACE32 macro
//
#define BREAK_INTO_TRACE32(BreakVal)    __outbyte(0xBDB0, (BYTE)(BreakVal))


#endif // _MINIHV_H_