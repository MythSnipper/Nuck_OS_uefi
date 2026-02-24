#ifndef KERNEL_H
#define KERNEL_H

#include <efi.h>
#include <efilib.h>

#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/ps2.h"
#include "../include/port_io.h"

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
    uint8_t  useAbsolutePosition;
} KERNEL_TEXT_OUTPUT;

typedef struct{
    uint8_t* addr;
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t frameCount;
    uint32_t frameCounter; //goes from 0 to frameCount-1
} KERNEL_NVIDEO;

typedef struct{
    uint8_t*                           bitmap;
    uint32_t                           bitmap_size_pages;
} KERNEL_PMM_RANGE;

typedef struct{
    int16_t*                           FirmwareVendor;
    uint32_t                           FirmwareRevision;
    EFI_RUNTIME_SERVICES*              RuntimeServices;

    EFI_MEMORY_DESCRIPTOR*             MemoryMap;
    uint32_t                           MemoryMapSizeBytes;
    uint32_t                           MemoryMapSizePages;
    uint32_t                           MemoryMapDescriptorSize;

    EFI_CONFIGURATION_TABLE*           ConfigTable;
    uint32_t                           ConfigTableEntriesCount;

    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* GOP;
    EFI_PHYSICAL_ADDRESS               fb; //backbuffer in bootloader, frontbuffer in kernel

    EFI_PHYSICAL_ADDRESS               kernelStack;
    uint32_t                           kernelStackSizePages;

    EFI_PHYSICAL_ADDRESS               kernelImageStart;
    uint32_t                           kernelImageSizePages;

    KERNEL_PMM_RANGE                   kernelPMMRange;
    EFI_PHYSICAL_ADDRESS               kernel_resource_addrs[3];
} KERNEL_CONTEXT_TABLE;

/*
typedef struct{
    KERNEL_PMM_RANGE*  heap;
    uint64_t*          freeListStart;
    uint64_t*          freeListEnd;
} KERNEL_SUBPAGE_ALLOCATOR;
*/

typedef EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE EFI_GOP;

//Global vars
extern uint8_t VGAfont[];
extern uint8_t Terminus8x16_Normal[];
extern uint8_t Terminus8x16_Bold[];

extern KERNEL_CONTEXT_TABLE* global_ctx;

void update_framebuffer(KERNEL_CONTEXT_TABLE* ctx);

void print_memory_map(KERNEL_CONTEXT_TABLE* ctx, KERNEL_TEXT_OUTPUT* Con);

//config table related
void* getConfigTable(EFI_CONFIGURATION_TABLE* tablePtr, uint64_t entries, uint8_t tableindex);
void viewConfigTables(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* con, EFI_CONFIGURATION_TABLE* tablePtr, uint64_t entries);

void printGUID(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* con, EFI_GUID* guid);
uint8_t cmpGUID(EFI_GUID* guid1, EFI_GUID* guid2);

typedef void (*Kernel_entry)(KERNEL_CONTEXT_TABLE*);

//PIC functions
static inline void PIC_disable();


void GDT_set_entry(GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void triple_fault();

/*
//dynamic memory allocation functions
void subpage_alloc_init(KERNEL_SUBPAGE_ALLOCATOR* alloc);
void* subpage_alloc(KERNEL_SUBPAGE_ALLOCATOR* alloc);
void subpage_free(KERNEL_SUBPAGE_ALLOCATOR* alloc, void* addr);
void subpage_alloc_expand(KERNEL_SUBPAGE_ALLOCATOR* alloc);

void heap_init(KERNEL_HEAP* heap);
void* heap_alloc(KERNEL_HEAP* heap, uint64_t pages);
void heap_free(KERNEL_HEAP* heap, void* addr, uint64_t pages);
void heap_display(KERNEL_HEAP* heap, EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut);
*/


//graphical functions
void NVIDEOParseHeader(KERNEL_NVIDEO* video, uint8_t* addr);
void GOPPlayVideo(EFI_GOP* GOP, uint32_t x, uint32_t y, KERNEL_NVIDEO* video, bool loop);
void GOPDrawImage(EFI_GOP* GOP, uint32_t x, uint32_t y, KERNEL_NVIDEO* img);
void printf(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, char* str, ...);
void printFloat(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec);
void printUfloat(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec);
void printInt(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, int64_t num, uint8_t base);
void printUint(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint64_t num, uint8_t base);
void printString(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, char* string);
void printChar(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, char ascii_char);
void GOPDrawRect(EFI_GOP* GOP, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color, uint8_t fill);
void GOPPutPixel(EFI_GOP* GOP, uint32_t x, uint32_t y, uint32_t color);
void printd(char* str, ...);


void* memcpy(void* source, void* dest, uint64_t size);
uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t hex(uint32_t hex);
void cpuid(int code, uint32_t* a, uint32_t* d);
void cpuid_get_vendor(uint8_t* CPUVendor);
uint64_t rdtsc();


#endif
