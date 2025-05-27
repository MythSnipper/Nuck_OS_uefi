#ifndef KERNEL_H
#define KERNEL_H

#include "../include/nuckdef.h"
#include <efi.h>
#include <efilib.h>

typedef EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE EFI_GOP;

void putPixel(EFI_GOP* GOP, uint32_t x, uint32_t y);




#endif