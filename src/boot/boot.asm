; boot.asm: Multiboot2 entry point
; Starts in 32-bit protected mode, transitions to 64-bit long mode, then calls kernel_main.

section .multiboot progbits alloc exec nowrite align=8
align 8
multiboot_header:
    dd 0xE85250D6 ; magic number GRUB looks for. GRUB confirms checksum and jumps to _start section
    dd 0  ; architecture: i386 (32-bit protected mode)
    dd multiboot_header_end - multiboot_header ; header length
    dd -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header)) ; checksum
    dw 0  ; type
    dw 0  ; flags
    dd 8  ; size
multiboot_header_end:
 
; Page tables (identity-map first 2 MiB using 2 MiB pages)
section .bss
align 4096
pml4:    resb 4096
pdpt:    resb 4096
pd:      resb 4096

align 16
stack_bottom:
    resb 16384  
stack_top:

; GDT for 64-bit long mode
section .rodata
align 16
gdt64:
    dq 0  ; null descriptor
.code: equ $ - gdt64
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53) ; code segment: executable, code/data, present, 64-bit
.data: equ $ - gdt64
    dq (1 << 44) | (1 << 47) | (1 << 41) ; data segment: code/data, present, writable
.pointer:
    dw $ - gdt64 - 1 ; limit
    dq gdt64  ; base

; 32-bit entry point
section .text
bits 32
global _start
extern kernel_main

_start:
    ; Save multiboot info (ebx = pointer, eax = magic)
    mov edi, ebx  ; multiboot info pointer (pass to kernel later)
    mov esi, eax  ; multiboot magic

    ; Set up identity-mapped page tables
    mov eax, pdpt
    or eax, 0b11       
    mov [pml4], eax

    ; PDPT[0] -> PD
    mov eax, pd
    or eax, 0b11    
    mov [pdpt], eax

    ; PD[0] -> 2 MiB page at physical address 0
    mov dword [pd], 0b10000011  ; present + writable + huge (2 MiB page)

    ; Enable paging and enter long mode
    mov eax, pml4
    mov cr3, eax

    ; Enable PAE in CR4
    mov eax, cr4
    or eax, 1 << 5            
    mov cr4, eax

    ; Set Long Mode Enable in EFER MSR
    mov ecx, 0xC0000080    
    rdmsr
    or eax, 1 << 8       
    wrmsr

    ; Enable paging in CR0
    mov eax, cr0
    or eax, 1 << 31  
    mov cr0, eax

    ; Load 64-bit GDT and far jump to long mode
    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode

; 64-bit entry point
bits 64
long_mode:
    ; Set up data segment registers
    mov ax, gdt64.data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up stack
    mov rsp, stack_top

    ; Call C++ kernel entry point
    call kernel_main

    ; If kernel_main returns halt
.halt:
    cli
    hlt
    jmp .halt
