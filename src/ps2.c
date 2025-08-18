#define PS2_HELPER
#include "../include/ps2.h"

//PS/2
uint8_t PS2_mouse_init() {
    //enable second ps2 port
    PS2_command(0xA8);

    //enable data reporting
    PS2_write(0xF4, 1);
    //read back ack
    if(PS2_read(PS2_DATA) != 0xFA){
        return 1;
    }
    return 0;
}
uint8_t PS2_mouse_set_sample_rate(uint8_t rate){
    PS2_write(0xF3, 1); //set sample rate command
    //read back ack
    if(PS2_read(PS2_DATA) != 0xFA){
        return 1;
    }

    PS2_write(rate, 1);
    //read back ack
    if(PS2_read(PS2_DATA) != 0xFA){
        return 1;
    }
    return 0;
}
uint8_t PS2_poll(uint8_t* scancode, int8_t* dx, int8_t* dy, uint8_t* lrm){
    uint8_t status = PS2_read(PS2_STATUS);
    uint8_t is_mouse = (status & 0x20) ? 1 : 0;
    if(!is_mouse){ //keyboard
        *scancode = inb(PS2_DATA);
    }
    else{ //mouse
        uint8_t bytes[3];
        for(int i = 0; i < 3; i++){
            bytes[i] = PS2_read(PS2_DATA);
        }
        uint8_t state = bytes[0];
        *lrm = state & 0b111;
        uint8_t x = bytes[1];
        *dx = (int8_t)x;
        uint8_t y = bytes[2];
        *dy = -(int8_t)y;
    }
    return is_mouse;
}

