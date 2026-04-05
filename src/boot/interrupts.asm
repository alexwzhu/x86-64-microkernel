; interrupts.asm: ISR and IRQ stubs for x86-64
; each stub pushes a normalized stack frame and jumps to the common handler

bits 64

extern isr_common_handler

; for exceptions that don't push an error code
%macro ISR_NOERR 1
global isr%1
isr%1:
    push qword 0    ; dummy error code
    push qword %1   ; vector number
    jmp isr_common_stub
%endmacro

; for exceptions that push an error code
%macro ISR_ERR 1
global isr%1
isr%1:
    push qword %1   ; vector number (CPU already pushed error code)
    jmp isr_common_stub
%endmacro

; CPU exceptions (0-31)
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR   29
ISR_ERR   30
ISR_NOERR 31

; hardware IRQs (32-47)
ISR_NOERR 32
ISR_NOERR 33
ISR_NOERR 34
ISR_NOERR 35
ISR_NOERR 36
ISR_NOERR 37
ISR_NOERR 38
ISR_NOERR 39
ISR_NOERR 40
ISR_NOERR 41
ISR_NOERR 42
ISR_NOERR 43
ISR_NOERR 44
ISR_NOERR 45
ISR_NOERR 46
ISR_NOERR 47

; common stub — saves all regs, calls C++ handler, restores, iretq
isr_common_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    xor rax, rax
    mov ax, ds
    push rax

    ; load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax

    ; first arg = pointer to stack frame
    mov rdi, rsp

    ; align stack to 16 bytes before call
    sub rsp, 8
    call isr_common_handler
    add rsp, 8

    pop rax
    mov ds, ax
    mov es, ax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16    ; skip vector and error code
    iretq
