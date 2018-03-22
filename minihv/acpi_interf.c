/******************************************************************************
*
* Module Name: oswinxf - Windows OSL
*
*****************************************************************************/

/******************************************************************************
*
* 1. Copyright Notice
*
* Some or all of this work - Copyright (c) 1999 - 2017, Intel Corp.
* All rights reserved.
*
* 2. License
*
* 2.1. This is your license from Intel Corp. under its intellectual property
* rights. You may have additional license terms from the party that provided
* you this software, covering your right to use that party's intellectual
* property rights.
*
* 2.2. Intel grants, free of charge, to any person ("Licensee") obtaining a
* copy of the source code appearing in this file ("Covered Code") an
* irrevocable, perpetual, worldwide license under Intel's copyrights in the
* base code distributed originally by Intel ("Original Intel Code") to copy,
* make derivatives, distribute, use and display any portion of the Covered
* Code in any form, with the right to sublicense such rights; and
*
* 2.3. Intel grants Licensee a non-exclusive and non-transferable patent
* license (with the right to sublicense), under only those claims of Intel
* patents that are infringed by the Original Intel Code, to make, use, sell,
* offer to sell, and import the Covered Code and derivative works thereof
* solely to the minimum extent necessary to exercise the above copyright
* license, and in no event shall the patent license extend to any additions
* to or modifications of the Original Intel Code. No other license or right
* is granted directly or by implication, estoppel or otherwise;
*
* The above copyright and patent license is granted only if the following
* conditions are met:
*
* 3. Conditions
*
* 3.1. Redistribution of Source with Rights to Further Distribute Source.
* Redistribution of source code of any substantial portion of the Covered
* Code or modification with rights to further distribute source must include
* the above Copyright Notice, the above License, this list of Conditions,
* and the following Disclaimer and Export Compliance provision. In addition,
* Licensee must cause all Covered Code to which Licensee contributes to
* contain a file documenting the changes Licensee made to create that Covered
* Code and the date of any change. Licensee must include in that file the
* documentation of any changes made by any predecessor Licensee. Licensee
* must include a prominent statement that the modification is derived,
* directly or indirectly, from Original Intel Code.
*
* 3.2. Redistribution of Source with no Rights to Further Distribute Source.
* Redistribution of source code of any substantial portion of the Covered
* Code or modification without rights to further distribute source must
* include the following Disclaimer and Export Compliance provision in the
* documentation and/or other materials provided with distribution. In
* addition, Licensee may not authorize further sublicense of source of any
* portion of the Covered Code, and must include terms to the effect that the
* license from Licensee to its licensee is limited to the intellectual
* property embodied in the software Licensee provides to its licensee, and
* not to intellectual property embodied in modifications its licensee may
* make.
*
* 3.3. Redistribution of Executable. Redistribution in executable form of any
* substantial portion of the Covered Code or modification must reproduce the
* above Copyright Notice, and the following Disclaimer and Export Compliance
* provision in the documentation and/or other materials provided with the
* distribution.
*
* 3.4. Intel retains all right, title, and interest in and to the Original
* Intel Code.
*
* 3.5. Neither the name Intel nor any other trademark owned or controlled by
* Intel shall be used in advertising or otherwise to promote the sale, use or
* other dealings in products derived from or relating to the Covered Code
* without prior written authorization from Intel.
*
* 4. Disclaimer and Export Compliance
*
* 4.1. INTEL MAKES NO WARRANTY OF ANY KIND REGARDING ANY SOFTWARE PROVIDED
* HERE. ANY SOFTWARE ORIGINATING FROM INTEL OR DERIVED FROM INTEL SOFTWARE
* IS PROVIDED "AS IS," AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
* INSTALLATION, TRAINING OR OTHER SERVICES. INTEL WILL NOT PROVIDE ANY
* UPDATES, ENHANCEMENTS OR EXTENSIONS. INTEL SPECIFICALLY DISCLAIMS ANY
* IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* 4.2. IN NO EVENT SHALL INTEL HAVE ANY LIABILITY TO LICENSEE, ITS LICENSEES
* OR ANY OTHER THIRD PARTY, FOR ANY LOST PROFITS, LOST DATA, LOSS OF USE OR
* COSTS OF PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, OR FOR ANY INDIRECT,
* SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THIS AGREEMENT, UNDER ANY
* CAUSE OF ACTION OR THEORY OF LIABILITY, AND IRRESPECTIVE OF WHETHER INTEL
* HAS ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES. THESE LIMITATIONS
* SHALL APPLY NOTWITHSTANDING THE FAILURE OF THE ESSENTIAL PURPOSE OF ANY
* LIMITED REMEDY.
*
* 4.3. Licensee shall not export, either directly or indirectly, any of this
* software or system incorporating such software without first obtaining any
* required license or other approval from the U. S. Department of Commerce or
* any other agency or department of the United States Government. In the
* event Licensee exports any such software from the United States or
* re-exports any such software from a foreign destination, Licensee shall
* ensure that the distribution and export/re-export of the software is in
* compliance with all laws, regulations, orders, or other restrictions of the
* U.S. Export Administration Regulations. Licensee agrees that neither it nor
* any of its subsidiaries will export/re-export any technical data, process,
* software, or service, directly or indirectly, to any country for which the
* United States government or any agency thereof requires an export license,
* other governmental approval, or letter of assurance, without first obtaining
* such license, approval or letter.
*
*****************************************************************************
*
* Alternatively, you may choose to be licensed under the terms of the
* following license:
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions, and the following disclaimer,
*    without modification.
* 2. Redistributions in binary form must reproduce at minimum a disclaimer
*    substantially similar to the "NO WARRANTY" disclaimer below
*    ("Disclaimer") and any redistribution must be conditioned upon
*    including a substantially similar Disclaimer requirement for further
*    binary redistribution.
* 3. Neither the names of the above-listed copyright holders nor the names
*    of any contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Alternatively, you may choose to be licensed under the terms of the
* GNU General Public License ("GPL") version 2 as published by the Free
* Software Foundation.
*
*****************************************************************************/

#include "acpi.h"
#include "accommon.h"
#include "minihv.h"
#include "map.h"
#include "print.h"
#include "dbglog.h"
#include "ssnprintf.h"
#include "format.h"
#include "print.h"

//#ifdef WIN32
//#pragma warning(disable:4115)   /* warning C4115: named type definition in parentheses (caused by rpcasync.h> */
//
//#include <windows.h>
//#include <winbase.h>
//
//#elif WIN64
//#include <windowsx.h>
//#endif
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <process.h>
//#include <time.h>

BOOLEAN gSetErrorCode = FALSE;

#define _COMPONENT          ACPI_OS_SERVICES
ACPI_MODULE_NAME("oswinxf")


UINT64                      TimerFrequency;
char                        TableName[ACPI_NAME_SIZE + 1];

#define ACPI_OS_DEBUG_TIMEOUT   30000 /* 30 seconds */


/* Upcalls to AcpiExec application */

void
AeTableOverride(
    ACPI_TABLE_HEADER       *ExistingTable,
    ACPI_TABLE_HEADER       **NewTable);


/*
* Real semaphores are only used for a multi-threaded application
*/
#ifndef ACPI_SINGLE_THREADED

/* Semaphore information structure */

typedef struct acpi_os_semaphore_info
{
    UINT16                  MaxUnits;
    UINT16                  CurrentUnits;
    void                    *OsHandle;

} ACPI_OS_SEMAPHORE_INFO;

/* Need enough semaphores to run the large aslts suite */

#define ACPI_OS_MAX_SEMAPHORES  256

ACPI_OS_SEMAPHORE_INFO          AcpiGbl_Semaphores[ACPI_OS_MAX_SEMAPHORES];

#endif /* ACPI_SINGLE_THREADED */

/******************************************************************************
*
* FUNCTION:    AcpiOsTerminate
*
* PARAMETERS:  None
*
* RETURN:      Status
*
* DESCRIPTION: Nothing to do for windows
*
*****************************************************************************/

