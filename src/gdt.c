#define GDT_HELPER
#include "../include/gdt.h"

__attribute__((aligned(0x10)))
GDT_Entry GDT[3];
GDT_Descriptor GDTR;

void GDT_initialize(uint8_t* code_seg, uint8_t* data_seg){
    //set GDT entries
    GDTR.size = sizeof(GDT)-1;
    GDTR.offset = GDT;

    GDT_set_entry(&GDT[0], 0, 0, 0, 0); //null descriptor right here le

    GDT_set_entry(&GDT[1], 0, 0, //Code segment, base and limit 0 because long mode
    0b10011010, //Access:present, ring 0, non system segment(code/data segment), executable(code segment), non conforming, readable, access
    0b1010 //granularity: page granularity(not byte), size flag(0 because long mode), long mode code, reserved
    );

    GDT_set_entry(&GDT[2], 0, 0, //Data segment, base and limit 0 because long mode
    0b10010010, //Access:present, ring 0, non system segment(code/data segment), non executable(data segment), up direction, writable, access
    0b1000 //granularity: page granularity(not byte), size flag(0 because long mode data), not long mode code, reserved
    );

    //load the GDT
    asm volatile(
        ".intel_syntax noprefix\n"
        "lgdt [%[gdt]]\n"
        "push 0x08\n"
        "lea rax, [rip+__long_jump_after_loading_gdt]\n"
        "push rax\n"
        "retfq\n"
        "__long_jump_after_loading_gdt:\n"
        "mov ax, 0x10\n"
        "mov ds, ax\n"
        "mov es, ax\n"
        "mov fs, ax\n"
        "mov gs, ax\n"
        "mov ss, ax\n"
        ".att_syntax\n"
        :
        : [gdt] "r"(&GDTR)
        : "memory", "rax"
    );

    *code_seg = 0x08;
    *data_seg = 0x10;
    
}



