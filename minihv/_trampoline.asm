
global tramp_start ; used to find the size of the tramp code
global tramp_end

global stack ; to be completed from C code by the BSP
global pml4TableAddress ; to be completed from C code by the BSP
global apsGdtrLength
global apsGdtrAddress
global apsTrSelector
global switch_rip_2_va
global paToVa

extern ApInit64

; computes addresses used for the tramp code, i.e. 0x2000 + offset
%define tramp_address(x) (TRAMP_LOCATION_16 + ((x) - tramp_start))

IA32_EFER                   equ 0xC0000080
CR4_PAE                     equ 0x00000020
IA32_EFER_LME               equ 0x100
CR0_PE						equ 0x1

TRAMP_LOCATION_16	equ 0x2000

TRAMP_GDT_SIZE		equ tramp_gdt_base.end - tramp_gdt_base
TRAMP_GDT_ADDRESS	equ tramp_address(tramp_gdt_base)
TRAMP_GDTR			equ TRAMP_GDT_ADDRESS + TRAMP_GDT_SIZE

TRAMP_CODE32_SEL	equ 0x10 ; these two are from the low address GDT
TRAMP_DATA32_SEL	equ 0x18

TRAMP_CODE64_SEL	equ 0x08 ; this one is from both the low address and high address GDT
TRAMP_DATA64_SEL	equ 0x10 ; this one is from the high address GDT

[bits 16]

tramp_start:

	wbinvd

	cli

	cld

	jmp 0:tramp_address(tramp_16_cs)

tramp_16_cs:

	xor ax, ax
    mov ds, ax
	mov es, ax
	mov fs, ax
	mov ss, ax

	; enable protection
	mov eax, cr0
	or eax, CR0_PE
	mov cr0, eax

	lgdt [TRAMP_GDTR]

	mov eax, TRAMP_DATA32_SEL
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	jmp TRAMP_CODE32_SEL:tramp_address(tramp_32_cs)

[bits 32]
tramp_32_cs:

	; enable PAE
	mov     eax, cr4
    or      eax, CR4_PAE
    mov     cr4, eax

	; load cr3
    mov     eax, [tramp_address(pml4TableAddress)]
    mov     cr3, eax

	; enable IA32_EFER.LME
    mov     ecx, IA32_EFER
    rdmsr
    or      eax, IA32_EFER_LME
    wrmsr

    ; enable paging
    mov     eax, cr0
    or      eax, 0x80000000
    mov     cr0, eax

[bits 64]

	; setup temporary stack
	mov esp, tramp_address(initial_stack)

	; push code selector on stack
	xor     eax, eax
	mov     ax, TRAMP_CODE64_SEL ; still using GDT loaded in 32 bits mode
	push    rax

	; perform a far return => we will switch to long mode
	call    $ + 5
	mov     eax, 10
	add     [rsp], eax
	retf

	; we are in 64 bits mode
	; load final 64 bit GDT, from the mapped kernel virtual address space
	lgdt [tramp_address(apsGdtrLength)]

	; push code selector
	xor     eax, eax
	mov     ax, TRAMP_CODE64_SEL
	push    rax

	; reload CS to reflect the new GDT
	call    $ + 5
	mov     eax, 10
	add     [rsp], eax
	retf ; far return, will pop code selector into CS

	; here we are already using the GDT from the virtual address space of the kernel
	mov ax, TRAMP_DATA64_SEL
	mov fs, ax
	mov gs, ax

	; load TR with selector coresponding to CPU id + 3
	ltr [tramp_address(apsTrSelector)]

	; setup final VA stack
	mov rsp, [tramp_address(stack)]

	; load VA into RIP
	sub rsp, 0x8 ; make room for the return VA
	mov rax, [tramp_address(paToVa)]
	mov [rsp], rax
	retn

switch_rip_2_va:

	call ApInit64

	hlt

; we put the data here, because when the AP receives the IPI, it starts execution at 0x2000 and we don't want to execute data

; temporary GDT, used to switch from switch 16 -> 32 -> 64
tramp_gdt_base:
.null	dq 0
.code64 dq 0x002F9A000000FFFF
.code32 dq 0x00CF9A000000FFFF
.data32 dq 0x00CF92000000FFFF
.end

tramp_gdt_limit dw TRAMP_GDT_SIZE - 1  ; limit of the temporary GDT
tramp_gdt_address dd TRAMP_GDT_ADDRESS ; base of the temporary GDT

stack dq 0 ; will be written from C, stores the VA to be loaded into rsp, which will be used by this CPU

pml4TableAddress dq 0 ; will be written from C, stores the value to be loaded in the CR3 when paging is enabled

apsGdtrLength dw 0 ; will be written from C, stores the length of the GDT from the mapped kernel address space
apsGdtrAddress dq 0 ; will be written from C, stores the VA of the GDT from the mapped kernel address space

apsTrSelector dw 0 ; will be written from C, stores tss selector in the GDT from the mapped kernel address space

times 0x90 dq 0 ; space for the temp stack

initial_stack dq 0 ; temporary stack

paToVa dq 0 ; will be written from C, stores the VA of the instruction to which we jump, in order to load RIP with VA

tramp_end:
