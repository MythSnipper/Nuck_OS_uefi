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

//ELF header
/* Basic ELF types */
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;
typedef struct {
    unsigned char e_ident[16];
    Elf64_Half     e_type;
    Elf64_Half     e_machine;
    Elf64_Word      e_version;
    Elf64_Addr      e_entry;
    Elf64_Off       e_phoff;
    Elf64_Off       e_shoff;
    Elf64_Word      e_flags;
    Elf64_Half      e_ehsize;
    Elf64_Half      e_phentsize;
    Elf64_Half      e_phnum;
    Elf64_Half      e_shentsize;
    Elf64_Half      e_shnum;
    Elf64_Half      e_shstrndx;
} Elf64_Ehdr;
typedef struct {
    Elf64_Word   p_type;
    Elf64_Word   p_flags;
    Elf64_Off    p_offset;
    Elf64_Addr   p_vaddr;
    Elf64_Addr   p_paddr;
    Elf64_Xword  p_filesz;
    Elf64_Xword  p_memsz;
    Elf64_Xword  p_align;
} Elf64_Phdr;
typedef struct {
    Elf64_Word   sh_name;
    Elf64_Word   sh_type;
    Elf64_Xword  sh_flags;
    Elf64_Addr   sh_addr;
    Elf64_Off    sh_offset;
    Elf64_Xword  sh_size;
    Elf64_Word   sh_link;
    Elf64_Word   sh_info;
    Elf64_Xword  sh_addralign;
    Elf64_Xword  sh_entsize;
} Elf64_Shdr;
typedef struct {
    Elf64_Addr   r_offset;
    Elf64_Xword  r_info;
    Elf64_Sxword r_addend;
} Elf64_Rela;
typedef struct {
    Elf64_Sxword d_tag;
    union {
        Elf64_Xword d_val;
        Elf64_Addr  d_ptr;
    } d_un;
} Elf64_Dyn;
#define R_X86_64_RELATIVE 8
#define PT_LOAD 1
#define PT_DYNAMIC 2

#define DT_NULL     0
#define DT_NEEDED   1
#define DT_PLTRELSZ 2
#define DT_PLTGOT   3
#define DT_HASH     4
#define DT_STRTAB   5
#define DT_SYMTAB   6
#define DT_RELA     7
#define DT_RELASZ   8
#define DT_RELAENT  9
#define DT_STRSZ    10
#define DT_SYMENT   11
#define DT_INIT     12
#define DT_FINI     13
#define DT_SONAME   14
#define DT_RPATH    15
#define DT_SYMBOLIC 16
#define DT_REL      17
#define DT_RELSZ    18
#define DT_RELENT   19
#define DT_PLTREL   20
#define DT_DEBUG    21
#define DT_TEXTREL  22
#define DT_JMPREL   23
#define DT_BIND_NOW 24
#define DT_INIT_ARRAY 25
#define DT_FINI_ARRAY 26
#define DT_INIT_ARRAYSZ 27
#define DT_FINI_ARRAYSZ 28
#define DT_RUNPATH   29
#define DT_FLAGS     30
#define DT_FLAGS_1   0x6ffffffb
#define DT_RELACOUNT 0x6ffffff9

#define R_X86_64_NONE     0
#define R_X86_64_64       1
#define R_X86_64_PC32     2
#define R_X86_64_GOT32    3
#define R_X86_64_PLT32     4
#define R_X86_64_COPY      5
#define R_X86_64_GLOB_DAT  6
#define R_X86_64_JUMP_SLOT 7
#define R_X86_64_RELATIVE   8
#define R_X86_64_GOTPCREL   9
#define R_X86_64_32        10
#define R_X86_64_32S        11
#define R_X86_64_16         12
#define R_X86_64_PC16        13
#define R_X86_64_8            14
#define R_X86_64_PC8           15
#define R_X86_64_DTPMOD64      16
#define R_X86_64_DTPOFF64       17
#define R_X86_64_TPOFF64        18

