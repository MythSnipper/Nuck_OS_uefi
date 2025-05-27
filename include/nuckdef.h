#ifndef NUCKDEF_H
#define NUCKDEF_H

#include <efi.h>
#include <efilib.h>


#define true 1
#define false 0


/*
typedef unsigned int size_t;

typedef int intptr_t;
typedef unsigned int uintptr_t;

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef signed long int64_t;
typedef unsigned long uint64_t;
*/

typedef struct{
    CHAR16*                                 FirmwareVendor;
    UINT32                                  FirmwareRevision;
    EFI_RUNTIME_SERVICES*                   RuntimeServices;
    EFI_MEMORY_DESCRIPTOR*                  MemoryMap;
    UINTN                                   MemoryMapSize;
    UINTN                                   MemoryMapDescriptorSize;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE*      GOP;
} KERNEL_CONTEXT_TABLE;



#endif