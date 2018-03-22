%define VA(x) (MULTIBOOT_BASE + (x - ___FirstAddressLabel))
%define REBASE(x)  (((x) - reloc) + INT32_BASE)
;
; PXE 32 bit loader code
;
; for documentation check out MULTIBOOT 0.6.96 specs
; http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
;

; ianichitei: in order to be sure that this is linked first, we put it in a different section,
; then we merge it with the .text section (see Project Properties -> Linker -> Advanced -> Merge)

SECTION .boot
___FirstAddressLabel:

global __MultiBootEntryPoint
global gTempE820
global gTempE820EntriesExt

extern Init64
;
; multiboot starts in 32 bit PROTECTED MODE, without paging beeing enabled (FLAT)
; check out '3.2 Machine state' from docs
;
[bits 32]

INT32_BASE                  equ 0x7C00
CODE64_SEL                  equ 0x08
DATA64_SEL                  equ 0x10
CODE16_SEL                  equ 0x18
DATA16_SEL                  equ 0x20
CODE32_SEL                  equ 0x08
DATA32_SEL                  equ 0x10

;
; we use hardcoded address space / map for our data structures, the multiboot header and the entry point
; the plain binary image is loaded to 0x00200000 (2MB), the entry point is fixed to 0x00209000
;
MULTIBOOT_HEADER_SIZE       equ 48                      ; check out '3.1.1 The layout of Multiboot header'
MULTIBOOT_HEADER_MAGIC      equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS      equ 0x00010003
MULTIBOOT_LOADER_MAGIC      equ 0x2BADB002				;  >>>> EAX must store this value

MULTIBOOT_INFO_STRUCT_SIZE  equ 90

KERNEL_BASE_VIRTUAL         equ 0x0000010000000000      ; magic 1T
KERNEL_BASE                 equ 0x2000000               ; 32 MB
KERNEL_LENGTH               equ 0x2000000               ; 32 MB (TDI: 2012/11/19)

MULTIBOOT_BASE              equ KERNEL_BASE + 0x400     ; take into account the MZ/PE header + 0x400 allignment

MULTIBOOT_INFO_STRUCT_BASE  equ MULTIBOOT_BASE + MULTIBOOT_HEADER_SIZE
MULTIBOOT_ENTRY_POINT       equ KERNEL_BASE + 0xC000    ; at EIP = 0x20C000 we start the execution (32 bit, non-paged)

PML4_TABLE_BASE             equ KERNEL_BASE + 0x2000
PT_TABLE_BASE               equ KERNEL_BASE + 0x7000

INITIAL_TOP_OF_STACK        equ KERNEL_BASE + 0xA000
INITIAL_TOP_OF_STACK_1T     equ KERNEL_BASE_VIRTUAL + 0xA000

IA32_EFER                   equ 0xC0000080
CR4_PAE                     equ 0x00000020
IA23_EFER_LME               equ 0x100

;;
;; KERNEL_BASE + 0x400 (this must be the first stuff to be linked into the code segment)
;;

multiboot_header:                                       ; check out '3.1.1 The layout of Multiboot header'
magic           dd MULTIBOOT_HEADER_MAGIC
flags           dd MULTIBOOT_HEADER_FLAGS
checksum        dd 0-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
header_addr     dd MULTIBOOT_BASE
load_addr       dd KERNEL_BASE
load_end_addr   dd 0
bss_end_addr    dd 0
entry_addr      dd MULTIBOOT_ENTRY_POINT
mode_type       dd 0
width           dd 0
height          dd 0
depth           dd 0

gMultiBootStruct:                                       ; reserve space for the multiboot info structure (will copy here)
times MULTIBOOT_INFO_STRUCT_SIZE db 0                   ; check out '3.3 Boot information format'

;; leave 0x40 bytes for GDT stuff
times (0x100 - MULTIBOOT_HEADER_SIZE - MULTIBOOT_INFO_STRUCT_SIZE - 0x40) db 0


;;
;; KERNEL_BASE + 0x4C0
;;
__gdt_base:                            ; GDT with 5 entries