ACPI_STATUS
AcpiOsTerminate(
    void)
{
    LOG_INFO("AcpiOsTerminate called..");

    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsInitialize
*
* PARAMETERS:  None
*
* RETURN:      Status
*
* DESCRIPTION: Init this OSL
*
*****************************************************************************/

ACPI_STATUS
AcpiOsInitialize(
    void)
{
    LOG_INFO("AcpiOsInitialize called");

    return (AE_OK);
}


#ifndef ACPI_USE_NATIVE_RSDP_POINTER
/******************************************************************************
*
* FUNCTION:    AcpiOsGetRootPointer
*
* PARAMETERS:  None
*
* RETURN:      RSDP physical address
*
* DESCRIPTION: Gets the root pointer (RSDP)
*
*****************************************************************************/

ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer(
    void)
{

    ACPI_PHYSICAL_ADDRESS rsdpAddress;

    LOG_INFO("Calling AcpiOsFindRootPointer...");

    AcpiFindRootPointer(&rsdpAddress);

    LOG_INFO("RsdpAddress is: %X", rsdpAddress);

    return rsdpAddress;
}
#endif


/******************************************************************************
*
* FUNCTION:    AcpiOsPredefinedOverride
*
* PARAMETERS:  InitVal             - Initial value of the predefined object
*              NewVal              - The new value for the object
*
* RETURN:      Status, pointer to value. Null pointer returned if not
*              overriding.
*
* DESCRIPTION: Allow the OS to override predefined names
*
*****************************************************************************/

ACPI_STATUS
AcpiOsPredefinedOverride(
    const ACPI_PREDEFINED_NAMES *InitVal,
    ACPI_STRING                 *NewVal)
{
    LOG_INFO("AcpiOsPredefinedOverride called");


    if (!InitVal || !NewVal)
    {
        return (AE_BAD_PARAMETER);
    }

    *NewVal = NULL;

    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsTableOverride
*
* PARAMETERS:  ExistingTable       - Header of current table (probably firmware)
*              NewTable            - Where an entire new table is returned.
*
* RETURN:      Status, pointer to new table. Null pointer returned if no
*              table is available to override
*
* DESCRIPTION: Return a different version of a table if one is available
*
*****************************************************************************/

ACPI_STATUS
AcpiOsTableOverride(
    ACPI_TABLE_HEADER       *ExistingTable,
    ACPI_TABLE_HEADER       **NewTable)
{
    LOG_INFO("AcpiOsTableOverride called");

    if (!ExistingTable || !NewTable)
    {
        return (AE_BAD_PARAMETER);
    }

    *NewTable = NULL;

#ifdef ACPI_EXEC_APP

    /* Call back up to AcpiExec */

    AeTableOverride(ExistingTable, NewTable);
#endif

    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsPhysicalTableOverride
*
* PARAMETERS:  ExistingTable       - Header of current table (probably firmware)
*              NewAddress          - Where new table address is returned
*                                    (Physical address)
*              NewTableLength      - Where new table length is returned
*
* RETURN:      Status, address/length of new table. Null pointer returned
*              if no table is available to override.
*
* DESCRIPTION: Returns AE_SUPPORT, function not used in user space.
*
*****************************************************************************/

ACPI_STATUS
AcpiOsPhysicalTableOverride(
    ACPI_TABLE_HEADER       *ExistingTable,
    ACPI_PHYSICAL_ADDRESS   *NewAddress,
    UINT32                  *NewTableLength)
{
    LOG_INFO("AcpiOsPhysicalTableOverride called");

    if (NULL == NewAddress)
    {
        return AE_BAD_PARAMETER;
    }

    if (NULL == NewTableLength)
    {
        return AE_BAD_PARAMETER;
    }

    *NewAddress = 0;
    *NewTableLength = 0;

    return AE_OK;
}


/******************************************************************************
*
* FUNCTION:    AcpiOsEnterSleep
*
* PARAMETERS:  SleepState          - Which sleep state to enter
*              RegaValue           - Register A value
*              RegbValue           - Register B value
*
* RETURN:      Status
*
* DESCRIPTION: A hook before writing sleep registers to enter the sleep
*              state. Return AE_CTRL_SKIP to skip further sleep register
*              writes.
*
*****************************************************************************/

ACPI_STATUS
AcpiOsEnterSleep(
    UINT8                   SleepState,
    UINT32                  RegaValue,
    UINT32                  RegbValue)
{
    LOG_INFO("AcpiOsEnterSleep called");

    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsGetTimer
*
* PARAMETERS:  None
*
* RETURN:      Current ticks in 100-nanosecond units
*
* DESCRIPTION: Get the value of a system timer
*
******************************************************************************/

UINT64
AcpiOsGetTimer(
    void)
{
    LOG_INFO("AcpiOsGetTimer called");

    return 0;
}


/******************************************************************************
*
* FUNCTION:    AcpiOsReadable
*
* PARAMETERS:  Pointer             - Area to be verified
*              Length              - Size of area
*
* RETURN:      TRUE if readable for entire length
*
* DESCRIPTION: Verify that a pointer is valid for reading
*
*****************************************************************************/

BOOLEAN
AcpiOsReadable(
    void                    *Pointer,
    ACPI_SIZE               Length)
{
    LOG_INFO("AcpiOsReadable called");

    return FALSE;
}


/******************************************************************************
*
* FUNCTION:    AcpiOsWritable
*
* PARAMETERS:  Pointer             - Area to be verified
*              Length              - Size of area
*
* RETURN:      TRUE if writable for entire length
*
* DESCRIPTION: Verify that a pointer is valid for writing
*
*****************************************************************************/

BOOLEAN
AcpiOsWritable(
    void                    *Pointer,
    ACPI_SIZE               Length)
{
    LOG_INFO("AcpiOsWritable called");

    return FALSE;
}


/******************************************************************************
*
* FUNCTION:    AcpiOsRedirectOutput
*
* PARAMETERS:  Destination         - An open file handle/pointer
*
* RETURN:      None
*
* DESCRIPTION: Causes redirect of AcpiOsPrintf and AcpiOsVprintf
*
*****************************************************************************/

void
AcpiOsRedirectOutput(
    void                    *Destination)
{
    LOG_INFO("AcpiOsRedirectOutput called");
}


/******************************************************************************
*
* FUNCTION:    AcpiOsPrintf
*
* PARAMETERS:  Fmt, ...            - Standard printf format
*
* RETURN:      None
*
* DESCRIPTION: Formatted output
*
*****************************************************************************/

#pragma warning(disable : 4090)
void ACPI_INTERNAL_VAR_XFACE
AcpiOsPrintf(
    const char              *Fmt,
    ...)
{
    //LOG_INFO("AcpiOsPrintf called");

    //Fmt;
    //va_list arg;

    //va_start(arg, Fmt);
    //AcpiOsVprintf(Fmt, arg);
    //va_end(arg);

    //IncrementLoaded(1, SET_LOAD_BAR);
    //IncrementLoaded(1, SET_MINIHV_BAR);

}


/******************************************************************************
*
* FUNCTION:    AcpiOsVprintf
*
* PARAMETERS:  Fmt                 - Standard printf format
*              Args                - Argument list
*
* RETURN:      None
*
* DESCRIPTION: Formatted output with argument list pointer
*
*****************************************************************************/
void
AcpiSetErrorCode(
    BOOLEAN errorCode
)
{
    gSetErrorCode = errorCode;
}

void
AcpiOsVprintf(
    const char              *Fmt,
    va_list                 Args)
{
    Fmt;
    Args;
    //LOG_INFO("AcpiOsVprintf called");
    //size_t length;
    //CHAR buffer[MAX_PATH];
    //va_start(Args, Fmt);
    //length = Rpl_vsnprintf(buffer, MAX_PATH, Fmt, Args);
    //va_end(Args);

    //if (gSetErrorCode)
    //{
    //    LOG_ACPI_ERROR("%s", buffer);
    //}
    //else
    //{
    //    LOG_ACPI("%s", buffer);
    //}
    //IncrementLoaded(1, SET_LOAD_BAR);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsGetLine
*
* PARAMETERS:  Buffer              - Where to return the command line
*              BufferLength        - Maximum length of Buffer
*              BytesRead           - Where the actual byte count is returned
*
* RETURN:      Status and actual bytes read
*
* DESCRIPTION: Formatted input with argument list pointer
*
*****************************************************************************/

ACPI_STATUS
AcpiOsGetLine(
    char                    *Buffer,
    UINT32                  BufferLength,
    UINT32                  *BytesRead)
{
    LOG_INFO("AcpiOsGetLine called");

    return (AE_OK);
}


#ifndef ACPI_USE_NATIVE_MEMORY_MAPPING
/******************************************************************************
*
* FUNCTION:    AcpiOsMapMemory
*
* PARAMETERS:  Where               - Physical address of memory to be mapped
*              Length              - How much memory to map
*
* RETURN:      Pointer to mapped memory. Null on error.
*
* DESCRIPTION: Map physical memory into caller's address space
*
*****************************************************************************/

void *
AcpiOsMapMemory(
    ACPI_PHYSICAL_ADDRESS   Where,
    ACPI_SIZE               Length)
{
    PVOID va;

    LOG_INFO("AcpiOsMapMemory called");

    va = MmuMapToVa(Where, Length, NULL, TRUE);
    return va;
}


/******************************************************************************
*
* FUNCTION:    AcpiOsUnmapMemory
*
* PARAMETERS:  Where               - Logical address of memory to be unmapped
*              Length              - How much memory to unmap
*
* RETURN:      None.
*
* DESCRIPTION: Delete a previously created mapping. Where and Length must
*              correspond to a previous mapping exactly.
*
*****************************************************************************/

void
AcpiOsUnmapMemory(
    void                    *Where,
    ACPI_SIZE               Length)
{
    LOG_INFO("AcpiOsUnmapMemory called");

    //MmuUnmapFromVa((QWORD)Where, Length);
    //LOG_INFO("AcpiOsUnmapMemory finished");
}
#endif


/******************************************************************************
*
* FUNCTION:    AcpiOsAllocate
*
* PARAMETERS:  Size                - Amount to allocate, in bytes
*
* RETURN:      Pointer to the new allocation. Null on error.
*
* DESCRIPTION: Allocate memory. Algorithm is dependent on the OS.
*
*****************************************************************************/

void *
AcpiOsAllocate(
    ACPI_SIZE               Size)
{
    VOID                    *Mem;


    Mem = (VOID*)MmuAllocAlignedVa((QWORD)Size);
    return (Mem);
}


#ifdef USE_NATIVE_ALLOCATE_ZEROED
/******************************************************************************
*
* FUNCTION:    AcpiOsAllocateZeroed
*
* PARAMETERS:  Size                - Amount to allocate, in bytes
*
* RETURN:      Pointer to the new allocation. Null on error.
*
* DESCRIPTION: Allocate and zero memory. Algorithm is dependent on the OS.
*
*****************************************************************************/

void *
AcpiOsAllocateZeroed(
    ACPI_SIZE               Size)
{
    void                    *Mem;


    Mem = (void *)calloc(1, (size_t)Size);
    return (Mem);
}
#endif


/******************************************************************************
*
* FUNCTION:    AcpiOsFree
*
* PARAMETERS:  Mem                 - Pointer to previously allocated memory
*
* RETURN:      None.
*
* DESCRIPTION: Free memory allocated via AcpiOsAllocate
*
*****************************************************************************/

void
AcpiOsFree(
    void                    *Mem)
{
    LOG_INFO("AcpiOsFree called");
    Mem;
}


#ifdef ACPI_SINGLE_THREADED
/******************************************************************************
*
* FUNCTION:    Semaphore stub functions
*
* DESCRIPTION: Stub functions used for single-thread applications that do
*              not require semaphore synchronization. Full implementations
*              of these functions appear after the stubs.
*
*****************************************************************************/

ACPI_STATUS
AcpiOsCreateSemaphore(
    UINT32              MaxUnits,
    UINT32              InitialUnits,
    ACPI_HANDLE         *OutHandle)
{
    *OutHandle = (ACPI_HANDLE)1;
    return (AE_OK);
}

ACPI_STATUS
AcpiOsDeleteSemaphore(
    ACPI_HANDLE         Handle)
{
    return (AE_OK);
}

ACPI_STATUS
AcpiOsWaitSemaphore(
    ACPI_HANDLE         Handle,
    UINT32              Units,
    UINT16              Timeout)
{
    return (AE_OK);
}

ACPI_STATUS
AcpiOsSignalSemaphore(
    ACPI_HANDLE         Handle,
    UINT32              Units)
{
    return (AE_OK);
}

#else
/******************************************************************************
*
* FUNCTION:    AcpiOsCreateSemaphore
*
* PARAMETERS:  MaxUnits            - Maximum units that can be sent
*              InitialUnits        - Units to be assigned to the new semaphore
*              OutHandle           - Where a handle will be returned
*
* RETURN:      Status
*
* DESCRIPTION: Create an OS semaphore
*
*****************************************************************************/

ACPI_STATUS
AcpiOsCreateSemaphore(
    UINT32              MaxUnits,
    UINT32              InitialUnits,
    ACPI_SEMAPHORE      *OutHandle)
{
    LOG_INFO("AcpiOsCreateSemaphore called");
    *OutHandle = 0;
    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsDeleteSemaphore
*
* PARAMETERS:  Handle              - Handle returned by AcpiOsCreateSemaphore
*
* RETURN:      Status
*
* DESCRIPTION: Delete an OS semaphore
*
*****************************************************************************/

ACPI_STATUS
AcpiOsDeleteSemaphore(
    ACPI_SEMAPHORE      Handle)
{
    LOG_INFO("AcpiOsDeleteSemaphore called");
    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsWaitSemaphore
*
* PARAMETERS:  Handle              - Handle returned by AcpiOsCreateSemaphore
*              Units               - How many units to wait for
*              Timeout             - How long to wait
*
* RETURN:      Status
*
* DESCRIPTION: Wait for units
*
*****************************************************************************/

ACPI_STATUS
AcpiOsWaitSemaphore(
    ACPI_SEMAPHORE      Handle,
    UINT32              Units,
    UINT16              Timeout)
{
    LOG_INFO("AcpiOsWaitSemaphore called");
    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsSignalSemaphore
*
* PARAMETERS:  Handle              - Handle returned by AcpiOsCreateSemaphore
*              Units               - Number of units to send
*
* RETURN:      Status
*
* DESCRIPTION: Send units
*
*****************************************************************************/

ACPI_STATUS
AcpiOsSignalSemaphore(
    ACPI_SEMAPHORE      Handle,
    UINT32              Units)
{
    LOG_INFO("AcpiOsSignalSemaphore called");
    return (AE_OK);
}

#endif /* ACPI_SINGLE_THREADED */


/******************************************************************************
*
* FUNCTION:    Spinlock interfaces
*
* DESCRIPTION: Map these interfaces to semaphore interfaces
*
*****************************************************************************/

ACPI_STATUS
AcpiOsCreateLock(
    ACPI_SPINLOCK           *OutHandle)
{
    LOG_INFO("AcpiOsCreateLock called");
    return (AcpiOsCreateSemaphore(1, 1, OutHandle));
}

void
AcpiOsDeleteLock(
    ACPI_SPINLOCK           Handle)
{
    LOG_INFO("AcpiOsDeleteLock called");
    AcpiOsDeleteSemaphore(Handle);
}

ACPI_CPU_FLAGS
AcpiOsAcquireLock(
    ACPI_SPINLOCK           Handle)
{
    LOG_INFO("AcpiOsAcquireLock called");
    AcpiOsWaitSemaphore(Handle, 1, 0xFFFF);
    return (0);
}

void
AcpiOsReleaseLock(
    ACPI_SPINLOCK           Handle,
    ACPI_CPU_FLAGS          Flags)
{
    LOG_INFO("AcpiOsReleaseLock called");
    AcpiOsSignalSemaphore(Handle, 1);
}


#if ACPI_FUTURE_IMPLEMENTATION

/* Mutex interfaces, just implement with a semaphore */

ACPI_STATUS
AcpiOsCreateMutex(
    ACPI_MUTEX              *OutHandle)
{
    return (AcpiOsCreateSemaphore(1, 1, OutHandle));
}

void
AcpiOsDeleteMutex(
    ACPI_MUTEX              Handle)
{
    AcpiOsDeleteSemaphore(Handle);
}

ACPI_STATUS
AcpiOsAcquireMutex(
    ACPI_MUTEX              Handle,
    UINT16                  Timeout)
{
    AcpiOsWaitSemaphore(Handle, 1, Timeout);
    return (0);
}

void
AcpiOsReleaseMutex(
    ACPI_MUTEX              Handle)
{
    AcpiOsSignalSemaphore(Handle, 1);
}
#endif


/******************************************************************************
*
* FUNCTION:    AcpiOsInstallInterruptHandler
*
* PARAMETERS:  InterruptNumber     - Level handler should respond to.
*              ServiceRoutine      - Address of the ACPI interrupt handler
*              Context             - User context
*
* RETURN:      Handle to the newly installed handler.
*
* DESCRIPTION: Install an interrupt handler. Used to install the ACPI
*              OS-independent handler.
*
*****************************************************************************/

UINT32
AcpiOsInstallInterruptHandler(
    UINT32                  InterruptNumber,
    ACPI_OSD_HANDLER        ServiceRoutine,
    void                    *Context)
{

    LOG_INFO("AcpiOsInstallInterruptHandler called");
    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsRemoveInterruptHandler
*
* PARAMETERS:  Handle              - Returned when handler installed
*
* RETURN:      Status
*
* DESCRIPTION: Uninstalls an interrupt handler.
*
*****************************************************************************/

ACPI_STATUS
AcpiOsRemoveInterruptHandler(
    UINT32                  InterruptNumber,
    ACPI_OSD_HANDLER        ServiceRoutine)
{

    LOG_INFO("AcpiOsRemoveInterruptHandler called");
    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsStall
*
* PARAMETERS:  Microseconds        - Time to stall
*
* RETURN:      None. Blocks until stall is completed.
*
* DESCRIPTION: Sleep at microsecond granularity
*
*****************************************************************************/

void
AcpiOsStall(
    UINT32                  Microseconds)
{
    LOG_INFO("AcpiOsStall called");
}


/******************************************************************************
*
* FUNCTION:    AcpiOsSleep
*
* PARAMETERS:  Milliseconds        - Time to sleep
*
* RETURN:      None. Blocks until sleep is completed.
*
* DESCRIPTION: Sleep at millisecond granularity
*
*****************************************************************************/

void
AcpiOsSleep(
    UINT64                  Milliseconds)
{
    LOG_INFO("AcpiOsSleep called");

    int i, j;

    for (i = 0; i < Milliseconds; i++)
        for (j = 0; j < 100; j++)
            j = j;
}


/******************************************************************************
*
* FUNCTION:    AcpiOsReadPciConfiguration
*
* PARAMETERS:  PciId               - Seg/Bus/Dev
*              Register            - Device Register
*              Value               - Buffer where value is placed
*              Width               - Number of bits
*
* RETURN:      Status
*
* DESCRIPTION: Read data from PCI configuration space
*
*****************************************************************************/

ACPI_STATUS
AcpiOsReadPciConfiguration(
    ACPI_PCI_ID             *PciId,
    UINT32                  Register,
    UINT64                  *Value,
    UINT32                  Width)
{

    LOG_INFO("AcpiOsReadPciConfiguration called");
    *Value = 0;
    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsWritePciConfiguration
*
* PARAMETERS:  PciId               - Seg/Bus/Dev
*              Register            - Device Register
*              Value               - Value to be written
*              Width               - Number of bits
*
* RETURN:      Status
*
* DESCRIPTION: Write data to PCI configuration space
*
*****************************************************************************/

ACPI_STATUS
AcpiOsWritePciConfiguration(
    ACPI_PCI_ID             *PciId,
    UINT32                  Register,
    UINT64                  Value,
    UINT32                  Width)
{

    LOG_INFO("AcpiOsWritePciConfiguration called");
    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsReadPort
*
* PARAMETERS:  Address             - Address of I/O port/register to read
*              Value               - Where value is placed
*              Width               - Number of bits
*
* RETURN:      Value read from port
*
* DESCRIPTION: Read data from an I/O port or register
*
*****************************************************************************/

ACPI_STATUS
AcpiOsReadPort(
    ACPI_IO_ADDRESS         Address,
    UINT32                  *Value,
    UINT32                  Width)
{
    LOG_INFO("AcpiOsReadPort called");
    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsWritePort
*
* PARAMETERS:  Address             - Address of I/O port/register to write
*              Value               - Value to write
*              Width               - Number of bits
*
* RETURN:      None
*
* DESCRIPTION: Write data to an I/O port or register
*
*****************************************************************************/

ACPI_STATUS
AcpiOsWritePort(
    ACPI_IO_ADDRESS         Address,
    UINT32                  Value,
    UINT32                  Width)
{
    LOG_INFO("AcpiOsWritePort called");
    return (AE_BAD_PARAMETER);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsReadMemory
*
* PARAMETERS:  Address             - Physical Memory Address to read
*              Value               - Where value is placed
*              Width               - Number of bits (8,16,32, or 64)
*
* RETURN:      Value read from physical memory address. Always returned
*              as a 64-bit integer, regardless of the read width.
*
* DESCRIPTION: Read data from a physical memory address
*
*****************************************************************************/

ACPI_STATUS
AcpiOsReadMemory(
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  *Value,
    UINT32                  Width)
{
    LOG_INFO("AcpiOsReadMemory called");

    switch (Width)
    {
    case 8:
    case 16:
    case 32:
    case 64:

        *Value = 0;
        break;

    default:

        return (AE_BAD_PARAMETER);
        break;
    }

    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsWriteMemory
*
* PARAMETERS:  Address             - Physical Memory Address to write
*              Value               - Value to write
*              Width               - Number of bits (8,16,32, or 64)
*
* RETURN:      None
*
* DESCRIPTION: Write data to a physical memory address
*
*****************************************************************************/

ACPI_STATUS
AcpiOsWriteMemory(
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  Value,
    UINT32                  Width)
{
    LOG_INFO("AcpiOsWriteMemory called");

    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    AcpiOsSignal
*
* PARAMETERS:  Function            - ACPICA signal function code
*              Info                - Pointer to function-dependent structure
*
* RETURN:      Status
*
* DESCRIPTION: Miscellaneous functions. Example implementation only.
*
*****************************************************************************/

ACPI_STATUS
AcpiOsSignal(
    UINT32                  Function,
    void                    *Info)
{
    LOG_INFO("AcpiOsSignal called");

    switch (Function)
    {
    case ACPI_SIGNAL_FATAL:

        break;

    case ACPI_SIGNAL_BREAKPOINT:

        break;

    default:

        break;
    }

    return (AE_OK);
}


/******************************************************************************
*
* FUNCTION:    Local cache interfaces
*
* DESCRIPTION: Implements cache interfaces via malloc/free for testing
*              purposes only.
*
*****************************************************************************/

#ifndef ACPI_USE_LOCAL_CACHE

ACPI_STATUS
AcpiOsCreateCache(
    char                    *CacheName,
    UINT16                  ObjectSize,
    UINT16                  MaxDepth,
    ACPI_CACHE_T            **ReturnCache)
{
    LOG_INFO("AcpiOsCreateCache called");
    LOG_INFO("ObjectSize is %X", ObjectSize);
    *ReturnCache = (ACPI_CACHE_T *)AcpiOsAllocate(ObjectSize);
    return (AE_OK);
}

ACPI_STATUS
AcpiOsDeleteCache(
    ACPI_CACHE_T            *Cache)
{
    LOG_INFO("AcpiOsDeleteCache called");
    return (AE_OK);
}

ACPI_STATUS
AcpiOsPurgeCache(
    ACPI_CACHE_T            *Cache)
{
    LOG_INFO("AcpiOsPurgeCache called");
    return (AE_OK);
}

void *
AcpiOsAcquireObject(
    ACPI_CACHE_T            *Cache)
{
    LOG_INFO("AcpiOsAcquireObject called");
    AquireLock();
    LOG_INFO("Cache is %X", *(WORD*)Cache);
    return AcpiOsAllocate(*(WORD*)Cache);
}

ACPI_STATUS
AcpiOsReleaseObject(
    ACPI_CACHE_T            *Cache,
    void                    *Object)
{
    LOG_INFO("AcpiOsReleaseObject called");
    ReleaseLock();
    return (AE_OK);
}

#endif /* ACPI_USE_LOCAL_CACHE */


/* Optional multi-thread support */

#ifndef ACPI_SINGLE_THREADED
/******************************************************************************
*
* FUNCTION:    AcpiOsGetThreadId
*
* PARAMETERS:  None
*
* RETURN:      Id of the running thread
*
* DESCRIPTION: Get the Id of the current (running) thread
*
*****************************************************************************/

ACPI_THREAD_ID
AcpiOsGetThreadId(
    void)
{
    LOG_INFO("AcpiOsGetThreadId called");
    return 0;
}


/******************************************************************************
*
* FUNCTION:    AcpiOsExecute
*
* PARAMETERS:  Type                - Type of execution
*              Function            - Address of the function to execute
*              Context             - Passed as a parameter to the function
*
* RETURN:      Status
*
* DESCRIPTION: Execute a new thread
*
*****************************************************************************/

ACPI_STATUS
AcpiOsExecute(
    ACPI_EXECUTE_TYPE       Type,
    ACPI_OSD_EXEC_CALLBACK  Function,
    void                    *Context)
{
    LOG_INFO("AcpiOsExecute called");
    return (0);
}

#else /* ACPI_SINGLE_THREADED */
ACPI_THREAD_ID
AcpiOsGetThreadId(
    void)
{
    return (1);
}

ACPI_STATUS
AcpiOsExecute(
    ACPI_EXECUTE_TYPE       Type,
    ACPI_OSD_EXEC_CALLBACK  Function,
    void                    *Context)
{

    Function(Context);
    return (AE_OK);
}

#endif /* ACPI_SINGLE_THREADED */


/******************************************************************************
*
* FUNCTION:    AcpiOsWaitEventsComplete
*
* PARAMETERS:  None
*
* RETURN:      None
*
* DESCRIPTION: Wait for all asynchronous events to complete. This
*              implementation does nothing.
*
*****************************************************************************/

void
AcpiOsWaitEventsComplete(
    void)
{
    LOG_INFO("AcpiOsWaitEventsComplete called");

    return;
}