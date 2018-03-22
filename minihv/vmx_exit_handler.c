#include "vmx_exit_handler.h"
#include "vmx.h"
#include "cpu.h"
#include "dbglog.h"
#include "pcci.h"
#include "map.h"

NTSTATUS
VmxExitHandlerDefault(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return VmxOff();
}

//
// Exit handler functions ..
//
NTSTATUS
VmxExitHandlerExceptionNmi(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerExternalInterrupt(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerTripleFault(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerInitSignal(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerStartupIpiSipi(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerIOSystemMananagementInterrupt(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}


NTSTATUS
VmxExitHandlerOtherSmi(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerInterruptWindow(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerNmiWindow(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerTaskSwitch(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    LOG("Exit reason: VMX_BASIC_EXIT_REASON_TASK_SWITCH");

    QWORD exitQualificationTS;

    // Selector of task-state segment (TSS) to which guest attempted to switch
    WORD tss;

    // Source of task switch initiation (can be one of the following )
    //  -> 0: CALL instruction
    //  -> 1: IRET instruction
    //  -> 2: JMP instruction
    //  -> 3: Task gate in IDT
    BYTE sourceOfTaskSwitch;


    VmxRead(NULL, VMCS_EXIT_QUALIFICATION_ENCODING, &exitQualificationTS);

    // bits 15:09
    tss = exitQualificationTS & VMX_BASIC_EXIT_REASON_MASK;
    sourceOfTaskSwitch = GET_BITS(exitQualificationTS, 30, SOURCE_OF_TASK_INIT);
    LOG_INFO("VM EXIT TASK SWITCH: [%d]", sourceOfTaskSwitch);
    switch (sourceOfTaskSwitch)
    {
    case BIT_CALL_INSTR:    LOG("CALL");                         break;
    case BIT_IRET_INSTR:    LOG("IRET");                         break;
    case BIT_JMP_INSTR:     LOG("JMP");                          break;
    case BIT_TASK_GATE_IDT: LOG("TASK GATE");                    break;
    default:                LOG_ERROR("Shouldn't reach here.."); break;
    }

    VmxOff();


    LOG_ERROR("Invalidating CR3");
    __writecr3(0);

    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerCpuid(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    DWORD cpuInfo[4];
    QWORD guestCr4;

    __cpuidex(cpuInfo, ProcessorState->RegisterValues[RegisterRax], ProcessorState->RegisterValues[RegisterRcx]);

    if ((DWORD)ProcessorState->RegisterValues[RegisterRax] == 0x01)
    {
        // check if guest set the OXSAVE bit, since CPUIDECX returns the OSXSAVE bit
        VmxRead(NULL, VMCS_GUEST_CR4_ENCODING, &guestCr4);
        if (GET_BITS(guestCr4, CR4_OSXSAVE_BIT_INDEX, 1))
        {
            SET_BITT(cpuInfo[CPUID_RCX], CPUID_01H_RCX_OSXSAVE_BIT_INDEX);
        }
        else
        {
            CLEAR_BITT(cpuInfo[CPUID_RCX], CPUID_01H_RCX_OSXSAVE_BIT_INDEX);
        }
    }



    //
    // Check if this was CPUID 1h, which is the features request.
    //
    if (ProcessorState->RegisterValues[RegisterRax] == 1)
    {
       //
       //
        
        cpuInfo[2] |= 0x80000000;
    }
    else if (ProcessorState->RegisterValues[RegisterRax] == 0x40000001)
    {
        //
        // Return 
        //
        cpuInfo[0] = 'abcd';
    }


    //
    // Copy the values from the logical processor registers into Virtual Processor GPRs
    //

    ProcessorState->RegisterValues[RegisterRax] = (QWORD)(cpuInfo[CPUID_RAX] & MAX_DWORD);
    ProcessorState->RegisterValues[RegisterRbx] = (QWORD)(cpuInfo[CPUID_RBX] & MAX_DWORD);
    ProcessorState->RegisterValues[RegisterRcx] = (QWORD)(cpuInfo[CPUID_RCX] & MAX_DWORD);
    ProcessorState->RegisterValues[RegisterRdx] = (QWORD)(cpuInfo[CPUID_RDX] & MAX_DWORD);


    VmxJumpToNextInstr();


    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerGetsec(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerHlt(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}


NTSTATUS
VmxExitHandlerInvd(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerInvlpg(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerRdpmc(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}


NTSTATUS
VmxExitHandlerRdtsc(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerRsm(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmcall(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmclear(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmlaunch(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmptrld(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}


NTSTATUS
VmxExitHandlerVmptrst(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmread(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmresume(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmxwrite(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmxoff(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmxon(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerCRAccess(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS VmxExitHandlerMovDr(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerIOInstruction(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);

    LOG("Exit reason: VMX_BASIC_EXIT_REASON_IO_INSTRUCTION");

    BYTE guestFunction = (ProcessorState->RegisterValues[RegisterRax] >> PCI_CONFIG_ADDRESS_FUNCTION_SHIFT) & PCI_CONFIG_ADDRESS_FUNCTION_MASK;
    BYTE guestDevice = (ProcessorState->RegisterValues[RegisterRax] >> PCI_CONFIG_ADDRESS_DEVICE_SHIFT) & PCI_CONFIG_ADDRESS_DEVICE_MASK;
    BYTE guestBus = (ProcessorState->RegisterValues[RegisterRax] >> PCI_CONFIG_ADDRESS_BUS_SHIFT) & PCI_CONFIG_ADDRESS_BUS_MASK;
    BYTE guestRegister = (ProcessorState->RegisterValues[RegisterRax] >> 2) & 0x3F;
    QWORD guestIoBitmapPa;
    PBYTE guestIoBitmapVa;

    if (ProcessorState->RegisterValues[RegisterRdx] == PCI_CONFIG_ADDRESS_PORT)
    {
        if (PciBDFEqualsReservedSerialPort(guestBus, guestDevice, guestFunction))
        {
            if (guestRegister == 0)
            {
                LOG("Guest tried to interogate PCI for our reserved serial port");
                VmxRead(NULL, VMCS_ADDRESS_OF_IO_BITMAP_A_FULL_ENCODING, &guestIoBitmapPa);
                guestIoBitmapVa = MmuMapToVa(guestIoBitmapPa, VMX_IO_BITMAP_SIZE, NULL, FALSE);
                VmxSetBitInBitmap(guestIoBitmapVa, PCI_CONFIG_ADDRESS_PORT, FALSE); // disable exits on CONFIG_ADDRESS port accesses
                VmxSetBitInBitmap(guestIoBitmapVa, PCI_CONFIG_DATA_PORT, TRUE); // enable exits on CONFIG_DATA port accesses
                MmuUnmapFromVa((QWORD)guestIoBitmapVa, VMX_IO_BITMAP_SIZE);
            }
        }
    }
    else if (ProcessorState->RegisterValues[RegisterRdx] == PCI_CONFIG_DATA_PORT)
    {
        LOG("Will send invalid vendor to guest");
        VmxRead(NULL, VMCS_ADDRESS_OF_IO_BITMAP_A_FULL_ENCODING, &guestIoBitmapPa);
        guestIoBitmapVa = MmuMapToVa(guestIoBitmapPa, VMX_IO_BITMAP_SIZE, NULL, FALSE);
        ProcessorState->RegisterValues[RegisterRax] = MAX_DWORD; // invalid vendor id
        VmxSetBitInBitmap(guestIoBitmapVa, PCI_CONFIG_DATA_PORT, FALSE); // disable exits on CONFIG_DATA port accesses
		MmuUnmapFromVa((QWORD)guestIoBitmapVa, VMX_IO_BITMAP_SIZE);
    }

    VmxJumpToNextInstr();

    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerRdmsr(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerWrmsr(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmEntryFailInvGState(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmEntryFailMsrLoad(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerMwait(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerMonitorTrapFlag(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerMonitor(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerPause(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmEntryFailMachineCheckEvent(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerTrpBelowThreshold(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}
NTSTATUS
VmxExitHandlerApicAccess(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVirtualizedEoi(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerAccessGdtrToIdtr(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerAccessLdtrToTr(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerEptViolation(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerEptMisconfiguration(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerInvept(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerRdtscp(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}
NTSTATUS
VmxExitHandlerVmxPreemptionTimerExpired(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);

    NTSTATUS ntStatus;
    ntStatus = STATUS_SUCCESS;
    LOG("Exit reason: VMX_BASIC_EXIT_REASON_PREEMPTION_TIMER_EXPIRED");

    return ntStatus;
}

NTSTATUS
VmxExitHandlerInvvpid(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerWbinvd(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerXsetbv(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerApicWrite(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerRdrand(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerInvpcid(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerVmfunc(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerEncls(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerRdseed(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerPageModificationLogFull(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerXSaves(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

NTSTATUS
VmxExitHandlerXrstors(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    return STATUS_SUCCESS;
}

