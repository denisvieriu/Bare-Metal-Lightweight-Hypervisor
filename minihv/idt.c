#pragma once

#include "minihv.h"
#include "idt.h"
#include "gdt.h"
#include "print.h"
#include "string.h"
#include "local_apic.h"
#include "dbglog.h"
#include "memzero.h"
#include "idt_handlers.h"
#include "sstring.h"
#include "string.h"


extern VOID DivideError();
extern VOID DebugException();
extern VOID NMIInterrupt();
extern VOID BreakpointException();
extern VOID OverflowException();
extern VOID BoundRangeExceededException();
extern VOID InvalidOpcode();
extern VOID DeviceNotAvailable();
extern VOID DoubleFault();
extern VOID CoprocessorSegmentOverrun();
extern VOID InvalidTSS();
extern VOID SegmentNotPresent();
extern VOID StackFault();
extern VOID GeneralProtection();
extern VOID PageFault();
extern VOID FloatingPointX87Error();
extern VOID AlignmentCheck();
extern VOID MachineCheck();
extern VOID FloatingPointSIMD();
extern VOID VirtualizationException();

#define LONG_MODE_INTERRUPT_GATE 0x0E

static IDT_PTR m_idtPtr;

static IDT_ENTRY m_idt[256];

void
IdtSetGate(
    BYTE    IdtIndex,
    QWORD   Offset,
    WORD    SegmentSelector,
    BYTE    Ist,
    BYTE    Type
)
{
    memzero((PBYTE)&m_idt[IdtIndex], sizeof(IDT_ENTRY));

    m_idt[IdtIndex].IdtEntryLow.SegmentSelector = SegmentSelector;
    m_idt[IdtIndex].IdtEntryLow.OffsetLow = Offset & 0xFFFF;
    m_idt[IdtIndex].IdtEntryLow.OffsetMiddle = (Offset >> 16) & 0xFFFF;
    m_idt[IdtIndex].IdtEntryHigh.OffsetHigh = (Offset >> 32) & 0xFFFFFFFF;

    m_idt[IdtIndex].IdtEntryLow.Type = Type;
    m_idt[IdtIndex].IdtEntryLow.Ist = Ist;
    m_idt[IdtIndex].IdtEntryLow.P = 1;
}

static
void
IdtInstall(
    void
)
{
    m_idtPtr.limit = sizeof(m_idt) - 1;
    m_idtPtr.base = (QWORD)m_idt;
}

void
IdtInit(
    void
)
{
    LOG_ERROR("%X", (QWORD)DivideError);
    IdtSetGate(ExceptionDivideError, (QWORD)DivideError, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionInvalidOpcode, (QWORD)InvalidOpcode, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
  /*  IdtSetGate(ExceptionGeneralProtection, (QWORD)GeneralProtection, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionDoubleFault, (QWORD)DoubleFault, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionNMI, (QWORD)NMIInterrupt, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionPageFault, (QWORD)PageFault, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionDebugException, (QWORD)DebugException, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionBreakpoint, (QWORD)BreakpointException, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionOverflow, (QWORD)OverflowException, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionBoundRange, (QWORD)BoundRangeExceededException, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionInvalidTSS, (QWORD)InvalidTSS, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionSegmentNotPresent, (QWORD)SegmentNotPresent, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionStackFault, (QWORD)StackFault, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionGeneralProtection, (QWORD)GeneralProtection, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionVirtualizationException, (QWORD)VirtualizationException, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionSIMDFpuException, (QWORD)FloatingPointSIMD, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionX87FpuException, (QWORD)FloatingPointX87Error, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionAlignmentCheck, (QWORD)AlignmentCheck, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionDeviceNotAvailable, (QWORD)DeviceNotAvailable, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);
    IdtSetGate(ExceptionCoprocOverrun, (QWORD)CoprocessorSegmentOverrun, GDT_CODE64_SEL, CURRENT_STACK_IST_INDEX, LONG_MODE_INTERRUPT_GATE);*/

    IdtInstall();

    IdtLoad();


}

void
IdtLoad(
    void
)
{
    __lidt(&m_idtPtr);
}

QWORD
IdtGetIdtrBaseAddress(
    VOID
)
{
    return m_idtPtr.base;
}