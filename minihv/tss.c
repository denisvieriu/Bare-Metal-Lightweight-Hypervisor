#include "tss.h"
#include "ntstatus.h"
#include "map.h"

NTSTATUS
TssInstallStack(
    PTSS        Tss,
    BYTE        StackIndex
)
{
    PVOID istStack;

    istStack = MmuAllocVa(CPU_STACK_SIZE);
    istStack = (PBYTE)istStack + CPU_STACK_SIZE;

    Tss->IST[StackIndex - 1] = (QWORD)istStack;

    return STATUS_SUCCESS;
}

VOID
TssInstallStacks(
    PTSS Tss
)
{
    TssInstallStack(Tss, 0);
    TssInstallStack(Tss, 1);
}