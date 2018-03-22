#include "cpu.h"
#include "vmx_exit_handler.h"
#include "vmx.h"
#include "dbglog.h"
#include "vmx_common.h"

extern void RestoreGuestRegistersAndResume(PPROCESSOR_STATE);


typedef struct _EXIT_HANDLER
{
    PFUNC_VmxExitHandler  Func;
    BOOLEAN               Status;
}EXIT_HANDLER, *PEXIT_HANDLER;

static
EXIT_HANDLER
gExitHandlerStruct[] =
{
    { (PFUNC_VmxExitHandler)VmxExitHandlerExceptionNmi,                     FALSE }, // 0
    { (PFUNC_VmxExitHandler)VmxExitHandlerExternalInterrupt,                FALSE }, // 1
    { (PFUNC_VmxExitHandler)VmxExitHandlerTripleFault,                      FALSE }, // 2
    { (PFUNC_VmxExitHandler)VmxExitHandlerInitSignal,                       FALSE }, // 3
    { (PFUNC_VmxExitHandler)VmxExitHandlerStartupIpiSipi,                   FALSE }, // 4
    { (PFUNC_VmxExitHandler)VmxExitHandlerIOSystemMananagementInterrupt,    FALSE }, // 5
    { (PFUNC_VmxExitHandler)VmxExitHandlerOtherSmi,                         FALSE }, // 6
    { (PFUNC_VmxExitHandler)VmxExitHandlerInterruptWindow,                  FALSE }, // 7
    { (PFUNC_VmxExitHandler)VmxExitHandlerNmiWindow,                        FALSE }, // 8
    { (PFUNC_VmxExitHandler)VmxExitHandlerTaskSwitch,                       TRUE }, // 9
    { (PFUNC_VmxExitHandler)VmxExitHandlerCpuid,                            TRUE }, // 10
    { (PFUNC_VmxExitHandler)VmxExitHandlerGetsec,                           FALSE }, // 11
    { (PFUNC_VmxExitHandler)VmxExitHandlerHlt,                              FALSE }, // 12
    { (PFUNC_VmxExitHandler)VmxExitHandlerInvd,                             FALSE }, // 13
    { (PFUNC_VmxExitHandler)VmxExitHandlerInvlpg,                           FALSE }, // 14
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdpmc,                            FALSE }, // 15
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdtsc,                            FALSE }, // 16
    { (PFUNC_VmxExitHandler)VmxExitHandlerRsm,                              FALSE }, // 17
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmcall,                           TRUE }, // 18
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmclear,                          FALSE }, // 19
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmlaunch,                         FALSE }, // 20
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmptrld,                          FALSE }, // 21
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmptrst,                          FALSE }, // 22
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmread,                           FALSE }, // 23
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmresume,                         FALSE }, // 24
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmxwrite,                         FALSE }, // 25
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmxoff,                           FALSE }, // 26
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmxon,                            FALSE }, // 27
    { (PFUNC_VmxExitHandler)VmxExitHandlerCRAccess,                         FALSE }, // 28
    { (PFUNC_VmxExitHandler)VmxExitHandlerMovDr,                            FALSE }, // 29
    { (PFUNC_VmxExitHandler)VmxExitHandlerIOInstruction,                    TRUE }, // 30
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdmsr,                            FALSE }, // 31
    { (PFUNC_VmxExitHandler)VmxExitHandlerWrmsr,                            FALSE }, // 32
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmEntryFailInvGState,             FALSE }, // 33
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmEntryFailMsrLoad,               FALSE }, // 34
    { (PFUNC_VmxExitHandler)VmxExitHandlerDefault,                          FALSE }, // DEFAULT // 35
    { (PFUNC_VmxExitHandler)VmxExitHandlerMwait,                            FALSE }, // 36
    { (PFUNC_VmxExitHandler)VmxExitHandlerMonitorTrapFlag,                  FALSE }, // 37
    { (PFUNC_VmxExitHandler)VmxExitHandlerDefault,                          FALSE }, // DEFAULT // 38
    { (PFUNC_VmxExitHandler)VmxExitHandlerMonitor,                          FALSE }, // 39
    { (PFUNC_VmxExitHandler)VmxExitHandlerPause,                            FALSE }, // 40
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmEntryFailMachineCheckEvent,     FALSE }, // 41
    { (PFUNC_VmxExitHandler)VmxExitHandlerDefault,                          FALSE }, // DEFAULT // 42
    { (PFUNC_VmxExitHandler)VmxExitHandlerTrpBelowThreshold,                FALSE }, // 43
    { (PFUNC_VmxExitHandler)VmxExitHandlerApicAccess,                       FALSE }, // 44
    { (PFUNC_VmxExitHandler)VmxExitHandlerVirtualizedEoi,                   FALSE }, // 45
    { (PFUNC_VmxExitHandler)VmxExitHandlerAccessGdtrToIdtr,                 FALSE }, // 46
    { (PFUNC_VmxExitHandler)VmxExitHandlerAccessLdtrToTr,                   FALSE }, // 47
    { (PFUNC_VmxExitHandler)VmxExitHandlerEptViolation,                     TRUE }, // 48
    { (PFUNC_VmxExitHandler)VmxExitHandlerEptMisconfiguration,              FALSE }, // 49
    { (PFUNC_VmxExitHandler)VmxExitHandlerInvept,                           FALSE }, // 50
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdtscp,                           FALSE }, // 51
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmxPreemptionTimerExpired,        TRUE }, // 52
    { (PFUNC_VmxExitHandler)VmxExitHandlerInvvpid,                          FALSE }, // 53
    { (PFUNC_VmxExitHandler)VmxExitHandlerWbinvd,                           FALSE }, // 54
    { (PFUNC_VmxExitHandler)VmxExitHandlerXsetbv,                           TRUE }, // 55
    { (PFUNC_VmxExitHandler)VmxExitHandlerApicWrite,                        FALSE }, // 56
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdrand,                           FALSE }, // 57
    { (PFUNC_VmxExitHandler)VmxExitHandlerInvpcid,                          FALSE }, // 58
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmfunc,                           FALSE }, // 59
    { (PFUNC_VmxExitHandler)VmxExitHandlerEncls,                            FALSE }, // 60
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdseed,                           FALSE }, // 61
    { (PFUNC_VmxExitHandler)VmxExitHandlerPageModificationLogFull,          FALSE }, // 62
    { (PFUNC_VmxExitHandler)VmxExitHandlerXSaves,                           FALSE }, // 63
    { (PFUNC_VmxExitHandler)VmxExitHandlerXrstors,                          FALSE }, // 64
    { (PFUNC_VmxExitHandler)VmxExitHandlerDefault,                          FALSE }, // DEFAULT ^65
};