#define ELF64_R_TYPE(info) ((info) & 0xffffffff)
#define ELF64_R_SYM(info)  ((info) >> 32)







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

    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* GOP; //frontbuffer in here
    uint32_t*                          fb; //backbuffer
    //values unset by bootloader are indented
        uint32_t pitch;
        uint32_t width;
        uint32_t height;


    EFI_PHYSICAL_ADDRESS               dirty_tilemap_addr; //map
    uint64_t                           dirty_tiles_x; //number of tiles horizontal
    uint64_t                           dirty_tiles_y; //number of tiles vertical
    uint64_t                           dirty_tile_size; //size of tile

    EFI_PHYSICAL_ADDRESS               kernelStack;

    EFI_PHYSICAL_ADDRESS               kernelImageStart;

    KERNEL_PMM_RANGE                   kernelPMMRange;
    EFI_PHYSICAL_ADDRESS               kernel_resource_addrs[3];
} KERNEL_CONTEXT_TABLE;



void dirty_tiles_size(uint64_t width, uint64_t height, uint64_t* tile_size);
void crashout(wchar_t* error, EFI_STATUS code);
void early_display_setting();
void load_kernel_resources(EFI_FILE_PROTOCOL** root, EFI_PHYSICAL_ADDRESS loaded_addrs[], uint64_t kernel_stack_size, EFI_PHYSICAL_ADDRESS* kernel_stack);
void GOP_auto_select(EFI_GRAPHICS_OUTPUT_PROTOCOL** GOP, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** GOP_info, UINTN* GOP_info_size, UINTN* selected_mode_num);
void GOP_auto_select_native(EFI_GRAPHICS_OUTPUT_PROTOCOL** GOP, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** GOP_info, UINTN* GOP_info_size, UINTN* selected_mode_num);
void GOP_manual_select(EFI_GRAPHICS_OUTPUT_PROTOCOL** GOP, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** GOP_info, UINTN* GOP_info_size, UINTN* selected_mode_num);
void display_refresh_entries(wchar_t* menu_entries[], UINTN menu_number_of_entries, UINTN selected_menu_entry_index, UINTN start_column, UINTN start_row);
EFI_PHYSICAL_ADDRESS load_file(EFI_FILE_PROTOCOL* root, wchar_t* filename);
EFI_PHYSICAL_ADDRESS load_kernel_elf(EFI_FILE_PROTOCOL* root, wchar_t* filename, uint64_t* stack_size_addr);
void close_file(EFI_FILE_PROTOCOL* file);
UINT64 get_file_size(EFI_FILE_PROTOCOL* file, wchar_t* filename);
EFI_FILE_PROTOCOL* open_file(EFI_FILE_PROTOCOL* volume, CHAR16* filename);
EFI_FILE_PROTOCOL* open_volume();
void print_config_tables();
void print_memory_map(UINTN memory_map_size, EFI_MEMORY_DESCRIPTOR* memory_map, UINTN memory_map_key, UINTN memory_map_descriptor_size, UINT32 memory_map_descriptor_version);
void get_memory_map_highest_address(UINTN memory_map_size, EFI_MEMORY_DESCRIPTOR* memory_map, UINTN memory_map_descriptor_size, EFI_PHYSICAL_ADDRESS* high);
void get_memory_map(UINTN* memory_map_size, UINTN* memory_map_size_pages, EFI_MEMORY_DESCRIPTOR** memory_map, UINTN* memory_map_key, UINTN* memory_map_descriptor_size, UINT32* memory_map_descriptor_version);
void print_logo();
void print_info();
void print_GUID(EFI_GUID* guid);
uint8_t compare_GUID(EFI_GUID* guid1, EFI_GUID* guid2);
void triple_fault();

//string functions implemented
int kmemcmp(const void* a, const void* b, unsigned long n);
void* kmemcpy(void* dest, const void* src, unsigned long n);
void* kmemset(void* dest, int value, unsigned long n);
unsigned long kstrlen(const char* str);


#endif