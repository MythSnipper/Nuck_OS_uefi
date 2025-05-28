#ifndef KERNEL_H
#define KERNEL_H

#include "../include/nuckdef.h"
#include <efi.h>
#include <efilib.h>

typedef EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE EFI_GOP;



void GOPDrawRect(EFI_GOP* GOP, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color, uint8_t fill);
void GOPPutPixel(EFI_GOP* GOP, uint32_t x, uint32_t y, uint32_t color);
static inline uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);


#endif