VOID
MhvVmxExitHandler(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    //
    //  Default Vmx Exit Handler
    //
    WORD basicExitReason;
    DWORD exitReason;
    NTSTATUS ntStatus;

    ntStatus = STATUS_SUCCESS;
    VmxRead(NULL, VMCS_EXIT_REASON_ENCODING, &exitReason);
    basicExitReason = exitReason & VMX_BASIC_EXIT_REASON_MASK;

    if (gExitHandlerStruct[ProcessorState->ExitReason].Status != FALSE)
    {
        //
        // Calling the handler for the given exit reason
        //

        ntStatus = gExitHandlerStruct[basicExitReason].Func(ProcessorState);
        if (!NT_SUCCESS(ntStatus))
        {
#            //DEF_PRINTF("VmxExitHandler failed, exit code: [0x%08x]", basicExitReason);
        }

        RestoreGuestRegistersAndResume(ProcessorState);
    }

    VmxJumpToNextInstr();
}

DECLSPEC_NORETURN
VOID
MhvVmxEntryHandler(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{

    ProcessorState->GuestContext.GuestRflags = MhvVmxRead(VMCS_GUEST_RFLAGS_ENCODING);
    ProcessorState->GuestContext.GuestRsp    = MhvVmxRead(VMCS_GUEST_RSP_ENCODING);
    ProcessorState->GuestContext.GuestRip    = MhvVmxRead(VMCS_GUEST_RIP_ENCODING);
    ProcessorState->ExitReason               = MhvVmxRead(VMCS_EXIT_REASON_ENCODING) & 0xFFFF;
    
    //
    // Call the generic handler
    //
    MhvVmxExitHandler(ProcessorState);
}

