%include "_utils.mac"

global RestoreRegisters

align 0x10, db 0
[bits 64]
; void __cdecl* RestoreRegisters( PROCESSOR_STATE* ProcessorState )
RestoreRegisters:
    mov     Rax,    [rcx+PROCESSOR_STATE.Rax]
    mov     Rdx,    [rcx+PROCESSOR_STATE.Rdx]
    mov     Rbx,    [rcx+PROCESSOR_STATE.Rbx]
    mov     Rbp,    [rcx+PROCESSOR_STATE.Rbp]
    mov     Rsi,    [rcx+PROCESSOR_STATE.Rsi]
    mov     Rdi,    [rcx+PROCESSOR_STATE.Rdi]

    mov     R8,     [rcx+PROCESSOR_STATE.R8]
    mov     R9,     [rcx+PROCESSOR_STATE.R9]
    mov     R10,    [rcx+PROCESSOR_STATE.R10]
    mov     R11,    [rcx+PROCESSOR_STATE.R11]
    mov     R12,    [rcx+PROCESSOR_STATE.R12]
    mov     R13,    [rcx+PROCESSOR_STATE.R13]
    mov     R14,    [rcx+PROCESSOR_STATE.R14]
    mov     R15,    [rcx+PROCESSOR_STATE.R15]

    mov     Rcx,    [rcx+PROCESSOR_STATE.Rcx]

    ret
