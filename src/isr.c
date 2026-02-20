#include "../include/isr.h"
#include "../include/kernel.h"

void isr_c_handler(interrupt_frame* ctx){
    printd("Interrupt %d TRIGGERED\n", ctx->interrupt);
    for(int i=0;i<300000000;i++);
}