.null:                                 ; 0x00 - null segment descriptor
    dd 0x00000000                      ; must be left zero'd
    dd 0x00000000                      ; must be left zero'd

.code64:                               ; 0x01 - 32bit code segment descriptor 0xFFFFFFFF
    dw 0xFFFF                          ; limit  0:15
    dw 0x0000                          ; base   0:15
    db 0x00                            ; base  16:23
    db 0x9A                            ; present, iopl/0, code, execute/read
    db 0x2F                            ; 1byte granularity, 32bit selector, limit 16:19
    db 0x00                            ; base  24:31

.data64:                               ; 0x02 - 32bit data segment descriptor 0xFFFFFFFF
    dw 0xFFFF                          ; limit  0:15
    dw 0x0000                          ; base   0:15
    db 0x00                            ; base  16:23
    db 0x92                            ; present, iopl/0, data, read/write
    db 0xCF                            ; 4Kbyte granularity, 32bit selector; limit 16:19
    db 0x00                            ; base  24:31

.code16:                               ; 0x03 - 16bit code segment descriptor 0x000FFFFF
    dw 0xFFFF                          ; limit  0:15
    dw 0x0000                          ; base   0:15
    db 0x00                            ; base  16:23
    db 0x9A                            ; present, iopl/0, code, execute/read
    db 0x0F                            ; 1Byte granularity, 16bit selector; limit 16:19
    db 0x00                            ; base  24:31

.data16:                               ; 0x04 - 16bit data segment descriptor 0x000FFFFF
    dw 0xFFFF                          ; limit  0:15
    dw 0x0000                          ; base   0:15
    db 0x00                            ; base  16:23
    db 0x92                            ; present, iopl/0, data, read/write
    db 0x0F                            ; 1Byte granularity, 16bit selector; limit 16:19
    db 0x00                            ; base  24:31


__gdt_end:


;CODE64_SEL      equ      0x08                           ; 64 bit mode code selector
;DATA64_SEL      equ      0x10                           ; 64 bit data selector / stack selector

