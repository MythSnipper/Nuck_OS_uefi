#include "../include/nuckboot.h"


EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
    EFI_SYSTEM_TABLE* ST = SystemTable;
    EFI_BOOT_SERVICES* BS = ST->BootServices;
    EFI_INPUT_KEY key;
    EFI_STATUS status;
 
    InitializeLib(ImageHandle, ST); //initialize runtime pointers
    bootloader_start:
    //disable watchdog timer
    status = uefi_call_wrapper(BS->SetWatchdogTimer, 4, 0, 0, 0, NULL);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to disable watchdog timer in SetWatchdogTimer", status);

    //simple keyboard loop to determine to reset or not to reset the display
    early_display_setting();

    //variables
    UINTN memory_map_size = 0; //size of the memory map in bytes
    UINTN memory_map_size_pages = 0; //size of memory map in pages
    EFI_MEMORY_DESCRIPTOR* memory_map = NULL; //pointer to the UEFI memory map
    UINTN memory_map_key; //key increments every time memory map changes
    UINTN memory_map_descriptor_size; //size of one entry in memory map
    UINT32 memory_map_descriptor_version; //memory map descriptor version

    //used to store the volume on the disk
    EFI_FILE_PROTOCOL* root; //root of boot device

    EFI_PHYSICAL_ADDRESS loaded_addrs[4]; //kernel, bad apple, nuckos logo, pointer icon

    //GOP variables
    EFI_GRAPHICS_OUTPUT_PROTOCOL* GOP; 
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* GOP_info;
    UINTN GOP_info_size;
    UINTN GOP_selected_mode = 0;


    EFI_PHYSICAL_ADDRESS backbuffer;  //backbuffer
    EFI_PHYSICAL_ADDRESS kernel_stack; //kernel stack
    uint32_t kernel_stack_size = 512; //size in pages, 2 MiB stack

    uint32_t kernel_image_size_pages;

    EFI_PHYSICAL_ADDRESS lowest_usable_range_addr;
    EFI_PHYSICAL_ADDRESS highest_usable_range_addr;
    EFI_PHYSICAL_ADDRESS kernel_pmm_bitmap_addr;
    uint32_t kernel_pmm_bitmap_size;

    //DISPLAY MENU data
    UINTN start_column; //starting column and row
    UINTN start_row;
    wchar_t* menu_entries[] = {
        L"Automatic boot",
        L"Load Nuck OS kernel and data",
        L"Select GOP mode(auto)",
        L"Select GOP mode(manual)",
        L"Boot Nuck OS",
        L"View memory map",
        L"View configuration tables",
        L"EFI Shell",
        L"Shutdown",
        L"triple fault"
    };
    UINTN menu_number_of_entries = 10;
    UINTN selected_menu_entry_index = 0;

    //clear console output, sets background color, cursor goes to 0, 0
    status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, BACKGROUND_COLOR);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to set ConOut background color attribute in SetAttribute", status);
    status = uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to clear ConOut screen in ClearScreen", status);

    //sets font color
    status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to set ConOut font color in SetAttribute", status);

    print_logo(ST);
    print_info(ST);

    Print(L"Press any key to continue...");
    while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) != EFI_SUCCESS);


    menu_start:
    selected_menu_entry_index = 0;
    status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, BACKGROUND_COLOR);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to set ConOut background color in SetAttribute", status);
    status = uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to clear ConOut screen in ClearScreen", status);

    //sets font color
    status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to set font color in SetAttribute", status);

    Print(L"---Boot Menu---\r\n");
    Print(L"   Arrow keys/WASD to move, Enter to select\r\n   Esc to reset bootloader, F12 to RESET bootloader\r\n\n");
    //save current cursor position because it's where the boot menu text starts
    start_column = ST->ConOut->Mode->CursorColumn;
    start_row = ST->ConOut->Mode->CursorRow;
    display_refresh_entries(ST, menu_entries, menu_number_of_entries, selected_menu_entry_index, start_column, start_row);

    while(true){
        //keyboard input
        if(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) == EFI_SUCCESS){
            
            //If it's F12 then really early start
            if(key.ScanCode == SCAN_F12){
                goto bootloader_start;
            }
            //If it's escape then reset bootloader
            if(key.ScanCode == SCAN_ESC){
                goto menu_start;
            }

            //Check if it's arrow keys
            if(key.ScanCode == SCAN_UP || key.ScanCode == SCAN_DOWN || key.UnicodeChar == L'w' || key.UnicodeChar == L's'){
                //Update index
                if(key.ScanCode == SCAN_UP || key.UnicodeChar == L'w'){
                    if(selected_menu_entry_index > 0)selected_menu_entry_index--;
                }
                if(key.ScanCode == SCAN_DOWN || key.UnicodeChar == L's'){
                    if(selected_menu_entry_index < menu_number_of_entries-1)selected_menu_entry_index++;
                }
                //refresh entries display
                display_refresh_entries(ST, menu_entries, menu_number_of_entries, selected_menu_entry_index, start_column, start_row);
            }

            //Check if it's Enter
            if(key.UnicodeChar == CHAR_CARRIAGE_RETURN){
                switch(selected_menu_entry_index){
                    case 0: {
                        //Auto boot
                    }
                    case 1: {
                        load_kernel_resources(ST, ImageHandle, &root, loaded_addrs, kernel_stack_size, &kernel_stack, &kernel_image_size_pages);
                        break;
                    }
                    case 2: {
                        GOP_auto_select(ST, &GOP, &GOP_info, &GOP_info_size, &GOP_selected_mode);
                        break;
                    }
                    case 3: {
                        GOP_manual_select(ST, &GOP, &GOP_info, &GOP_info_size, &GOP_selected_mode);
                        break;
                    }
                    case 4: {
                        //Boot Nuck OS

                        //kernel PMM stuff
                        get_memory_map(ST, &memory_map_size, &memory_map_size_pages, &memory_map, &memory_map_key, &memory_map_descriptor_size, &memory_map_descriptor_version);
                        get_memory_map_highlow_address(memory_map_size, memory_map, memory_map_descriptor_size, &lowest_usable_range_addr, &highest_usable_range_addr);
                        //allocate memory for kernel PMM range
                        uint32_t heap_size_pages = (highest_usable_range_addr-lowest_usable_range_addr)/4096;
                        uint32_t heap_bitmap_size_bytes = (heap_size_pages+7)/8; //1 byte = 8 pages
                        uint32_t heap_bitmap_size_pages = (heap_bitmap_size_bytes + 4095)/4096; //1 page = 4096 bytes
                        kernel_pmm_bitmap_size = heap_bitmap_size_pages;
                        status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, heap_bitmap_size_pages, &kernel_pmm_bitmap_addr);
                        Print(L"pmm bitmap allocated at address %X!\r\nheap size: %d, heap bitmap size: %d bytes %d pages\r\n", kernel_pmm_bitmap_addr, heap_size_pages, heap_bitmap_size_bytes, heap_bitmap_size_pages);
                        if(EFI_ERROR(status)){
                            Print(L"Can't allocate %d pages for pmm bitmap\r\n", heap_size_pages);
                            while(1);
                        }
                        get_memory_map(ST, &memory_map_size, &memory_map_size_pages, &memory_map, &memory_map_key, &memory_map_descriptor_size, &memory_map_descriptor_version);
                        get_memory_map_highlow_address(memory_map_size, memory_map, memory_map_descriptor_size, &lowest_usable_range_addr, &highest_usable_range_addr);



                        //last allocation
                        //allocate memory for backbuffer
                        UINTN backbuffer_size = GOP->Mode->Info->PixelsPerScanLine * GOP->Mode->Info->VerticalResolution * 4; //4 bpp
                        status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, GOP->Mode->FrameBufferSize, &backbuffer);
                        Print(L"backbuffer allocated at address %X!\r\n", backbuffer);
                        if(EFI_ERROR(status)){
                            Print(L"Can't allocate %d bytes for video backbuffer\r\n", GOP->Mode->FrameBufferSize);
                            while(1);
                        }


                        Print(L"Say goodbye to UEFI-land!\r\nPress any key to continue...\r\n");
                        while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) != EFI_SUCCESS);

                        //GOP info
                        uefi_call_wrapper(GOP->QueryMode, 4, GOP, GOP_selected_mode, &GOP_info_size, &GOP_info);
                        if(EFI_ERROR(status)) crashout(ST, L"Can't query GOP mode in QueryMode", status);
                        
                        //Set GOP mode
                        status = uefi_call_wrapper(GOP->SetMode, 2, GOP, GOP_selected_mode);
                        if(EFI_ERROR(status)) crashout(ST, L"Unable to get GOP mode in SetMode", status);

                        goto exit_boot_services;
                        break;
                    }
                    case 5: {
                        //view mem map
                        get_memory_map(ST, &memory_map_size, &memory_map_size_pages, &memory_map, &memory_map_key, &memory_map_descriptor_size, &memory_map_descriptor_version);
                        print_memory_map(ST, memory_map_size, memory_map, memory_map_key, memory_map_descriptor_size, memory_map_descriptor_version);
                        break;
                    }
                    case 6: {
                        //view config tables
                        print_config_tables(ST);
                        break;
                    }
                    case 7: {
                        //EFI shell
                        Print(L"EFI SHELL\r\n");
                        root = open_volume(ST, ImageHandle); //opens root of filesystem of boot device

                        wchar_t* filename = L"EFI\\BOOT\\SHELLX64.EFI";
                        EFI_PHYSICAL_ADDRESS shell_addr = load_file(ST, root, filename);
                        UINT64 size;
                        {
                            EFI_FILE_PROTOCOL* file;
                            file = open_file(root, filename);
                            size = get_file_size(ST, file, filename);
                        }

                        EFI_HANDLE shell_image = NULL;
                        status = uefi_call_wrapper(BS->LoadImage, 6, FALSE, ImageHandle, NULL, (VOID*)shell_addr, size, &shell_image);
                        if(EFI_ERROR(status)){
                            Print(L"Failed to Load EFI Shell image\r\n");
                            while(1);
                        }
                        status = uefi_call_wrapper(BS->StartImage, 3, shell_image, NULL, NULL);
                        if(EFI_ERROR(status)){
                            Print(L"Failed to Start EFI Shell image\r\n");
                            while(1);
                        }
                        break;
                    }
                    case 8: {
                        //shutdown
                        uefi_call_wrapper(ST->RuntimeServices->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);
                        break;
                    }
                    case 9: {
                        //Hard reset
                        triple_fault();
                        break;
                    }
                }



            }
        }
    }
    exit_boot_services:

    //construct kernel main function to call
    typedef void (*Kernel_entry)(KERNEL_CONTEXT_TABLE*);
    Kernel_entry kernel_main = (void*)loaded_addrs[0];

    //construct information for kernel

    //kernel context table
    KERNEL_CONTEXT_TABLE* ctx; //pointer pointing to copied context table
    UINTN ctx_size_bytes = sizeof(KERNEL_CONTEXT_TABLE);
    UINTN ctx_size_pages = (ctx_size_bytes + 4095)/4096;

    //allocate memory to copy ctx to
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, ctx_size_pages, &ctx);
    if(EFI_ERROR(status)){
        Print(L"Can't allocate %d pages for ctx\r\n", ctx_size_pages);
        while(1);
    }

    ctx->FirmwareVendor = ST->FirmwareVendor;
    ctx->FirmwareRevision = ST->FirmwareRevision;
    ctx->RuntimeServices = ST->RuntimeServices;

    ctx->MemoryMap = memory_map;
    ctx->MemoryMapSizeBytes = memory_map_size;
    ctx->MemoryMapSizePages = memory_map_size_pages;
    ctx->MemoryMapDescriptorSize = memory_map_descriptor_size;

    ctx->ConfigTable = ST->ConfigurationTable;
    ctx->ConfigTableEntriesCount = ST->NumberOfTableEntries;

    ctx->GOP = GOP->Mode;
    ctx->fb = backbuffer;

    ctx->kernelStack = kernel_stack;
    ctx->kernelStackSizePages = kernel_stack_size;

    ctx->kernelImageStart = loaded_addrs[0];
    ctx->kernelImageSizePages = kernel_image_size_pages;

    ctx->kernelPMMRange.start_addr = (uint8_t*)lowest_usable_range_addr;
    ctx->kernelPMMRange.bitmap = (uint8_t*)kernel_pmm_bitmap_addr;
    ctx->kernelPMMRange.bitmap_size_pages = kernel_pmm_bitmap_size;

    ctx->kernel_resource_addrs[0] = loaded_addrs[1];
    ctx->kernel_resource_addrs[1] = loaded_addrs[2];
    ctx->kernel_resource_addrs[2] = loaded_addrs[3];

    
    //get memory map and exit boot services
    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &memory_map_size, memory_map, &memory_map_key, &memory_map_descriptor_size, &memory_map_descriptor_version);
    uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, memory_map_key);


    //switch to kernel stack and go to start of kernel image
    asm volatile(
        ".intel_syntax noprefix\n"
        "mov rsp, %[stack_top]\n"
        "mov rdi, %[ctx_ptr]\n"
        "jmp %[kernel_entry]\n"
        ".att_syntax\n"
        :
        : [stack_top] "r"((uint64_t) kernel_stack),
        [ctx_ptr] "r"(ctx),
        [kernel_entry] "r"(kernel_main)
    );

    while(true);
    return EFI_SUCCESS;
}

