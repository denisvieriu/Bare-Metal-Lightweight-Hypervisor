
#include "idt_handlers.h"
#include "cpu.h"
#include "dbglog.h"


void
IsrCommonHandler(
    BYTE                                 InterruptIndex,
    PINTERRUPT_STACK_COMPLETE            StackPointer,
    BOOLEAN                              ErrorCodeAvailable,
    PROCESSOR_STATE*                     ProcessorState
)
{
    UNREFERENCED_PARAMETER(InterruptIndex);
    UNREFERENCED_PARAMETER(StackPointer);
    UNREFERENCED_PARAMETER(ErrorCodeAvailable);
    UNREFERENCED_PARAMETER(ProcessorState);

    LOG("InterruptIndex %d", InterruptIndex);
    LOG("Rip %X", StackPointer->Registers.Rip);

    if (InterruptIndex == ExceptionDivideError)
    {
        LOG("Awesome! We got a division by zero!");
    }
    else if (InterruptIndex == ExceptionInvalidOpcode)
    {
        LOG("Awesome! We got an invalid opcode!");
    }


}