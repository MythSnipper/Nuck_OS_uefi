#ifndef NUCKBOOT_H
#define NUCKBOOT_H

#include <efi.h>
#include <efilib.h>

//to solve intellisense errors grrrrrrrrrrrrr
#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define BACKGROUND_COLOR EFI_TEXT_ATTR(EFI_BLACK, EFI_BLACK)
#define FONT_COLOR EFI_TEXT_ATTR(EFI_GREEN, EFI_BLACK)
#define FONT_COLOR_SELECTED EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK)

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


void crashout(EFI_SYSTEM_TABLE* ST, wchar_t* error, EFI_STATUS code);
void early_display_setting();
void load_kernel_resources(EFI_SYSTEM_TABLE* ST, EFI_HANDLE IH, EFI_FILE_PROTOCOL** root, EFI_PHYSICAL_ADDRESS loaded_addrs[], uint64_t kernel_stack_size, EFI_PHYSICAL_ADDRESS* kernel_stack, uint32_t* kernel_size);
void GOP_auto_select(EFI_SYSTEM_TABLE* ST, EFI_GRAPHICS_OUTPUT_PROTOCOL** GOP, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** GOP_info, UINTN* GOP_info_size, UINTN* selected_mode_num);
void GOP_auto_select_native(EFI_SYSTEM_TABLE* ST, EFI_GRAPHICS_OUTPUT_PROTOCOL** GOP, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** GOP_info, UINTN* GOP_info_size, UINTN* selected_mode_num);
void GOP_manual_select(EFI_SYSTEM_TABLE* ST, EFI_GRAPHICS_OUTPUT_PROTOCOL** GOP, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** GOP_info, UINTN* GOP_info_size, UINTN* selected_mode_num);

void display_refresh_entries(EFI_SYSTEM_TABLE* ST, wchar_t* menu_entries[], UINTN menu_number_of_entries, UINTN selected_menu_entry_index, UINTN start_column, UINTN start_row);
EFI_PHYSICAL_ADDRESS load_file(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* root, wchar_t* filename);
EFI_PHYSICAL_ADDRESS load_kernel_with_stack(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* root, wchar_t* filename, uint32_t* stack_size);
void close_file(EFI_FILE_PROTOCOL* file);
UINT64 get_file_size(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* file, wchar_t* filename);
EFI_FILE_PROTOCOL* open_file(EFI_FILE_PROTOCOL* volume, CHAR16* filename);
EFI_FILE_PROTOCOL* open_volume(EFI_SYSTEM_TABLE* ST, EFI_HANDLE IH);
void print_config_tables(EFI_SYSTEM_TABLE* ST);
void print_memory_map(EFI_SYSTEM_TABLE* ST, UINTN memory_map_size, EFI_MEMORY_DESCRIPTOR* memory_map, UINTN memory_map_key, UINTN memory_map_descriptor_size, UINT32 memory_map_descriptor_version);
void get_memory_map_highest_address(UINTN memory_map_size, EFI_MEMORY_DESCRIPTOR* memory_map, UINTN memory_map_descriptor_size, EFI_PHYSICAL_ADDRESS* high);
void get_memory_map(EFI_SYSTEM_TABLE* ST, UINTN* memory_map_size, UINTN* memory_map_size_pages, EFI_MEMORY_DESCRIPTOR** memory_map, UINTN* memory_map_key, UINTN* memory_map_descriptor_size, UINT32* memory_map_descriptor_version);
void print_logo(EFI_SYSTEM_TABLE* ST);
void print_info(EFI_SYSTEM_TABLE* ST);
void print_GUID(EFI_GUID* guid);
uint8_t compare_GUID(EFI_GUID* guid1, EFI_GUID* guid2);
void triple_fault();

#endif