void crashout(EFI_SYSTEM_TABLE* ST, wchar_t* error, EFI_STATUS code){
    EFI_INPUT_KEY key;
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);
    Print(L"Error: %s\r\nReason: %r\r\n", error, code);
    while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) != EFI_SUCCESS);
}

void early_display_setting(){
    EFI_INPUT_KEY key;
    EFI_STATUS status;
    Print(L"Reset display to standard VGA(80x25)(safe graphics)? (y/n/r):");
    while(true){
        //keyboard input
        if(uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &key) == EFI_SUCCESS){
            if(key.UnicodeChar == L'y'){
                status = uefi_call_wrapper(ST->ConOut->SetMode, 2, ST->ConOut, 0);
                if(EFI_ERROR(status)) crashout(ST, L"Failed to set ConOut mode in func early_display_setting, SetMode", status);
                break;
            }
            else if(key.UnicodeChar == L'n'){
                break;
            }
            else if(key.UnicodeChar == L'r'){
                status = uefi_call_wrapper(ST->ConOut->Reset, 2, ST->ConOut, false);
                if(EFI_ERROR(status)) crashout(ST, L"Failed to reset ConOut in func early_display_setting, Reset", status);
                break;
            }
        }
    }
}

void load_kernel_resources(EFI_SYSTEM_TABLE* ST, EFI_HANDLE IH, EFI_FILE_PROTOCOL** root, EFI_PHYSICAL_ADDRESS loaded_addrs[], uint64_t kernel_stack_size, EFI_PHYSICAL_ADDRESS* kernel_stack, uint32_t* kernel_size){
    //Load kernel and data
    Print(L"loading kernel and data\r\n");
    
    uint32_t kernel_stack_size_tmp = kernel_stack_size;
    *root = open_volume(ST, IH);
    

    //kernel stack size tmp becomes size of mem range file occupies
    loaded_addrs[0] = load_file_with_stack(ST, *root, L"kernel.bin", &kernel_stack_size_tmp);
    *kernel_size = kernel_stack_size_tmp;
    //calculate kernel stack top addr from this info
    *kernel_stack = loaded_addrs[0] + kernel_stack_size_tmp * 0x1000;

    loaded_addrs[1] = load_file(ST, *root, L"bad_apple.nvideo");
    loaded_addrs[2] = load_file(ST, *root, L"nuckos_logo.nvideo");
    loaded_addrs[3] = load_file(ST, *root, L"pointer.nvideo");

    Print(L"files loaded successfully\r\n");
}

