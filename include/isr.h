#ifndef ISR_H
#define ISR_H

#include "../include/nuckdef.h"

struct interrupt_frame{
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

void isr_empty(struct interrupt_frame* frame);
void isr_52(struct interrupt_frame* frame);




#endif
