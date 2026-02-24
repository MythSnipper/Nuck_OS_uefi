#ifndef GDT_H
#define GDT_H

#include "../include/nuckdef.h"

typedef struct __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  limit__flags;
    uint8_t  base_high;
} GDT_Entry;

typedef struct __attribute__((packed)) {
    uint16_t size; //gdt size - 1
    GDT_Entry* offset; //gdt start
} GDT_Descriptor;

extern GDT_Descriptor GDTR;

void GDT_initialize(uint8_t* code_seg, uint8_t* data_seg);

//helper functions
#ifdef GDT_HELPER
static inline void GDT_set_entry(GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    entry->limit_low = (uint16_t)(limit & 0xFFFF);
    entry->base_low = (uint16_t)(base & 0xFFFF);
    entry->base_mid = (uint8_t)((base >> 16) & 0xFF);
    entry->access = access;
    entry->limit__flags = (uint8_t)(((limit >> 16) & 0xF) | (flags << 4));
    entry->base_high = (uint8_t)(base >> 24);
}
#endif

#endif