void GOP_auto_select(EFI_SYSTEM_TABLE* ST, EFI_GRAPHICS_OUTPUT_PROTOCOL** GOP, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** GOP_info, UINTN* GOP_info_size, UINTN* selected_mode_num){
    EFI_STATUS status;
    EFI_BOOT_SERVICES* BS = ST->BootServices;
    EFI_GUID GOP_GUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    UINTN GOP_mode_count;
    UINTN GOP_native_mode_num;

    UINTN best_mode_num = 0;
    UINTN best_mode_pixel_count = 0;
    UINTN best_mode_width = 0;


    //auto select GOP
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &GOP_GUID, NULL, (void**) GOP);
    if(EFI_ERROR(status)) crashout(ST, L"Couldn't locate protocol GOP in LocateProtocol", status);
    status = uefi_call_wrapper((*GOP)->QueryMode, 4, *GOP, (*GOP)->Mode==NULL?0:(*GOP)->Mode->Mode, GOP_info_size, GOP_info);
    //get the current video mode
    if(status == EFI_NOT_STARTED){
        status = uefi_call_wrapper((*GOP)->SetMode, 2, *GOP, 0);
        if(EFI_ERROR(status)) crashout(ST, L"Cannot set GOP mode in SetMode", status);
    }
    if(EFI_ERROR(status)) crashout(ST, L"Unable to get GOP native mode in QueryMode", status);
    else{
        GOP_native_mode_num = (*GOP)->Mode->Mode;
        GOP_mode_count = (*GOP)->Mode->MaxMode;
    }
    Print(L"GOP native mode: %d\r\nGOP number of modes: %d\r\n", GOP_native_mode_num, GOP_mode_count);
    //query GOP modes
    for(UINTN i = 0;i<GOP_mode_count;i++){
        status = uefi_call_wrapper((*GOP)->QueryMode, 4, *GOP, i, GOP_info_size, GOP_info);
        if(EFI_ERROR(status)) crashout(ST, L"Could not query GOP mode in QueryMode", status);
        else{
            Print(L"mode %d: %dx%d format %x%s\r\n", i, (*GOP_info)->HorizontalResolution, (*GOP_info)->VerticalResolution, (*GOP_info)->PixelFormat, i == GOP_native_mode_num ? L"(current)" : L"");                  
        }
        if((*GOP_info)->PixelFormat != 1){
            Print(L"cannot be used\r\n");
            continue;
        }
    }


    for(UINTN i = 0;i<GOP_mode_count;i++){
        status = uefi_call_wrapper((*GOP)->QueryMode, 4, *GOP, i, GOP_info_size, GOP_info);
        if(EFI_ERROR(status)) crashout(ST, L"Cannot query GOP mode in QueryMode", status);
        if((*GOP_info)->PixelFormat != 1){
            continue;
        }
        UINTN pixelCount = (*GOP_info)->HorizontalResolution * (*GOP_info)->VerticalResolution;
        if(pixelCount > best_mode_pixel_count){
            best_mode_pixel_count = pixelCount;
            best_mode_num = i;
            best_mode_width = (*GOP_info)->HorizontalResolution;
        }
        else if(pixelCount == best_mode_pixel_count){
            if(best_mode_width > (*GOP_info)->HorizontalResolution){
                best_mode_num = i;
                best_mode_width = (*GOP_info)->HorizontalResolution;
            }
        }
    }
    status = uefi_call_wrapper((*GOP)->QueryMode, 4, *GOP, best_mode_num, GOP_info_size, GOP_info);
    if(EFI_ERROR(status)) crashout(ST, L"Can't query GOP mode in QueryMode", status);
    Print(L"Selected:\r\nmode %d: %dx%d format %x%s\r\n", best_mode_num, (*GOP_info)->HorizontalResolution, (*GOP_info)->VerticalResolution, (*GOP_info)->PixelFormat, best_mode_num == GOP_native_mode_num ? L"(current)" : L"");
    *selected_mode_num = best_mode_num;
}