; size and address of __gdt_base                        ; base is 0x2004D8 (GDT_TABLE_ADDRESS
lgdt_structure:
    gdt_size        dw  __gdt_end - __gdt_base - 1
    gdt_address     dq  VA(__gdt_base)


;;
;; KERNEL_BASE + 0x0500
;;
align 0x100, db 0
; memory map as reported by E820 map and retrieved by the bootloader for us
; we copy here the memory map from the location specified in the MultiBoot Information structure
; check out '3.3 Boot information format' and also 'http://en.wikipedia.org/wiki/E820'
; IMPORTANT: this allows us at most 287 entries of default size (0x18 / entry, 4 bytes initially for length)
gTempE820EntriesExt dd 0
gTempE820:
times 0x1AFC db 0


;;
;; KERNEL_BASE + 0x2000 - PML4
;;
;
; setup page tables to identity map the 0-8M physical space
; we also need to map the 1T (0x0000`0100`0000`0000-0x0000`0100`001F`FFFF) virtual space to 2-4M physical space for x64
;
; IMPORTANT: here we DO assume that there is always a continous 2-6 physical RAM present and available
;
__pml4_table:
dq              0x2003007                ; entry for 0 - 512G, PDP
dq 0
dq              0x2004007                ; entry for 1T - 1,5T, PDP
times 509 dq 0

;; KERNEL_BASE + 0x3000 - PDP #1, for identity mapping
__pdp_table_identity:
dq              0x2005007                ; entry for 0 - 1G, PD #1
times 511 dq 0

;; KERNEL_BASE + 0x4000 - PDP #2, for 1T mapping
__pdp_table_1t:
dq              0x2006007                ; entry for 1T - 1T+1G, PD #2
times 511 dq 0

;; KERNEL_BASE + 0x5000 - PD #1
__pd_table1:
dq              0x2007007                ; entry for 0 - 2M, PT, using PT to avoid mapping first 4K (do NOT map NULL pointer)
dq              0x0200087                ; identity mapping for 2-4M, page
dq              0x0400087                ; identity mapping for 4-6M, page
dq              0x0600087                ; identity mapping for 6-8M, page
dq              0x0800087                ; identity mapping for 8-10M, page
dq              0x0A00087                ; identity mapping for 10-12M, page
dq              0x0C00087                ; identity mapping for 12-14M, page
dq              0x0E00087                ; identity mapping for 14-16M, page
dq              0x1000087                ; identity mapping for 16-18M, page
dq              0x1200087                ; identity mapping for 18-20M, page
dq              0x1400087                ; identity mapping for 20-22M, page
dq              0x1600087                ; identity mapping for 22-24M, page
dq              0x1800087                ; identity mapping for 24-26M, page
dq              0x1A00087                ; identity mapping for 26-28M, page
dq              0x1C00087                ; identity mapping for 28-30M, page
dq              0x1E00087                ; identity mapping for 30-32M, page
dq              0x2000087                ; identity mapping for 32-34M, page
times 495 dq 0

;; KERNEL_BASE + 0x6000 - PD #2
__pd_table2:
dq              0x2000087                ; mapping for physical 32-34M to virtual 1T-to-1T+32M range, page
dq              0x2200087                ; mapping for physical 34-36M to virtual 1T+32M-to-1T+34M range, page
dq              0x2400087                ; mapping for physical 36-38M to virtual 1T+34M-to-1T+36M range, page
dq              0x2600087                ; mapping for physical 38-40M to virtual 1T+36M-to-1T+38M range, page
dq              0x2800087                ; mapping for physical 40-42M to virtual 1T+38M-to-1T+40M range, page
dq              0x2A00087                ; mapping for physical 42-44M to virtual 1T+40M-to-1T+42M range, page
dq              0x2C00087                ; mapping for physical 44-46M to virtual 1T+42M-to-1T+44M range, page
dq              0x2E00087                ; mapping for physical 46-48M, to virtual 1T+44M-to-1T+46M range, page
dq              0x3000087                ; mapping for physical 48-50M, to virtual 1T+46M-to-1T+48M range, page
dq              0x3200087                ; mapping for physical 50-52M, to virtual 1T+48M-to-1T+50M range, page
dq              0x3400087                ; mapping for physical 52-54M, to virtual 1T+50M-to-1T+52M range, page
dq              0x3600087                ; mapping for physical 54-56M, to virtual 1T+52M-to-1T+54M range, page
dq              0x3800087                ; mapping for physical 56-58M, to virtual 1T+54M-to-1T+56M range, page
dq              0x3A00087                ; mapping for physical 58-60M, to virtual 1T+56M-to-1T+58M range, page
dq              0x3C00087                ; mapping for physical 60-62M, to virtual 1T+58M-to-1T+60M range, page
dq              0x3E00087                ; mapping for physical 62-64M, to virtual 1T+60M-to-1T+62M range, page
times 496 dq 0

;; KERNEL_BASE + 0x7000 - PT
__pt_table:
dq              0x000000                ; P = 0, NOT preset, to avoid NULL pointers
times 511 dq 0                          ; will be dynamically generated
;;dq              0x001007
;;dq              0x002007
;;dq              0x003007
;;...
;;dq              0x1FF007

;;
;; KERNEL_BASE + 0x8000 - temporary storage for PXE command line (will be reused as 1K more for stack)
;;
__TempCmdLine:
times 0x400 db 0xDD


;;
;; KERNEL_BASE + 0x8400 - we reserve a 8K stack for the initial thread (1K is above)
;; TOP-OF-STACK is KERNEL_BASE + 0xA000
;;
__stack:
times 0x1C00 db 0xCC


;;
;; KERNEL_BASE + 0xA000 - AP trampoline code, 16 bit part
;;
;; NOTE: this MUST be copied down below 1 MB (at AP_TRAMPOLINE_16B_BASE)
;;       and has the sole role to switch to flat 32 bit PM, and jump to __ApTrampoline32to64
;;

;; ...TBD...

times 0x2000 db 0xA0

;;
;; KERNEL_BASE + 0xC000 - code
;;
[bits 32]
; code_start - the multiboot loader transfers execution here (based on the entry_addr in the multiboot header above)
__MultiBootEntryPoint: use32

    cli                               ; disable interrupts

    ; special TRACE32 breakpoint on I/O 0xBDB0
    ;;mov     dx, 0xbdb0
    ;;mov     al, 0x01                ; TRACE32 break code 0x01
    ;;out     dx, al

    ; simply echo something to the screen, by direct memory write to 80x25 text mode VGA video mem (0xB8000)




    mov     ecx, '1111'                     ; signal our presence
    mov     [0x000B8000], ecx

    ; setup initial ESP, to have stack
    mov     esp, INITIAL_TOP_OF_STACK

    ; special TRACE32 breakpoint on I/O 0xBDB0
    mov     ecx, eax
    mov     esi, edx                        ; EDX might contain the legacy-boot magic value
    mov     dx, 0xbdb0
    mov     al, 0x01                        ; TRACE32 break code 0x01
    out     dx, al
    mov     eax, ecx
    mov     edx, esi

    ; check if we were loaded by multiboot
    cmp     eax, MULTIBOOT_LOADER_MAGIC
    jz      mb_load

    mov     eax, 'EEEE'                     ; signal error
    mov     [0x000B8000], eax
    cli
    hlt

mb_load: use32

    ; multiboot loaded us, check for cmd line and copy it inside kernel image at __cmdline
    mov     eax, '2222'                     ; signal our presence
    mov     [0x000B8004], eax


    ; start transition from pm to rm
int32: use32
    pusha                                      ; save register state to 32bit stack
    lgdt    [VA(lgdt_structure)]               ; load 16 bit gdt structure
    mov     esi, VA(reloc)                     ; set source to code below
    mov     edi, INT32_BASE                    ; set destination to new base address
    mov     ecx, (int16_end - reloc)           ; set copy size to our codes size
    cld                                        ; clear direction flag (so we copy forward)
    rep     movsb                              ; do the actual copy (relocate code to low 16bit space)
    jmp     word CODE16_SEL:REBASE(p_mode16)   ; switch to 16bit selector (16bit protected mode)

continue32:
    ;
    ; setup final PT table, to avoid mapping NULL pointers
    ;
    cld
    mov     edi, PT_TABLE_BASE + 8
    mov     ecx, 511                ; we need 511 entries
    mov     eax, 0x00001007         ; P = 1, R/W = 1, U/S = 1, base physical address = 0x0000`1000
    mov     edx, 0x00000000         ; upper half, because we use 64 bit entries
_one_more_entry: use32
    stosd                           ; store lower half of entry
    add     eax, 0x00001000         ; +4K, next physical page
    xchg    eax, edx
    stosd                           ; store upper half 0x0000`0000
    xchg    eax, edx
    loop    _one_more_entry


;
; now, we will enable PAE, setup LME, paging, load GDT and go to 64 bit mode
;
enable_pae: use32
    mov     eax, cr4
    or      eax, CR4_PAE            ; set bit 0x00000020
    mov     cr4, eax

    mov     eax, PML4_TABLE_BASE    ; 0x202000 physical
    mov     cr3, eax                ; set PBDR

    mov     ecx, IA32_EFER          ; MSR 0xC0000080, check out '9.8.5 Initializing IA-32e Mode' from Intel docs
    rdmsr                           ; also check out 'Table B-2. IA-32 Architectural MSRs' from Intel docs
    or      eax, IA23_EFER_LME      ; set LME bit, 0x100
    wrmsr

    ; enable paging
    mov     eax, cr0
    or      eax, 0x80000000
    mov     cr0, eax


;
; now we should be in 64-bit compatibility mode
;
[bits 64]
    mov     eax, '3333'             ; signal our presence
    mov     [0x000B8008], eax

    ; load the new GDT and go to real 64-bit mode
    ;mov     rsi, GDT_TABLE_ADDRESS  ; 0x2004D8, with GDT base at 0x2004C0


    lgdt [VA(lgdt_structure)]

    ; set the cs
    mov     esp, INITIAL_TOP_OF_STACK
    xor     eax, eax
    mov     ax, CODE64_SEL
    push    rax                     ; this is a MUST, because retf will pop out 4 bytes for CS (OPE found out this ;-)
                                    ; and 'push rax' actually means 'push eax', because we still run in 32 bit compat mode
    call    $ + 5                   ; place return EIP onto the stack
    mov     eax, 10                 ; instrux length to continue right after 'retf'
    add     [rsp], eax
    retf

;
; we are in true 64-bit code, but still using the identity mappings, NOT the final 1T VA
;
    ; set also fs, gs
    ; NOTE: ds, es, ss are NOT used on x64
    mov     ax, DATA64_SEL
    mov     fs, ax
    mov     gs, ax

    mov     eax, '4444'             ; signal our presence
    mov     [0x000B800C], eax

    ; setup the stack
    mov     rsp, INITIAL_TOP_OF_STACK_1T    ; 8K stack, already using the 1T VA addresses
    sub     rsp, 0x20

    ; switch to final 1T virtual addresses (0x0000`0100`0000`0000)
    call    $ + 5                   ; place return RIP onto the stack
    mov     rax, KERNEL_BASE_VIRTUAL - KERNEL_BASE
    add     qword [rsp], rax
    add     qword [rsp], 0x14       ; instrux length to continue right after 'retn'
    retn

    mov     eax, '5555'             ; signal our presence
    mov     [0x000B8010], eax

;
; now we are using final 1T virtual addresses, full x64, have an 8K stack in place, so are ready to jump to our C code
;
call_final: use64

    ; special TRACE32 breakpoint on I/O 0xBDB0
    ;;mov     dx, 0xbdb0
    ;;mov     al, 0x02                ; TRACE32 break code 0x02
    ;;out     dx, al

    sub     rsp, 0x20
    call    Init64
    add     rsp, 0x20

   mov     eax, 'BPBP'
   mov     [0xB8000], eax

    cli
    hlt

[bits 16]

reloc:

gTempE820Entries dd 0

__gTempE820:
    times 0x1AFC db 0

idt_real:
	dw 0x3ff		; 256 entries, 4b each = 1K
	dd 0			; Real Mode IVT @ 0x0000

__gdt32_base:                              ; GDT descriptor table
    .null32:                               ; 0x00 - null segment descriptor
        dd 0x00000000                      ; must be left zero'd
        dd 0x00000000                      ; must be left zero'd

    .code32:                               ; 0x01 - 32bit code segment descriptor 0xFFFFFFFF
        dw 0xFFFF                          ; limit  0:15
        dw 0x0000                          ; base   0:15
        db 0x00                            ; base  16:23
        db 0x9A                            ; present, iopl/0, code, execute/read
        db 0xCF                            ; 4Kbyte granularity, 32bit selector; limit 16:19
        db 0x00                            ; base  24:31

    .data32:                               ; 0x02 - 32bit data segment descriptor 0xFFFFFFFF
        dw 0xFFFF                          ; limit  0:15
        dw 0x0000                          ; base   0:15
        db 0x00                            ; base  16:23
        db 0x92                            ; present, iopl/0, data, read/write
        db 0xCF                            ; 4Kbyte granularity, 32bit selector; limit 16:19
        db 0x00                            ; base  24:31

gdt32_ptr:                                 ; ptr to our gdt
    dw gdt32_ptr - __gdt32_base - 1
    dd REBASE(__gdt32_base)

p_mode16: use16
	cli

	mov ax, DATA16_SEL              ; get or 16bit data selector
	mov ds, ax                      ; set ds to 16bit selector
	mov es, ax                      ; set es to 16bit selector
	mov fs, ax                      ; set fs to 16bit selector
	mov gs, ax                      ; set gs to 16bit selector
	mov ss, ax                      ; set ss to 16bit selector

    ; Disable paging (we need everything t  o be 1:1 mapped).
	mov eax, cr0
	and eax, 0x7FFFFFFE	            ; Disable paging bit & disable 16-bit pmode
	mov cr0, eax

	lidt [REBASE(idt_real)]         ; load 16bit idt

	jmp word 0x0000:REBASE(r_mode16)

r_mode16: use16
   	mov esp, INT32_BASE
	xor ax, ax                      ; set ax to 0
	mov ds, ax                      ; set ds so we can access idt16
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax                      ; set ss so the stack is valid



init_regs_e820:
    xor esi, esi
    mov edi, REBASE(__gTempE820)


do_e820:
    xor ebx, ebx                    ; ebx must be 0 at start
    xor bp, bp                      ; keep an entry count in bp
    mov edx, 0x534D4150             ; Places "SMAP" into edx
    mov eax, 0xE820
    mov [es:di + 20], dword 1       ; forces a valid ACPI 3.X entry ( 24 bytes )
    mov ecx, 24                     ; ask for 24 bytes
    int 0x15
    jc short .failed                ; carry set on first call means unsupported function
    mov edx, 0x0534D4150             ; some BIOSes apparently trash this register; we reset it to "SMAP" to be sure
    cmp eax, edx                    ; on success, eax mush have been reset to "SMAP"
    jne short .failed
    test ebx, ebx                   ; ebx = 0 implies list is only 1 entry long (worthless)
    je short .failed
    jmp .jmpin

.e820lp:                            ; this will almost do the same as do_e820 ( used after successfull calls after do_e820 )
    mov eax, 0xE820                 ; eax, ecx get trashed on every int 0x15 call
    mov [es:di + 20], dword 1       ; force a valid ACPI 3.X entry
    mov ecx, 24                     ; ask for 24 bytes
    int 0x15
    jc short .e820f                 ; carry set mean "end of list already reached"
    mov edx, 0x0534D4150            ; set back edx register to magic val : "SMAP"

.jmpin:
    jcxz .skipent                   ; skip any 0 length entries
    cmp cl, 20                      ; got a 24 byte ACPI 3.X response?
    jbe short .notext
    test byte [es:di + 20], 1       ; if so: is the "ignore this data" bit clear?

.notext:
    mov ecx, [es:di + 8]            ; get lower uint32_t of memory region length
    or edx, [es:di + 12]            ; "or" it with upper uint32_t to test for zero
    jz .skipent                     ; if length uint64_t is 0, skip this entry
    inc bp                          ; got a good entry: ++count, move to next storage spot
    add di, 24                      ; add 24 bytes ( next entry )

.skipent:
    test ebx, ebx                   ; if ebx resets to 0, list is complete
    jne short .e820lp               ; continue to search for memory

.e820f:                             ; final of query_e820_map (all succes)
    mov eax, 0xB8014
    mov dword [eax], '6666'
    mov word [REBASE(gTempE820Entries)], bp
    jmp short .pass

.failed:
    mov eax, 0xB8018                ; tests we reached this part of code
    mov dword [eax], '7777'

    ;hlt                             ; treat fail with hlt

.pass:

enable_vga_50_lines:

    mov     ax,     0x1112
    mov     bl,     0
    int     0x10

done_query:
    mov eax, cr0                                    ; get cr0 so we can modify it
    inc eax                                         ; set PE bit to turn on protected mode
    mov cr0, eax                                    ; set cr0 to result
    lgdt [REBASE(gdt32_ptr)]                        ; load 32bit gdt pointer
    jmp dword CODE32_SEL:REBASE(p_mode32)           ; switch to 32bit selector (32bit protected mode)

[bits 32]
p_mode32:

    mov eax, DATA32_SEL                             ; get our 32bit data selector
	mov ds, ax                                      ; reset ds selector
	mov es, ax                                      ; reset es selector
	mov fs, ax                                      ; reset fs selector
	mov gs, ax                                      ; reset gs selector
	mov ss, ax                                      ; reset ss selector

    mov eax, 0xB801C
    mov dword [eax], '8888'

    mov ecx, [REBASE(gTempE820Entries)]
    mov [VA(gTempE820EntriesExt)], ecx

    mov eax, 24
    mul ecx
    mov ecx, eax

    mov esi, REBASE(__gTempE820)
    mov edi, VA(gTempE820)

    rep movsb

    mov eax, VA(continue32)
    jmp eax                                         ; jump to continue the execution of 32bit code, ( to 64 )
int16_end: