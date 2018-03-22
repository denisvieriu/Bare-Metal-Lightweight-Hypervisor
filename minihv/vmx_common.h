#ifndef _VMX_COMMON_H
#define _VMX_COMMON_H

#include "minihv.h"
#include "dbglog.h"
#include "ntstatus.h"
#include "minihv_status.h"
#include "cpu.h"
#include "cpuid.h"
#include "vmx.h"


/*++

Reoutine Description:

Execute VMXRESUME

--*/
DECLSPEC_NORETURN
VOID
MhvVmxResume(
    VOID
    );

/*++

Reoutine Description:

VMXREAD - returns an error code, instead of the data read
Also this function also does error checking, and returns the data read

Arguments:

Field - Field encoding

Return Value:

Data - data read from Field

--*/
QWORD
FORCEINLINE
MhvVmxRead(
    _In_ size_t Field
    );

/*++

Reoutine Description:

Execute VMLAUNCH

--*/
DWORD
MhvVmxLaunch(
    VOID
    );


NTSTATUS
MhvVmxHardSupported(
    VOID
    );


NTSTATUS
MhvVmxInit(
    _In_ PCPU pCpu
    );

BOOLEAN
MhvPresentHypervisor(
    VOID
    );

VOID
MhvVmxExitHandler(
    _In_ PPROCESSOR_STATE ProcessorState
    );

DECLSPEC_NORETURN
VOID
MhvVmxEntryHandler(
    _In_ PPROCESSOR_STATE ProcessorState
    );

#endif _VMX_COMMON_H