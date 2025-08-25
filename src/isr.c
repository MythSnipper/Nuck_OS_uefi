#include "../include/isr.h"
#include "../include/kernel.h"


void isr_empty(struct interrupt_frame* frame){
    KERNEL_TEXT_OUTPUT title;
    KERNEL_TEXT_OUTPUT ConOut;

    title = (KERNEL_TEXT_OUTPUT){Terminus8x16_Bold, 8, 16, 2, 2, 0, 0, 20, 20, hex(0xFF10F0), hex(0x000000), true};
    ConOut = (KERNEL_TEXT_OUTPUT){Terminus8x16_Normal, 8, 16, 1, 1, 0, 8, 0, 0, hex(0xFF10F0), hex(0x000000), false};


    printf(global_ctx->GOP, &title, "hello world! interrupt btw\r\n");
}


void isr_52(struct interrupt_frame* frame){

}
