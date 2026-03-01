ENTRY(__asm_entry_start)

PHDRS
{
    text    PT_LOAD FILEHDR PHDRS FLAGS(5);   /* R + X */
    data    PT_LOAD FLAGS(6);                 /* R + W */
    dynamic PT_DYNAMIC FLAGS(6);
}

SECTIONS {
    . = SIZEOF_HEADERS;

    /* Code section: contains executable kernel code */
    .text ALIGN(4096) : {
        *(.text.boot)
        *(.text)
        *(.text.asm_isr_stubs)
    } :text

    /* Read-only data: constants, strings, vtables, etc. */
    .rodata ALIGN(4096) : {
        *(.rodata*)
    } :text

    /* Initialized data section */
    .data ALIGN(4096) : {
        *(.gdata)
        *(.data*)
    } :data

    /* Dynamic section (required for relocations in PIE) */
    .dynamic ALIGN(4096) : {
        *(.dynamic*)
    } :data :dynamic

    /* BSS (NOLOAD — not stored in file) */
    .bss ALIGN(4096) (NOLOAD) : {
        *(.bss*)
        *(COMMON)
    } :data

    _kernel_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame*)
        *(.interp)
        *(.dynsym)
        *(.dynstr)
        *(.hash)
    }
}
