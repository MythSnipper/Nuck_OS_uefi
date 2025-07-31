#ifndef NUCKBOOT_H
#define NUCKBOOT_H

#include "../include/nuckdef.h"
#include <efi.h>
#include <efilib.h>

#define BACKGROUND_COLOR EFI_TEXT_ATTR(EFI_MAGENTA, EFI_MAGENTA)
#define FONT_COLOR EFI_TEXT_ATTR(EFI_WHITE, EFI_MAGENTA)
#define FONT_COLOR_SELECTED EFI_TEXT_ATTR(EFI_WHITE, EFI_CYAN)

//OTHER COLOR CHOICES
/*
#define BACKGROUND_COLOR EFI_TEXT_ATTR(EFI_BLACK, EFI_BLACK)
#define FONT_COLOR EFI_TEXT_ATTR(EFI_GREEN, EFI_BLACK)
#define FONT_COLOR_SELECTED EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK)
*/
/*
#define BACKGROUND_COLOR EFI_TEXT_ATTR(EFI_BLACK, EFI_BLACK)
#define FONT_COLOR EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLACK)
#define FONT_COLOR_SELECTED EFI_TEXT_ATTR(EFI_LIGHTRED, EFI_BLACK)
*/

typedef struct{
    uint8_t*                           map;
    uint8_t*                           heap;
} KERNEL_HEAP;

typedef struct{
    CHAR16*                            FirmwareVendor;
    uint32_t                           FirmwareRevision;
    EFI_RUNTIME_SERVICES*              RuntimeServices;
    EFI_MEMORY_DESCRIPTOR*             MemoryMap;
    uint64_t                           MemoryMapSize;
    uint64_t                           MemoryMapDescriptorSize;
    EFI_CONFIGURATION_TABLE*           ConfigTable;
    uint64_t                           ConfigTableEntriesCount;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* GOP;
    EFI_PHYSICAL_ADDRESS               fb; //backbuffer in bootloader, frontbuffer in kernel
    EFI_PHYSICAL_ADDRESS               kernelStack;
    uint64_t                           kernelStackSize;
    KERNEL_HEAP*                       heap;
    EFI_PHYSICAL_ADDRESS*              resource_addrs;
} KERNEL_CONTEXT_TABLE;




void crashout(EFI_SYSTEM_TABLE* ST, wchar_t* error, EFI_STATUS code);
void early_display_setting();
void load_kernel_resources(EFI_SYSTEM_TABLE* ST, EFI_HANDLE IH, EFI_FILE_PROTOCOL** root, EFI_PHYSICAL_ADDRESS loaded_addrs[]);
void GOP_auto_select(EFI_SYSTEM_TABLE* ST, EFI_GRAPHICS_OUTPUT_PROTOCOL** GOP, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** GOP_info, UINTN* GOP_info_size, UINTN* selected_mode_num);
void GOP_manual_select(EFI_SYSTEM_TABLE* ST, EFI_GRAPHICS_OUTPUT_PROTOCOL** GOP, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** GOP_info, UINTN* GOP_info_size, UINTN* selected_mode_num);


void display_refresh_entries(EFI_SYSTEM_TABLE* ST, wchar_t* menu_entries[], UINTN menu_number_of_entries, UINTN selected_menu_entry_index, UINTN start_column, UINTN start_row);
EFI_PHYSICAL_ADDRESS load_file(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* root, wchar_t* filename);
EFI_PHYSICAL_ADDRESS load_file_at_addr(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* root, wchar_t* filename, EFI_PHYSICAL_ADDRESS addr);
void close_file(EFI_FILE_PROTOCOL* file);
UINT64 get_file_size(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* file, wchar_t* filename);
EFI_FILE_PROTOCOL* open_file(EFI_FILE_PROTOCOL* volume, CHAR16* filename);
EFI_FILE_PROTOCOL* open_volume(EFI_SYSTEM_TABLE* ST, EFI_HANDLE IH);
void print_config_tables(EFI_SYSTEM_TABLE* ST);
void print_memory_map(EFI_SYSTEM_TABLE* ST, UINTN memory_map_size, EFI_MEMORY_DESCRIPTOR* memory_map, UINTN memory_map_key, UINTN memory_map_descriptor_size, UINT32 memory_map_descriptor_version);
void get_memory_map(EFI_SYSTEM_TABLE* ST, UINTN* memory_map_size, EFI_MEMORY_DESCRIPTOR** memory_map, UINTN* memory_map_key, UINTN* memory_map_descriptor_size, UINT32* memory_map_descriptor_version);
void print_logo(EFI_SYSTEM_TABLE* ST);
void print_info(EFI_SYSTEM_TABLE* ST);
void print_GUID(EFI_GUID* guid);
uint8_t compare_GUID(EFI_GUID* guid1, EFI_GUID* guid2);

#endif