#ifndef KERNEL_H
#define KERNEL_H

#include "../include/nuckdef.h"
#include <efi.h>
#include <efilib.h>

typedef struct __attribute__((packed)) {
    uint16_t limit_low;     // Lower 16 bits of limit
    uint16_t base_low;      // Lower 16 bits of base
    uint8_t  base_mid;      // Middle 8 bits of base
    uint8_t  access;        // Access flags
    uint8_t  granularity;   // Granularity + upper 4 bits of limit
    uint8_t  base_high;     // Highest 8 bits of base
} GDT_Entry;

typedef struct __attribute__((packed)) {
    uint16_t limit;     // Size of the GDT - 1
    uint64_t base;      // Address of the first GDT entry
} GDT_Descriptor;

typedef struct{
    uint8_t* font;
    uint32_t charWidth;
    uint32_t charHeight;
    uint32_t scaleX;
    uint32_t scaleY;
    uint32_t cursorX;
    uint32_t cursorY;
    uint32_t offsetX;
    uint32_t offsetY;
    uint32_t frontColor;
    uint32_t backColor;
    uint8_t useAbsolutePosition;
} KERNEL_TEXT_OUTPUT;

typedef struct{
    CHAR16*                            FirmwareVendor;
    uint32_t                           FirmwareRevision;
    EFI_RUNTIME_SERVICES*              RuntimeServices;
    EFI_MEMORY_DESCRIPTOR*             MemoryMap;
    uint64_t                           MemoryMapSize;
    uint64_t                           MemoryMapDescriptorSize;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* GOP;
    EFI_PHYSICAL_ADDRESS               fb; //backbuffer in bootloader, frontbuffer in kernel
    EFI_PHYSICAL_ADDRESS               kernelStack;
    uint64_t                           kernelStackSize;
} KERNEL_CONTEXT_TABLE;

typedef EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE EFI_GOP;

void printFloat(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec);
void printUfloat(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec);
void printInt(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, int64_t num, uint8_t base);
void printUint(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint64_t num, uint8_t base);
void printf(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t* str, ...);
uint64_t encodeGDTEntry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void printString(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t* string);
void printChar(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t ascii_char);
void GOPDrawRect(EFI_GOP* GOP, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color, uint8_t fill);
void GOPPutPixel(EFI_GOP* GOP, uint32_t x, uint32_t y, uint32_t color);
static inline uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
static inline uint32_t hex(uint32_t hex);


#endif