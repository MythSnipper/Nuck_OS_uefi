ENTRY(__asm_entry_start)

SECTIONS {
    . = 0x100000;

    /* Code section: contains executable kernel code */
    .text ALIGN(4096) : {
        *(.text.boot)
        *(.text)
        *(.text.asm_isr_stubs)
    }

    /* Read-only data: constants, strings, vtables, etc. */
    .rodata ALIGN(4096) : {
        *(.rodata)
    }

    /* Initialized data section: global variables with initial values */
    .data ALIGN(4096) : {
        *(.gdata)
        *(.data)
        KEEP(*(.bss))
        *(COMMON)
    }


    _kernel_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}



