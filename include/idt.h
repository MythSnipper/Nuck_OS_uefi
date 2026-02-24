#ifndef IDT_H
#define IDT_H

#include "../include/kernel.h"
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


#ifdef IDT_HELPER
//sets entry in the IDT table, uses global IDT
extern __attribute__((aligned(0x10)))IDT_Entry IDT[256];
static inline void IDT_set_entry(uint8_t vector, void* isr, uint8_t attrs, uint16_t segment, uint8_t IST){
    IDT_Entry* descriptor = &IDT[vector];

    descriptor->offset_low = (uint64_t)isr & 0xFFFF;
    descriptor->segment = segment;
    descriptor->ist = IST & 0b111;
    descriptor->attributes = attrs;
    descriptor->offset_mid = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->offset_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}
#endif

#endif
