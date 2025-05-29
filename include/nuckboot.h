#ifndef NUCKBOOT_H
#define NUCKBOOT_H

#include "../include/nuckdef.h"
#include <efi.h>
#include <efilib.h>

typedef struct{
    CHAR16*                            FirmwareVendor;
    UINT32                             FirmwareRevision;
    EFI_RUNTIME_SERVICES*              RuntimeServices;
    EFI_MEMORY_DESCRIPTOR*             MemoryMap;
    UINTN                              MemoryMapSize;
    UINTN                              MemoryMapDescriptorSize;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* GOP;
} KERNEL_CONTEXT_TABLE;

void closeFile(EFI_FILE_PROTOCOL* file);
UINT64 getFileSize(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* file);
EFI_FILE_PROTOCOL* openFile(EFI_FILE_PROTOCOL* volume, CHAR16* filename);
EFI_FILE_PROTOCOL* openVolume(EFI_SYSTEM_TABLE* ST, EFI_HANDLE IH);
void printMemoryMap(EFI_SYSTEM_TABLE* ST, UINTN MemoryMapSize, EFI_MEMORY_DESCRIPTOR* MemoryMap, UINTN MapKey, UINTN DescriptorSize, UINT32 DescriptorVersion);
void getMemoryMap(EFI_SYSTEM_TABLE* ST, UINTN* MemoryMapSize, EFI_MEMORY_DESCRIPTOR** MemoryMap, UINTN* MapKey, UINTN* DescriptorSize, UINT32* DescriptorVersion);
void printLogo(EFI_SYSTEM_TABLE* ST);
void printInfo(EFI_SYSTEM_TABLE* ST);

#endif