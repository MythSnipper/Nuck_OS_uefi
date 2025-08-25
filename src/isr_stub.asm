BITS 64


extern IDT_handlers_ptr

section .text

%macro ISR_STUB 1
global isr_stub_%1
isr_stub_%1:
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

    ;rdi = &interrupt_frame
    lea rdi, [rsp + 15*8]
    mov rsi, %1

    ; Lookup handler: rax = idt_handlers[rsi]
    mov rax, [rel IDT_handlers_ptr]
    mov rax, [rax]
    mov rcx, rsi
    shl rcx, 3              ; multiply by 8 bytes
    add rax, rcx
    mov rax, [rax]          ; load function pointer
    ;call handler
    cld
    call rax

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

    iretq
%endmacro


;generate all asm stubs
%assign i 0
%rep 256
    ISR_STUB i
%assign i i+1
%endrep

