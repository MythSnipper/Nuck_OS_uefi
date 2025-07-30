ENTRY(kernel_main)

SECTIONS {
    /* Start at offset 0, position-independent */
    . = 0;

    _kernel_start = .;

    /* Code section: contains executable kernel code */
    .text ALIGN(4096) : {
        KEEP(*(.text.boot))     /* Optional boot-specific code */
        *(.text*)               /* All other code */
    }

    /* Read-only data: constants, strings, vtables, etc. */
    .rodata ALIGN(4096) : {
        *(.rodata*)
    }

    /* Initialized data section: global variables with initial values */
    .data ALIGN(4096) : {
        *(.data*)
    }

    /* BSS: Uninitialized global/static variables (zeroed at runtime) */
    .bss ALIGN(4096): {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    /* End of kernel binary marker */
    _kernel_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
