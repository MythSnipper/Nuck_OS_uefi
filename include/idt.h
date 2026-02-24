#ifndef IDT_H
#define IDT_H

#include "../include/nuckdef.h"
#include "../include/isr.h"




typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t segment;
    uint8_t  ist;
    uint8_t  attributes;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} IDT_Entry;

typedef struct __attribute__((packed)) {
    uint16_t size; //idt size - 1
    uint64_t offset; //idt start
} IDT_Descriptor;

typedef void (*isr_stub)();

extern IDT_Descriptor IDTR;

void IDT_initialize(uint16_t segment, uint8_t IST);
void IDT_set_entry(uint8_t vector, void* isr, uint8_t attrs, uint16_t segment, uint8_t IST);
//IDT END---------------------------------------



#endif
