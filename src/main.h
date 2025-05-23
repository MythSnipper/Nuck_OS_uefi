#ifndef MAIN_H
#define MAIN_H

#include <efi.h>
#include <efilib.h>


void printMemoryMap(EFI_SYSTEM_TABLE* ST, UINTN MemoryMapSize, EFI_MEMORY_DESCRIPTOR* MemoryMap, UINTN MapKey, UINTN DescriptorSize, UINT32 DescriptorVersion);
void getMemoryMap(EFI_SYSTEM_TABLE* ST, UINTN* MemoryMapSize, EFI_MEMORY_DESCRIPTOR** MemoryMap, UINTN* MapKey, UINTN* DescriptorSize, UINT32* DescriptorVersion);
void printLogo(EFI_SYSTEM_TABLE* ST);
void printInfo(EFI_SYSTEM_TABLE* ST);

#endif