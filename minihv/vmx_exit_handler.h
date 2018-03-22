#ifndef _VMX_EXIT_HANDLER_H
#define _VMX_EXIT_HANDLER_H

#include "minihv.h"
#include "cpu.h"


typedef
NTSTATUS
(*PFUNC_VmxExitHandler)(
    _In_ PPROCESSOR_STATE ProcessorState
    );


// Appendix C, VOLUME 3
NTSTATUS VmxExitHandlerDefault(_In_ PPROCESSOR_STATE ProcessorState);                           //

NTSTATUS VmxExitHandlerExceptionNmi(_In_ PPROCESSOR_STATE ProcessorState);                      // 0
NTSTATUS VmxExitHandlerExternalInterrupt(_In_ PPROCESSOR_STATE ProcessorState);                 // 1
NTSTATUS VmxExitHandlerTripleFault(_In_ PPROCESSOR_STATE ProcessorState);                       // 2
NTSTATUS VmxExitHandlerInitSignal(_In_ PPROCESSOR_STATE ProcessorState);                        // 3
NTSTATUS VmxExitHandlerStartupIpiSipi(_In_ PPROCESSOR_STATE ProcessorState);                    // 4
NTSTATUS VmxExitHandlerIOSystemMananagementInterrupt(_In_ PPROCESSOR_STATE ProcessorState);     // 5
NTSTATUS VmxExitHandlerOtherSmi(_In_ PPROCESSOR_STATE ProcessorState);                          // 6
NTSTATUS VmxExitHandlerInterruptWindow(_In_ PPROCESSOR_STATE ProcessorState);                   // 7
NTSTATUS VmxExitHandlerNmiWindow(_In_ PPROCESSOR_STATE ProcessorState);                         // 8
NTSTATUS VmxExitHandlerTaskSwitch(_In_ PPROCESSOR_STATE ProcessorState);                        // 9
NTSTATUS VmxExitHandlerCpuid(_In_ PPROCESSOR_STATE ProcessorState);                             // 10
NTSTATUS VmxExitHandlerGetsec(_In_ PPROCESSOR_STATE ProcessorState);                            // 11
NTSTATUS VmxExitHandlerHlt(_In_ PPROCESSOR_STATE ProcessorState);                               // 12
NTSTATUS VmxExitHandlerInvd(_In_ PPROCESSOR_STATE ProcessorState);                              // 13
NTSTATUS VmxExitHandlerInvlpg(_In_ PPROCESSOR_STATE ProcessorState);                            // 14
NTSTATUS VmxExitHandlerRdpmc(_In_ PPROCESSOR_STATE ProcessorState);                             // 15
NTSTATUS VmxExitHandlerRdtsc(_In_ PPROCESSOR_STATE ProcessorState);                             // 16
NTSTATUS VmxExitHandlerRsm(_In_ PPROCESSOR_STATE ProcessorState);                               // 17
NTSTATUS VmxExitHandlerVmcall(_In_ PPROCESSOR_STATE ProcessorState);                            // 18
NTSTATUS VmxExitHandlerVmclear(_In_ PPROCESSOR_STATE ProcessorState);                           // 19
NTSTATUS VmxExitHandlerVmlaunch(_In_ PPROCESSOR_STATE ProcessorState);                          // 20
NTSTATUS VmxExitHandlerVmptrld(_In_ PPROCESSOR_STATE ProcessorState);                           // 21
NTSTATUS VmxExitHandlerVmptrst(_In_ PPROCESSOR_STATE ProcessorState);                           // 22
NTSTATUS VmxExitHandlerVmread(_In_ PPROCESSOR_STATE ProcessorState);                            // 23
NTSTATUS VmxExitHandlerVmresume(_In_ PPROCESSOR_STATE ProcessorState);                          // 24
NTSTATUS VmxExitHandlerVmxwrite(_In_ PPROCESSOR_STATE ProcessorState);                          // 25
NTSTATUS VmxExitHandlerVmxoff(_In_ PPROCESSOR_STATE ProcessorState);                            // 26
NTSTATUS VmxExitHandlerVmxon(_In_ PPROCESSOR_STATE ProcessorState);                             // 27
NTSTATUS VmxExitHandlerCRAccess(_In_ PPROCESSOR_STATE ProcessorState);                          // 28
NTSTATUS VmxExitHandlerMovDr(_In_ PPROCESSOR_STATE ProcessorState);                             // 29
NTSTATUS VmxExitHandlerIOInstruction(_In_ PPROCESSOR_STATE ProcessorState);                     // 30
NTSTATUS VmxExitHandlerRdmsr(_In_ PPROCESSOR_STATE ProcessorState);                             // 31
NTSTATUS VmxExitHandlerWrmsr(_In_ PPROCESSOR_STATE ProcessorState);                             // 32
NTSTATUS VmxExitHandlerVmEntryFailInvGState(_In_ PPROCESSOR_STATE ProcessorState);              // 33
NTSTATUS VmxExitHandlerVmEntryFailMsrLoad(_In_ PPROCESSOR_STATE ProcessorState);                // 34
                                                                                                // Default
