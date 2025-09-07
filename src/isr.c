#include "../include/isr.h"
#include "../include/kernel.h"

void __attribute__((cdecl)) isr_c_handler(interrupt_frame* ctx){
    printd("Interrupt %d TRIGGERED\n", ctx->interrupt);
    while(1);
}






