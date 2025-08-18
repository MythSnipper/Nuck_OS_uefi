#ifndef PS2_H
#define PS2_H

#include "../include/nuckdef.h"
#include "../include/port_io.h"

//PS/2
uint8_t PS2_mouse_init();
uint8_t PS2_mouse_set_sample_rate(uint8_t rate);
uint8_t PS2_poll(uint8_t* scancode, int8_t* dx, int8_t* dy, uint8_t* lrm);

//helper functions
#ifdef PS2_HELPER
#define PS2_DATA 0x60
#define PS2_STATUS 0x64
static inline uint8_t PS2_read(uint8_t port){
    while((inb(PS2_STATUS) & 0x01) == 0);
    return inb(port);
}
static inline void PS2_command(uint8_t command){
    while(inb(PS2_STATUS) & 0x02);
    outb(PS2_STATUS, command);
}
static inline void PS2_write(uint8_t data, bool isMouse){
    if(isMouse){
        PS2_command(0xD4);
    }
    //send data
    while(inb(PS2_STATUS) & 0x02);
    outb(PS2_DATA, data);
}
#endif

#endif