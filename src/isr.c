#include "../include/isr.h"
#include "../include/kernel.h"

void isr_c_handler(interrupt_frame* ctx){
    printd("Interrupt %d TRIGGERED\n", ctx->interrupt);
    for(uint64_t i=0;i<30000000;i++);
}