void GOP_manual_select(EFI_SYSTEM_TABLE* ST, EFI_GRAPHICS_OUTPUT_PROTOCOL** GOP, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** GOP_info, UINTN* GOP_info_size, UINTN* selected_mode_num){
    EFI_STATUS status;
    EFI_INPUT_KEY key;
    EFI_BOOT_SERVICES* BS = ST->BootServices;
    EFI_GUID GOP_GUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    UINTN GOP_mode_count;
    UINTN GOP_native_mode_num;

    UINTN best_mode_num = 0;


    //manual select GOP
    //set GOP
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &GOP_GUID, NULL, (void**) GOP);
    if(EFI_ERROR(status)) crashout(ST, L"Couldn't locate protocol GOP in LocateProtocol", status);
    status = uefi_call_wrapper((*GOP)->QueryMode, 4, *GOP, (*GOP)->Mode==NULL?0:(*GOP)->Mode->Mode, GOP_info_size, GOP_info);
    //get the current video mode
    if(status == EFI_NOT_STARTED){
        status = uefi_call_wrapper((*GOP)->SetMode, 2, *GOP, 0);
        if(EFI_ERROR(status)) crashout(ST, L"Cannot set GOP mode in SetMode", status);
    }
    if(EFI_ERROR(status)) crashout(ST, L"Unable to get GOP Native Mode in QueryMode", status);
    else{
        GOP_native_mode_num = (*GOP)->Mode->Mode;
        GOP_mode_count = (*GOP)->Mode->MaxMode;
    }
    Print(L"GOP native mode: %d\r\nGOP number of modes: %d\r\n", GOP_native_mode_num, GOP_mode_count);
    //query GOP modes
    for(UINTN i = 0;i<GOP_mode_count;i++){
        status = uefi_call_wrapper((*GOP)->QueryMode, 4, *GOP, i, GOP_info_size, GOP_info);
        if(EFI_ERROR(status)) crashout(ST, L"Could not query GOP mode in QueryMode", status);
        
        else{
            Print(L"mode %d: %dx%d format %x%s  ", i, (*GOP_info)->HorizontalResolution, (*GOP_info)->VerticalResolution, (*GOP_info)->PixelFormat, i == GOP_native_mode_num ? L"(current)" : L"");                  
        }
        if((*GOP_info)->PixelFormat != 1){
            Print(L"cannot be used\r\n");
            continue;
        }
        else{
            Print(L"  press y to select:");
        }
        //prompt
        while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &key) != EFI_SUCCESS);
        if(key.UnicodeChar == L'y'){
            best_mode_num = i;
            status = uefi_call_wrapper((*GOP)->QueryMode, 4, *GOP, best_mode_num, GOP_info_size, GOP_info);
            if(EFI_ERROR(status)) crashout(ST, L"Could not query GOP mode in QueryMode", status);
        
            Print(L"\r\nSelected:\r\nmode %d: %dx%d format %x%s\r\n", best_mode_num, (*GOP_info)->HorizontalResolution, (*GOP_info)->VerticalResolution, (*GOP_info)->PixelFormat, best_mode_num == GOP_native_mode_num ? L"(current)" : L"");
            break;
        }
        else{
            Print(L"Nuh uh\r\n");
        }
    }
    *selected_mode_num = best_mode_num;
}

