%include "_utils.mac"

extern IsrCommonHandler
extern RestoreRegisters

%macro call_handler 2
%if %2 == 0
    sub             rsp,        0x8
%endif

    sub             rsp,        0x10
    mov             [rsp],      DWORD %1      ; interrupt index
    mov             [rsp+8],    DWORD %2      ; if 1 => Error code on stack

    jmp             PreIsrHandler
%endmacro

%macro call_func_64    1-5
; stack will be either aligned to 0x10 or to 0x8
; bytes after the PUSH

; NOTE: WE NEED THE PUSH BEFORE THE AND
; TO BE SURE WE DON'T 'DEALLOCATE' ELEMENTS FROM THE
; STACK

; Example:
; RSP <- aligned at 0x10
; We to AND without sub => still aligned to 0x10
; We then add 0x8 bytes to RSP (which we didn't allocate)
; This means 8 bytes are lost from the stack
    push    rbx

    mov     rbx, 0xF
    and     rbx, rsp

; align stack to 0x10
    sub     rsp, rbx

%if %0 > 1
    mov     rcx, %2
%endif

%if %0 > 2
    mov     rdx, %3
%endif

%if %0 > 3
    mov     r8, %4
%endif

%if %0 > 4
    mov     r9, %5
%endif

    sub     rsp, 0x20
; the stack must be aligned at 16 bytes here,
; not after the call
    call %1

    add     rsp, 0x20

    add     rsp, rbx
    pop     rbx
%endmacro

; exceptions
global DivideError
global DebugException
global NMIInterrupt
global BreakpointException
global OverflowException
global BoundRangeExceededException
global InvalidOpcode
global DeviceNotAvailable
global DoubleFault
global CoprocessorSegmentOverrun
global InvalidTSS
global SegmentNotPresent
global StackFault
global GeneralProtection
global PageFault
global FloatingPointX87Error
global AlignmentCheck
global MachineCheck
global FloatingPointSIMD
global VirtualizationException

global GenerateDivideException

align 0x10, db 0
[bits 64]
GenerateDivideException:
	xchg bx, bx ; bochs magic breakpoint
	xor rax, rax
	div rax
	ret

; Interrupt 0 - Divide Error Exception (#DE)
align 0x10, db 0
[bits 64]
DivideError:
    call_handler    0, 0

; Interrupt 1 - Debug Exception (#DB)
align 0x10, db 0
[bits 64]
DebugException:
    call_handler    1, 0

; Interrupt 2 - NMI Interrupt
align 0x10, db 0
[bits 64]
NMIInterrupt:
    call_handler    2, 0

; Interrupt 3 - Breakpoint Exception (#BP)
align 0x10, db 0
[bits 64]
BreakpointException:
    call_handler    3, 0

; Interrupt 4 - Overflow Exception (#OF)
align 0x10, db 0
[bits 64]
OverflowException:
    call_handler    4, 0

; Interrupt 5 - BOUND Range Exceeded Exception (#BR)
align 0x10, db 0
[bits 64]
BoundRangeExceededException:
    call_handler    5, 0

; Interrupt 6 - Invalid Opcode Exception (#UD)
align 0x10, db 0
[bits 64]
InvalidOpcode:
    call_handler    6, 0

; Interrupt 7 - Device Not Available Exception (#NM)
align 0x10, db 0
[bits 64]
DeviceNotAvailable:
    call_handler    7, 0

; Interrupt 8 - Double Fault Exception (#DF)
align 0x10, db 0
[bits 64]
DoubleFault:
    call_handler    8, 1

; Interrupt 9 - Coprocessor Segment Overrun
align 0x10, db 0
[bits 64]
CoprocessorSegmentOverrun:
    call_handler    9, 0

; Interrupt 10 - Invalid TSS Exception (#TS)
align 0x10, db 0
[bits 64]
InvalidTSS:
    call_handler    10, 1

; Interrupt 11 - Segment Not Present (#NP)
align 0x10, db 0
[bits 64]
SegmentNotPresent:
    call_handler    11, 1

; Interrupt 12 - Stack Fault (#SS)
align 0x10, db 0
[bits 64]
StackFault:
    call_handler    12, 1

; Interrupt 13 - General Protection Exception (#GP)
align 0x10, db 0
[bits 64]
GeneralProtection:
    call_handler    13, 1

; Interrupt 14 - Page Fault Exception (#PF)
align 0x10, db 0
[bits 64]
PageFault:
    call_handler    14, 1

; Interrupt 16 - x87 FPU Floating-Point Error (#MF)
align 0x10, db 0
[bits 64]
FloatingPointX87Error:
    call_handler    16, 0

; Interrupt 17 - Alignment Check Exception (#AC)
align 0x10, db 0
[bits 64]
AlignmentCheck:
    call_handler    17, 1

; Interrupt 18 - Machine-Check Exception (#MC)
align 0x10, db 0
[bits 64]
MachineCheck:
    call_handler    18, 0

; Interrupt 19 - SIMD Floating-Point Exception (#XM)
align 0x10, db 0
[bits 64]
FloatingPointSIMD:
    call_handler    19, 0

; Interrupt 20 - Virtualization Exception (#VE)
align 0x10, db 0
[bits 64]
VirtualizationException:
    call_handler    20, 0

align 0x10, db 0
[bits 64]
PreIsrHandler:
    save_proc_state

    ; 4th argument - pointer to processor state
    mov             r9, rsp

    ; 3rd argument - BOOLEAN, error code available
    ; we only read a DWORD because that's what we wrote
    ; because operand size is 32-bit on x64 upper half of
    ; register will be zeroed
    mov             r8d, [rsp + PROCESSOR_STATE_size + 8]

    ; 2nd argument - pointer to interrupt stack
    ; need to add an additional +0x10 because we use 2 bytes
    ; from the stack for the arguments received
    lea             rdx, [rsp + PROCESSOR_STATE_size + 0x10]

    ; 1st argument - interrupt index
    mov             ecx, [rsp + PROCESSOR_STATE_size]

    call_func_64    IsrCommonHandler, rcx, rdx, r8, r9

    mov             rcx, rsp
    call            RestoreRegisters

    ; restore stack
    add             rsp, PROCESSOR_STATE_size

    ; we also have the 2 arguments passed to us on the stack
    ; and the error code
    add             rsp, 0x18

    iretq