NTSTATUS VmxExitHandlerMwait(_In_ PPROCESSOR_STATE ProcessorState);                             // 36
NTSTATUS VmxExitHandlerMonitorTrapFlag(_In_ PPROCESSOR_STATE ProcessorState);                   // 37
                                                                                                // Default
NTSTATUS VmxExitHandlerMonitor(_In_ PPROCESSOR_STATE ProcessorState);                           // 39
NTSTATUS VmxExitHandlerPause(_In_ PPROCESSOR_STATE ProcessorState);                             // 40
NTSTATUS VmxExitHandlerVmEntryFailMachineCheckEvent(PPROCESSOR_STATE ProcessorState);           // 41
                                                                                                // Default
NTSTATUS VmxExitHandlerTrpBelowThreshold(PPROCESSOR_STATE ProcessorState);                      // 43
NTSTATUS VmxExitHandlerApicAccess(PPROCESSOR_STATE ProcessorState);                             // 44
NTSTATUS VmxExitHandlerVirtualizedEoi(PPROCESSOR_STATE ProcessorState);                         // 45
NTSTATUS VmxExitHandlerAccessGdtrToIdtr(PPROCESSOR_STATE ProcessorState);                       // 46
NTSTATUS VmxExitHandlerAccessLdtrToTr(PPROCESSOR_STATE ProcessorState);                         // 47
NTSTATUS VmxExitHandlerEptViolation(PPROCESSOR_STATE ProcessorState);                           // 48
NTSTATUS VmxExitHandlerEptMisconfiguration(PPROCESSOR_STATE ProcessorState);                    // 49
NTSTATUS VmxExitHandlerInvept(PPROCESSOR_STATE ProcessorState);                                 // 50
NTSTATUS VmxExitHandlerRdtscp(PPROCESSOR_STATE ProcessorState);                                 // 51
NTSTATUS VmxExitHandlerVmxPreemptionTimerExpired(PPROCESSOR_STATE ProcessorState);              // 52
NTSTATUS VmxExitHandlerInvvpid(PPROCESSOR_STATE ProcessorState);                                // 53
NTSTATUS VmxExitHandlerWbinvd(PPROCESSOR_STATE ProcessorState);                                 // 54
NTSTATUS VmxExitHandlerXsetbv(PPROCESSOR_STATE ProcessorState);                                 // 55
NTSTATUS VmxExitHandlerApicWrite(PPROCESSOR_STATE ProcessorState);                              // 56
NTSTATUS VmxExitHandlerRdrand(PPROCESSOR_STATE ProcessorState);                                 // 57
NTSTATUS VmxExitHandlerInvpcid(PPROCESSOR_STATE ProcessorState);                                // 58
NTSTATUS VmxExitHandlerVmfunc(PPROCESSOR_STATE ProcessorState);                                 // 59
NTSTATUS VmxExitHandlerEncls(PPROCESSOR_STATE ProcessorState);                                  // 60
NTSTATUS VmxExitHandlerRdseed(PPROCESSOR_STATE ProcessorState);                                 // 61
NTSTATUS VmxExitHandlerPageModificationLogFull(PPROCESSOR_STATE ProcessorState);                // 62
NTSTATUS VmxExitHandlerXSaves(PPROCESSOR_STATE ProcessorState);                                 // 63
NTSTATUS VmxExitHandlerXrstors(PPROCESSOR_STATE ProcessorState);                                // 64

//NTSTATUS VmxExitHanlder

#endif