void display_refresh_entries(EFI_SYSTEM_TABLE* ST, wchar_t* menu_entries[], UINTN menu_number_of_entries, UINTN selected_menu_entry_index, UINTN start_column, UINTN start_row){
    EFI_STATUS status;
    //sets cursor position
    status = uefi_call_wrapper(ST->ConOut->SetCursorPosition, 3, ST->ConOut, start_column, start_row);
    if(EFI_ERROR(status)) crashout(ST, L"Can't set cursor position in func display_refresh_entries, SetCursorPosition", status);
    //sets font color
    status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to set font color in func display_refresh_entries, SetAttribute", status);
    
    for(UINTN entryIndex = 0;entryIndex < menu_number_of_entries;entryIndex++){ //print all menu entries
        if(entryIndex == selected_menu_entry_index){
            status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR_SELECTED);
            if(EFI_ERROR(status)) crashout(ST, L"Failed to set selected font color in func display_refresh_entries, SetAttribute", status);
        }
        Print(menu_entries[entryIndex]);
        status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);
        if(EFI_ERROR(status)) crashout(ST, L"Failed to reset font color in func display_refresh_entries, SetAttribute", status);
        Print(L"\r\n");
    }
}

EFI_PHYSICAL_ADDRESS load_file(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* root, wchar_t* filename){
    EFI_BOOT_SERVICES* BS = ST->BootServices;
    EFI_STATUS status;
    EFI_FILE_PROTOCOL* file;
    UINTN size;
    UINTN pages;
    EFI_PHYSICAL_ADDRESS addr;

    file = open_file(root, filename);
    size = get_file_size(ST, file, filename);

    // Round size up to nearest page
    pages = (size + 0xFFF) / 0x1000;

    // Allocate pages at address
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, pages, &addr);
    if(EFI_ERROR(status)) crashout(ST, L"Cannot allocate pages in func load_file, AllocatePages", status);

    // Read kernel binary into memory
    status = uefi_call_wrapper(file->Read, 3, file, &size, (void*)addr);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to read file in func load_file, Read", status);

    close_file(file);

    Print(L"%s loaded at address: %X\r\n", filename, addr);
    return addr;
}

//takes in stack size, returns (total size of the file + stack) in pages
EFI_PHYSICAL_ADDRESS load_file_with_stack(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* root, wchar_t* filename, uint32_t* stack_size){
    EFI_BOOT_SERVICES* BS = ST->BootServices;
    EFI_STATUS status;
    EFI_FILE_PROTOCOL* file;
    UINTN size;
    UINTN pages;
    EFI_PHYSICAL_ADDRESS addr;

    file = open_file(root, filename);
    size = get_file_size(ST, file, filename);

    //Round size up to nearest page
    pages = (size + 0xFFF) / 0x1000;
    //Add stack size
    pages += *stack_size;

    *stack_size = pages;
    // Allocate pages at address
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, pages, &addr);
    if(EFI_ERROR(status)) crashout(ST, L"Cannot allocate pages in func load_file, AllocatePages", status);

    // Read kernel binary into memory
    status = uefi_call_wrapper(file->Read, 3, file, &size, (void*)addr);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to read file in func load_file, Read", status);

    close_file(file);

    Print(L"%s loaded with stack at address: %X\r\n", filename, addr);
    return addr;
}

void close_file(EFI_FILE_PROTOCOL* file){
    EFI_STATUS status;
    status = uefi_call_wrapper(file->Close, 1, file);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to close file in func close_file, Close", status);
}

UINT64 get_file_size(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* file, wchar_t* filename){    
    EFI_STATUS status;
    EFI_BOOT_SERVICES* BS = ST->BootServices;
    EFI_FILE_INFO* info;
    UINT64 ret;

    info = LibFileInfo(file);
    ret = info->FileSize;
    status = uefi_call_wrapper(BS->FreePool, 1, info);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to free FileInfo pool in func get_file_size, FreePool", status);

    Print(filename);
    Print(L" file size: ");
    if(ret >= 1024*1024){
        Print(L"%f MiB/%f MB ", ret/(1048576.0f), ret/(1000000.0f));
    }
    else if(ret >= 1024){
        Print(L"%f KiB/%f KB ", ret/(1024.0f), ret/(1000.0f));
    }
    Print(L"%f pages/%d bytes\r\n", (float)((ret+0xFFF)/0x1000), ret);
    return ret;
}

EFI_FILE_PROTOCOL* open_file(EFI_FILE_PROTOCOL* volume, CHAR16* filename){
    EFI_STATUS status;
    EFI_FILE_PROTOCOL* file; //holds file

    status = uefi_call_wrapper(volume->Open, 5, volume, &file, filename, EFI_FILE_MODE_READ, 0); //no need for attributes, only for creating files
    if(EFI_ERROR(status)) crashout(ST, L"Failed to open file in func open_file, Open", status);
    return file;
}

EFI_FILE_PROTOCOL* open_volume(EFI_SYSTEM_TABLE* ST, EFI_HANDLE IH){
    //image interface
    EFI_STATUS status;
    EFI_BOOT_SERVICES* BS = ST->BootServices;
    EFI_LOADED_IMAGE* loadedImage = NULL; //stores info about current uefi app + disk volume
    EFI_FILE_IO_INTERFACE* fsInterface; 
    EFI_FILE_PROTOCOL* volume;

    EFI_GUID imgGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

    //get loaded image info, puts info into loadedImage
    status = uefi_call_wrapper(BS->HandleProtocol, 3, IH, &imgGuid, (void**)&loadedImage);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to get current loaded image info in func open_volume, HandleProtocol", status);
    //get volume handle, gets fs from the disk
    status = uefi_call_wrapper(BS->HandleProtocol, 3, loadedImage->DeviceHandle, &fsGuid, (void*)&fsInterface);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to get fs from disk(device handle) in func open_volume, HandleProtocol", status);
    //open root of the filesystem
    status = uefi_call_wrapper(fsInterface->OpenVolume, 2, fsInterface, &volume);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to open fs root in func open_volume, open_volume", status);
    return volume;
}

