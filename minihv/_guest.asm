
%include "_utils.mac"

extern VmxVmExitHandler
extern RestoreRegisters
extern VmxBasicExitReasonVmcall

global _start_guest_16
global _end_guest_16
global SaveGuestRegisters
global RestoreGuestRegistersAndResume
global _start_hooked_int15h
global _end_hooked_int15h

place times 0x18 db 0

[bits 16]


; STARTUP:
; 1. LegacyBios checks for signature 0xAA55
; 2. LegacyBios passes execution to guest in real mode from 0x7c00
; 3. 
_start_guest_16:

	xchg bx, bx ; BOCHS magic breakpoint

	mov ax, 0xB800
	mov gs, ax

	mov byte [gs:0], '!'
	mov byte [gs:1], 0x0F

    mov byte [gs:2], 'L'
	mov byte [gs:3], 0x0F

    mov byte [gs:4], 'O'
	mov byte [gs:5], 0x0F

    mov byte [gs:6], 'A'
	mov byte [gs:7], 0x0F

    mov byte [gs:8], 'D'
	mov byte [gs:9], 0x0F

    mov byte [gs:10], 'I'
	mov byte [gs:11], 0x0F

    mov byte [gs:12], 'N'
	mov byte [gs:13], 0x0F

    mov byte [gs:14], 'G'
	mov byte [gs:15], 0x0F

	xor ax, ax
	mov es, ax

	mov ah, 0x02    ; load sector service
	mov al, 0x01    ; drive 1
	mov ch, 0x00    ; track/cylinder
	mov cl, 0x01    ; sector number
	mov dh, 0x00    ; head number
	mov dl, 0x80    ; drive 0
	mov bx, 0x7C00  ; store here the mbr

	int 13h

	mov ax, 0xB800
	mov gs, ax

	jc error ; check if int13h failed

	;mov byte [gs:0], 'B'
	;mov byte [gs:1], 0x0F

	cmp word [0x7E00 - 2], 0xAA55
	jne error

	;mov byte [gs:0], 'C'
	;mov byte [gs:1], 0x0F

	jmp 0:0x7C00

error:

	mov byte [gs:0], 'E'
	mov byte [gs:1], 0x0F

	hlt

_end_guest_16:

[bits 16]
_start_hooked_int15h:


    ;; intentionatly do a vmexit
    ;; and treat it from C code

    vmcall ;; handle it in C code
	iret    ; we arrive here only if int15h was made with eax=E820

    ;; 


_end_hooked_int15h:

[bits 64]

SaveGuestRegisters:

    ; In case of VM EXIT
    ; Set RIP to this addr

    save_proc_state

	mov rcx, rsp

	sub rsp, 0x20

	call VmxVmExitHandler



    ; HLT Instructions/sec - Number of CPU halts per seconds on the VP (VIRTUAL PROCESSOR)
    ; A HLT will cause the hypervisor scheduler to de-schedule the current VP and move the the next VP in the runlist
	hlt

RestoreGuestRegistersAndResume:

	call RestoreRegisters

	vmresume

	ret
