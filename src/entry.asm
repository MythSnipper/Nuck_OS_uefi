BITS 64

global __asm_entry_start
extern kernel_main

section .text.boot
__asm_entry_start:
    

    call kernel_main




__asm_entry_hang:
    cli
    hlt
    jmp __asm_entry_hang