void print_config_tables(EFI_SYSTEM_TABLE* ST){
    EFI_GUID GUIDTableKeys[] = {
        {0x8868e871,0xe4f1,0x11d3,{0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}},
        {0xeb9d2d30,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}},
        {0xeb9d2d32,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}},
        {0xeb9d2d31,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}},
        {0xf2fd1544,0x9794,0x4a2c,{0x99,0x2e,0xe5,0xbb,0xcf,0x20,0xe3,0x94}},
        {0xeb9d2d2f,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}},
        {0x87367f87,0x1119,0x41ce,{0xaa,0xec,0x8b,0xe0,0x11,0x1f,0x55,0x8a}},
        {0x35e7a725,0x8dd2,0x4cac,{0x80,0x11,0x33,0xcd,0xa8,0x10,0x90,0x56}},
        {0xdbc461c3,0xb3de,0x422a,{0xb9,0xb4,0x98,0x86,0xfd,0x49,0xa1,0xe5}},
        {0xb1b621d5,0xf19c,0x41a5,{0x83,0x0b,0xd9,0x15,0x2c,0x69,0xaa,0xe0}},
        {0xeb66918a,0x7eef,0x402a,{0x84,0x2e,0x93,0x1d,0x21,0xc3,0x8a,0xe9}},
        {0xdcfa911d,0x26eb,0x469f,{0xa2,0x20,0x38,0xb7,0xdc,0x46,0x12,0x20}},
        {0x36122546,0xf7e7,0x4c8f,{0xbd,0x9b,0xeb,0x85,0x25,0xb5,0x0c,0x0b}},
        {0x523c91af,0xa195,0x4382,{0x81,0x8d,0x29,0x5f,0xe4,0x00,0x64,0x65}},
        {0x0de9f0ec,0x88b6,0x428f,{0x97,0x7a,0x25,0x8f,0x1d,0x0e,0x5e,0x72}},
        {0x49152E77,0x1ADA,0x4764,{0xB7,0xA2,0x7A,0xFE,0xFE,0xD9,0x5E,0x8B}},
        {0xb122a263,0x3661,0x4f68,{0x99,0x29,0x78,0xf8,0xb0,0xd6,0x21,0x80}},
        {0xd719b2cb,0x3d3a,0x4596,{0xa3,0xbc,0xda,0xd0,0x0e,0x67,0x65,0x6f}}
    };
    wchar_t* GUIDTableValues[] = {
        L"ACPI 2.0 TABLE",
        L"ACPI TABLE",
        L"SAL SYSTEM TABLE",
        L"SMBIOS TABLE",
        L"SMBIOS3 TABLE",
        L"MPS TABLE",
        L"JSON CONFIG DATA TABLE",
        L"JSON CAPSULE DATA TABLE",
        L"JSON CAPSULE RESULT TABLE",
        L"DTB TABLE",
        L"RT PROPERTIES TABLE",
        L"MEMORY ATTRIBUTES TABLE",
        L"CONFORMANCE PROFILE TABLE",
        L"CONFORMANCE PROFILES UEFI SPEC",
        L"MEMORY RANGE CAPSULE",
        L"DEBUG IMAGE INFO TABLE",
        L"SYSTEM RESOURCE TABLE"
        L"IMAGE SECURITY DATABASE"
    };
    EFI_CONFIGURATION_TABLE table;
    Print(L"config table entries ptr: %p\r\n", ST->ConfigurationTable);

    Print(L"Number of configuration table entries: %d\r\n", ST->NumberOfTableEntries);
    while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, NULL) != EFI_SUCCESS);

    for(UINTN entry = 0;entry < ST->NumberOfTableEntries;entry++){
        table = ST->ConfigurationTable[entry];
        Print(L"Table #%d:", entry+1);
        print_GUID(&table.VendorGuid);
        for(UINTN guidIndex = 0;guidIndex < (sizeof(GUIDTableKeys)/sizeof(GUIDTableKeys[0]));guidIndex++){
            if(compare_GUID(&table.VendorGuid, &GUIDTableKeys[guidIndex])){
                Print(L"  ");
                Print(GUIDTableValues[guidIndex]);
                break;
            }
        }
        Print(L"  ptr: %p\r\n", table.VendorTable);
        while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, NULL) != EFI_SUCCESS);
    }
    Print(L"----------END----------\r\n");
}

