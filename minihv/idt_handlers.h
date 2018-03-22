#pragma once

#include "minihv.h"


typedef struct _INTERRUPT_STACK
{
    QWORD                       Rip;
    QWORD                       CS;
    QWORD                       RFLAGS;
    QWORD                       Rsp;
    QWORD                       SS;
} INTERRUPT_STACK, *PINTERRUPT_STACK;

typedef enum _EXCEPTION
{
    ExceptionDivideError = 0,
    ExceptionDebugException,
    ExceptionNMI,
    ExceptionBreakpoint,
    ExceptionOverflow,
    ExceptionBoundRange,
    ExceptionInvalidOpcode,
    ExceptionDeviceNotAvailable,
    ExceptionDoubleFault,
    ExceptionCoprocOverrun,
    ExceptionInvalidTSS,
    ExceptionSegmentNotPresent,
    ExceptionStackFault,
    ExceptionGeneralProtection,
    ExceptionPageFault = 14,
    ExceptionX87FpuException = 16,
    ExceptionAlignmentCheck,
    ExceptionMachineCheck,
    ExceptionSIMDFpuException,
    ExceptionVirtualizationException
} EXCEPTION;


typedef struct _INTERRUPT_STACK_COMPLETE
{
    QWORD                       ErrorCode;
    INTERRUPT_STACK             Registers;
} INTERRUPT_STACK_COMPLETE, *PINTERRUPT_STACK_COMPLETE;