void print_memory_map(EFI_SYSTEM_TABLE* ST, UINTN memory_map_size, EFI_MEMORY_DESCRIPTOR* memory_map, UINTN memory_map_key, UINTN memory_map_descriptor_size, UINT32 memory_map_descriptor_version){
    EFI_STATUS status;
    EFI_INPUT_KEY key;
    UINT16* type_arr[] = {
    L"EfiReservedMemoryType",
    L"EfiLoaderCode", //yes
    L"EfiLoaderData", //for safety best to avoid - might contain memory map
    L"EfiBootServicesCode", //yes
    L"EfiBootServicesData", //yes
    L"EfiRuntimeServicesCode",
    L"EfiRuntimeServicesData",
    L"EfiConventionalMemory", //yes
    L"EfiUnusableMemory",
    L"EfiACPIReclaimMemory",
    L"EfiACPIMemoryNVS",
    L"Efimemory_mappedIO",
    L"Efimemory_mappedIOPortSpace",
    L"EfiPalCode",
    L"EfiPersistentMemory", //yes
    L"EfiUnacceptedMemoryType",
    L"EfiMaxMemoryType"
    };

    UINTN entries = memory_map_size / memory_map_descriptor_size;
    EFI_MEMORY_DESCRIPTOR* MM = memory_map;
    
    //size of conventional memory in number of 4 KiB pages
    UINTN totalMapped = 0;
    UINTN totalUsable = 0;
    UINTN totalConventional = 0;

    //print other info
    Print(L"\r\nMemory Map Size: %lu\r\nMap Key: %lu\r\nSize of each entry: %lu\r\nVer: %u\r\nTotal entries: %lu\r\n", memory_map_size, memory_map_key, memory_map_descriptor_size, memory_map_descriptor_version, entries);
    Print(L"----------START----------\r\n");
    for(UINTN i = 0;i < entries;i++){
        Print(L"#%lu - ", i+1);
        
        if(MM->Type < sizeof(type_arr)/sizeof(type_arr[0])){
            //add to mem size counters
            totalMapped += MM->NumberOfPages;
            if(MM->Type == EfiConventionalMemory || MM->Type == EfiPersistentMemory){
                status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_GREEN, EFI_GREEN));
                if(EFI_ERROR(status)) crashout(ST, L"Failed to set ConOut attribute in func print_memory_map, SetAttribute", status);
                totalUsable += MM->NumberOfPages;
            }
            else if(MM->Type == EfiLoaderCode || MM->Type == EfiLoaderData){
                status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_BLUE, EFI_BLUE));
                if(EFI_ERROR(status)) crashout(ST, L"Failed to set ConOut attribute in func print_memory_map, SetAttribute", status);
            }
            else{
                status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_RED, EFI_RED));
                if(EFI_ERROR(status)) crashout(ST, L"Failed to set ConOut attribute in func print_memory_map, SetAttribute", status);
            }
            if(MM->Type == EfiConventionalMemory){
                totalConventional += MM->NumberOfPages;
            }
            Print(L" ");
            //reset color
            status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);
            if(EFI_ERROR(status)) crashout(ST, L"Failed to set ConOut attribute in func print_memory_map, SetAttribute", status);
            Print(L"%s ", type_arr[MM->Type]);
        }
        else{
            status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, BACKGROUND_COLOR);
            if(EFI_ERROR(status)) crashout(ST, L"Failed to set ConOut attribute in func print_memory_map, SetAttribute", status);
            Print(L" ");
            //reset color
            status = uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);
            if(EFI_ERROR(status)) crashout(ST, L"Failed to set ConOut attribute in func print_memory_map, SetAttribute", status);
            Print(L"0x%x ", MM->Type);
        }
        Print(L" ");

        Print(L"Range:0x%lx - 0x%lx ", MM->PhysicalStart, (MM->PhysicalStart + (MM->NumberOfPages*4096) - 1));
        if(MM->Attribute & 0x1)Print(L"UC ");
        if(MM->Attribute & 0x2)Print(L"WC ");
        if(MM->Attribute & 0x4)Print(L"WT ");
        if(MM->Attribute & 0x8)Print(L"WB ");
        if(MM->Attribute & 0x10)Print(L"UCE ");
        if(MM->Attribute & 0x1000)Print(L"WP ");
        if(MM->Attribute & 0x2000)Print(L"RP ");
        if(MM->Attribute & 0x4000)Print(L"XP ");
        if(MM->Attribute & 0x8000)Print(L"NV ");
        if(MM->Attribute & 0x10000)Print(L"MORE_RELIABLE ");
        if(MM->Attribute & 0x20000)Print(L"RO ");
        if(MM->Attribute & 0x40000)Print(L"SP ");
        if(MM->Attribute & 0x80000)Print(L"CRYPTO ");
        if(MM->Attribute & 0x8000000000000000)Print(L"RUNTIME ");
        if(MM->Attribute & 0x4000000000000000)Print(L"ISA_VALID ");
        if(MM->Attribute & 0x0FFFF00000000000)Print(L"ISA_MASK ");

        Print(L"\r\n");
        //go to next one
        MM = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MM + memory_map_descriptor_size);
        //break here
        while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) != EFI_SUCCESS);
    }
    Print(L"\r\n----------END----------\r\n");
    Print(L"Total mapped memory: %d pages/%f GB/%f GiB\r\n", totalMapped, totalMapped/250000.0f, totalMapped/262144.0f);
    Print(L"Total usable memory: %d pages/%f GB/%f GiB\r\n", totalUsable, totalUsable/250000.0f, totalUsable/262144.0f);
    Print(L"Total conventional memory: %d pages/%f GB/%f GiB\r\n", totalConventional, totalConventional/250000.0f, totalConventional/262144.0f);

}

void get_memory_map_highlow_address(UINTN memory_map_size, EFI_MEMORY_DESCRIPTOR* memory_map, UINTN memory_map_descriptor_size, EFI_PHYSICAL_ADDRESS* low, EFI_PHYSICAL_ADDRESS* high){
    UINTN entries = memory_map_size / memory_map_descriptor_size;
    EFI_MEMORY_DESCRIPTOR* MM = memory_map;
    
    //highlow est addr of range conventional memory
    EFI_PHYSICAL_ADDRESS lowest_addr = -1;
    EFI_PHYSICAL_ADDRESS highest_addr = 0;

    for(UINTN i = 0;i < entries;i++){
        if(MM->Type == EfiConventionalMemory || MM->Type == EfiPersistentMemory){
            if(MM->PhysicalStart < lowest_addr){
                lowest_addr = MM->PhysicalStart;
            }
            if((MM->PhysicalStart + (MM->NumberOfPages*4096)) > highest_addr){
                highest_addr = (MM->PhysicalStart + (MM->NumberOfPages*4096));
            }
        }  
        //go to next one
        MM = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MM + memory_map_descriptor_size);
    }
    uint64_t totalRangeSize = (highest_addr-lowest_addr);
    Print(L"Total managed memory range: 0x%lx - 0x%lx, %f pages/%f GB/%f GiB\r\n", lowest_addr, highest_addr, totalRangeSize/4096.0f, totalRangeSize/1000000000.0f, totalRangeSize/1073741824.0f);

    *low = lowest_addr;
    *high = highest_addr;

}

void get_memory_map(EFI_SYSTEM_TABLE* ST, UINTN* memory_map_size, UINTN* memory_map_size_pages, EFI_MEMORY_DESCRIPTOR** memory_map, UINTN* memory_map_key, UINTN* memory_map_descriptor_size, UINT32* memory_map_descriptor_version){
    EFI_STATUS status;
    EFI_BOOT_SERVICES* BS = ST->BootServices;

    if((*memory_map) != NULL){
        Print(L"Freeing previous memory map at %X...\r\n", *memory_map);
        status = uefi_call_wrapper(BS->FreePages, 2, *memory_map, *memory_map_size_pages);
        if(EFI_ERROR(status)) crashout(ST, L"Error freeing previously allocated memory map in func get_memory_map, FreePages", status);
    }

    *memory_map_size = 0;
    //get memory size of memory map
    status = uefi_call_wrapper(BS->GetMemoryMap, 5, memory_map_size, NULL, memory_map_key, memory_map_descriptor_size, memory_map_descriptor_version); //all are type*
    if(status != EFI_BUFFER_TOO_SMALL){
        if(EFI_ERROR(status)) crashout(ST, L"Error when getting size of memory map + reason is not because buffer too small, in func get_memory_map, GetMemoryMap", status);
    }
    *memory_map_size += (*memory_map_descriptor_size) * 20; //20 more entries
    *memory_map_size_pages = ((*memory_map_size) + 0xFFF)/0x1000;
    //allocate pages for memory map
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, *memory_map_size_pages, (void**)memory_map); //here memory_map is a void**
    if(EFI_ERROR(status)){
        Print(L"Failed to allocate %d bytes -> %d pages! for Memory Map\r\n", (*memory_map_size), (*memory_map_size_pages));
        while(1);
    }
    *memory_map_size += (*memory_map_descriptor_size) * 5;
    //get memory map
    status = uefi_call_wrapper(BS->GetMemoryMap, 5, memory_map_size, *memory_map, memory_map_key, memory_map_descriptor_size, memory_map_descriptor_version);
    if(EFI_ERROR(status)) crashout(ST, L"Failed when getting memory map in func get_memory_map, GetMemoryMap", status);
}

void print_logo(EFI_SYSTEM_TABLE* ST){
    EFI_STATUS status;
    //CHAR16* oslogo = L"                                   _   _    ___\r\n                                  | | | |  / _ \\\r\n    _   _                  _      | |_| | |  __/\r\n   | \\ | |  _   _    ___  | | __   \\__,_|  \\___|   / _ \\  / ___| \r\n   |  \\| | | | | |  / __| | |/ /      __   _      | | | | \\___ \\ \r\n   | |\\  | | |_| | | (__  |   <      / _| (_)     | |_| |  ___) |\r\n   |_| \\_|  \\__,_|  \\___| |_|\\_\\    | |_  | |      \\___/  |____/ \r\n                                    |  _| | |                    \r\n                                    |_|   |_|                    \r\n               \"operating system of the future\" (TM)\r\n";
    CHAR16* oah = L"                                   _   _    ___\r\n                                  | | | |  / _ \\\r\n    _   _                  _      | |_| | |  __/   ____              _\r\n   | \\ | |  _   _    ___  | | __   \\__,_|  \\___|  | __ )  ___   ___ | |\r\n   |  \\| | | | | |  / __| | |/ /      __   _      |  _ \\ / _ \\ / _ \\| __| \r\n   | |\\  | | |_| | | (__  |   <      / _| (_)     | |_) | (_) | (_) | |_\r\n   |_| \\_|  \\__,_|  \\___| |_|\\_\\    | |_  | |     |____/ \\___/ \\___/ \\__|\r\n                                    |  _| | |\r\n                                    |_|   |_|\r\n                   \"operating system of the future\" (TM)\r\n";
    status = uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, oah);
    if(EFI_ERROR(status)) crashout(ST, L"Failed to print the thing in func print_logo, OutputString", status);
}

void print_info(EFI_SYSTEM_TABLE* ST){
    Print(L"Firmware Vendor: %s\r\n", ST->FirmwareVendor);
    Print(L"System UEFI firmware revision: %d.%d\r\n", (ST->FirmwareRevision >> 16) && 0xFFFF, ST->FirmwareRevision & 0xFFFF);

    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_BLACK, EFI_BLACK));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_BLUE, EFI_BLUE));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_GREEN, EFI_GREEN));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_CYAN, EFI_CYAN));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_RED, EFI_RED));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_MAGENTA, EFI_MAGENTA));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_BROWN, EFI_BROWN));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_LIGHTGRAY));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_DARKGRAY, EFI_DARKGRAY));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTBLUE, EFI_LIGHTBLUE));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_LIGHTGREEN));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_LIGHTCYAN));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTRED, EFI_LIGHTRED));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTMAGENTA, EFI_LIGHTMAGENTA));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_YELLOW));
    Print(L"0");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_WHITE));
    Print(L"0\r\n");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);
}

void print_GUID(EFI_GUID* guid){
    Print(L"GUID: %x-%x-%x-", guid->Data1, guid->Data2, guid->Data3);
    Print(L"%x-%x-%x-%x-%x-%x-%x-%x",
        guid->Data4[0],
        guid->Data4[1],
        guid->Data4[2],
        guid->Data4[3],
        guid->Data4[4],
        guid->Data4[5],
        guid->Data4[6],
        guid->Data4[7]
    );
}

uint8_t compare_GUID(EFI_GUID* guid1, EFI_GUID* guid2){
    if(guid1->Data1 != guid2->Data1)return 0;
    if(guid1->Data2 != guid2->Data2)return 0;
    if(guid1->Data3 != guid2->Data3)return 0;
    for(uint8_t c = 0;c < 8;c++){
        if(guid1->Data4[c] != guid2->Data4[c])return 0;
    }
    return 1;
}

void triple_fault(){
    uint64_t egg = 0;
    asm volatile (
        ".intel_syntax noprefix\n"
        "lidt [%[eggman]]\n"
        ".byte 0x0F, 0x0B\n" //invalid opcode(fault), invalid idt(double fault), exception handler not found(triple fault)
        ".att_syntax\n"
        :
        : [eggman] "r"(&egg)
    );
}

