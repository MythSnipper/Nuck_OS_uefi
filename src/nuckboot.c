#include "../include/nuckboot.h"



EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
    EFI_SYSTEM_TABLE* ST = SystemTable;
    EFI_INPUT_KEY key;
    EFI_STATUS status;

    wchar_t buff[2] = L"\0\0";

    InitializeLib(ImageHandle, ST); //initialize runtime pointers
    really_early_start:
    //disable watchdog timer
    uefi_call_wrapper(ST->BootServices->SetWatchdogTimer, 4, 0, 0x10000, 0, NULL);

    //simple keyboard loop to determine to reset or not to reset the display
    Print(L"Reset display to standard VGA(80x25)(safe graphics)? (y/n/r):");
    while(true){
        //keyboard input
        if(uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &key) == EFI_SUCCESS){
            if(key.UnicodeChar == L'y'){
                uefi_call_wrapper(ST->ConOut->SetMode, 2, ST->ConOut, 0);
                break;
            }
            else if(key.UnicodeChar == L'n'){
                break;
            }
            else if(key.UnicodeChar == L'r'){
                uefi_call_wrapper(ST->ConOut->Reset, 2, ST->ConOut, false);
                break;
            }
        }
    }

    //variables used in main logic
    UINTN MemoryMapSize = 0; //size of the memory map in bytes
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    //used to store the volume on the disk
    EFI_FILE_PROTOCOL* root;

    EFI_PHYSICAL_ADDRESS kernel;
    EFI_PHYSICAL_ADDRESS bad_apple;
    EFI_PHYSICAL_ADDRESS nuckos_logo;
    EFI_PHYSICAL_ADDRESS pointer_icon;

    //GOP variables
    EFI_GUID GOPGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* GOP;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* GOPInfo;
    UINTN GOPInfoSize;
    UINTN GOPNumModes;
    UINTN GOPNativeMode;

    UINTN bestModeNum = 0;
    UINTN bestModePixelCount = 0;
    UINTN bestModeWidth = 0;
    
    EFI_PHYSICAL_ADDRESS fb2_addr; //backbuffer
    EFI_PHYSICAL_ADDRESS kernel_stack; //kernel stack(not top, start)
    uint64_t kernel_stack_size = 512; //size in pages, 2MiB

    EFI_PHYSICAL_ADDRESS kernel_heap_map; //kernel heap bitmap(1 page)
    EFI_PHYSICAL_ADDRESS kernel_heap; //kernel heap(4096 pages)

    //DISPLAY MENU data
    UINTN startColumn; //starting column and row
    UINTN startRow;
    wchar_t* menuEntries[] = {
        L"Automatic boot",
        L"Load Nuck OS kernel and data",
        L"Select GOP mode(auto)",
        L"Select GOP mode(manual)",
        L"Boot Nuck OS",
        L"View memory map",
        L"View configuration tables",
        L"EFI Shell",
        L"Shutdown",
    };
    UINTN menuEntriesCount = 9;
    UINTN selectedEntryIndex = 0;

    
    //clear console output, sets background color, cursor goes to 0, 0
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, BACKGROUND_COLOR);
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

    //sets font color
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);

    printLogo(ST);
    printInfo(ST);

    //sets font color
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);

    Print(L"Press any key multiple times to continue...");
    for(uint8_t i = 0;i < 3;i++){
        while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) != EFI_SUCCESS);
    }
    bootloader_start:

    selectedEntryIndex = 0;
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, BACKGROUND_COLOR);
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

    //sets font color
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);

    Print(L"---Boot Menu---\r\n");
    Print(L"   Arrow keys/WASD to move, Enter to select\r\n   Esc to reset bootloader, F12 to RESET bootloader\r\n\n");
    //save current cursor position because it's where the boot menu text starts
    startColumn = ST->ConOut->Mode->CursorColumn;
    startRow = ST->ConOut->Mode->CursorRow;
    refreshEntries(ST, menuEntries, menuEntriesCount, selectedEntryIndex, startColumn, startRow);

    while(true){
        //keyboard input
        if(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) == EFI_SUCCESS){
            
            //Basic text output
            /*
            if(key.UnicodeChar == L'\r'){
                uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"\r\n"); 
            }
            else{
                buff[0] = key.UnicodeChar;
                uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, buff);
            }
            */
            //If it's F12 then really early start
            if(key.ScanCode == SCAN_F12){
                goto really_early_start;
            }
            //If it's escape then reset bootloader
            if(key.ScanCode == SCAN_ESC){
                goto bootloader_start;
            }

            //Check if it's arrow keys
            if(key.ScanCode == SCAN_UP || key.ScanCode == SCAN_DOWN || key.UnicodeChar == L'w' || key.UnicodeChar == L's'){
                //Update index
                if(key.ScanCode == SCAN_UP || key.UnicodeChar == L'w'){
                    if(selectedEntryIndex > 0)selectedEntryIndex--;
                }
                if(key.ScanCode == SCAN_DOWN || key.UnicodeChar == L's'){
                    if(selectedEntryIndex < menuEntriesCount-1)selectedEntryIndex++;
                }
                //refresh entries display
                refreshEntries(ST, menuEntries, menuEntriesCount, selectedEntryIndex, startColumn, startRow);
            }

            //Check if it's Enter
            if(key.UnicodeChar == CHAR_CARRIAGE_RETURN){
                switch(selectedEntryIndex){
                    case 0: {
                        //Auto boot
                        Print(L"loading kernel and data\r\n");
                        root = openVolume(ST, ImageHandle); //opens root of filesystem of boot device
                        
                        kernel = loadFile(ST, root, L"kernel.bin");
                        bad_apple = loadFile(ST, root, L"bad_apple.nvideo");
                        nuckos_logo = loadFile(ST, root, L"nuckos_logo.nvideo");
                        pointer_icon = loadFile(ST, root, L"pointer.nvideo");

                        Print(L"files loaded successfully\r\n");
                        status = uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &GOPGuid, NULL, (void**) &GOP);
                        if(EFI_ERROR(status)){
                            Print(L"No GOP\r\n");
                            while(1);
                        }
                        status = uefi_call_wrapper(GOP->QueryMode, 4, GOP, GOP->Mode==NULL?0:GOP->Mode->Mode, &GOPInfoSize, &GOPInfo);
                        //get the current video mode
                        if(status == EFI_NOT_STARTED){
                            status = uefi_call_wrapper(GOP->SetMode, 2, GOP, 0);
                        }
                        if(EFI_ERROR(status)){
                            Print(L"Unable to get GOP native mode\r\n");
                        }
                        else{
                            GOPNativeMode = GOP->Mode->Mode;
                            GOPNumModes = GOP->Mode->MaxMode;
                        }
                        Print(L"GOP native mode: %d\r\nGOP number of modes: %d\r\n", GOPNativeMode, GOPNumModes);
                        //query GOP modes
                        for(UINTN i = 0;i<GOPNumModes;i++){
                            status = uefi_call_wrapper(GOP->QueryMode, 4, GOP, i, &GOPInfoSize, &GOPInfo);
                            if(EFI_ERROR(status)){
                                Print(L"Get mode %d failed!", i);
                            }
                            else{
                                Print(L"mode %d: %dx%d format %x%s\r\n", i, GOPInfo->HorizontalResolution, GOPInfo->VerticalResolution, GOPInfo->PixelFormat, i == GOPNativeMode ? L"(current)" : L"");                  
                            }
                        }
                        bestModeNum = 0;
                        bestModePixelCount = 0;
                        bestModeWidth = 0;
                        for(UINTN i = 0;i<GOPNumModes;i++){
                            uefi_call_wrapper(GOP->QueryMode, 4, GOP, i, &GOPInfoSize, &GOPInfo);
                            if(GOPInfo->PixelFormat != 1){
                                continue;
                            }
                            UINTN pixelCount = GOPInfo->HorizontalResolution * GOPInfo->VerticalResolution;
                            if(pixelCount > bestModePixelCount){
                                bestModePixelCount = pixelCount;
                                bestModeNum = i;
                                bestModeWidth = GOPInfo->HorizontalResolution;
                            }
                            else if(pixelCount == bestModePixelCount){
                                if(bestModeWidth > GOPInfo->HorizontalResolution){
                                    bestModeNum = i;
                                    bestModeWidth = GOPInfo->HorizontalResolution;
                                }
                            }
                        }
                        uefi_call_wrapper(GOP->QueryMode, 4, GOP, bestModeNum, &GOPInfoSize, &GOPInfo);
                        Print(L"Selected:\r\nmode %d: %dx%d format %x%s\r\n", bestModeNum, GOPInfo->HorizontalResolution, GOPInfo->VerticalResolution, GOPInfo->PixelFormat, bestModeNum == GOPNativeMode ? L"(current)" : L"");
                        uefi_call_wrapper(GOP->QueryMode, 4, GOP, bestModeNum, &GOPInfoSize, &GOPInfo);
                        //Set GOP mode
                        status = uefi_call_wrapper(GOP->SetMode, 2, GOP, bestModeNum);
                        if(EFI_ERROR(status)){
                            Print(L"Unable to set GOP mode %d\r\n", bestModeNum);
                            while(1);
                        }

                        //allocate memory for backbuffer
                        status = uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, GOP->Mode->FrameBufferSize, &fb2_addr);
                        if(EFI_ERROR(status)){
                            Print(L"Can't allocate pool of %d bytes for video backbuffer\r\n", GOP->Mode->FrameBufferSize);
                            while(1);
                        }
                        //allocate memory for kernel stack(2 MiB)
                        status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, kernel_stack_size, &kernel_stack);
                        if(EFI_ERROR(status)){
                            Print(L"Can't allocate %d pages for kernel stack\r\n", kernel_stack_size);
                            while(1);
                        }
                        //allocate memory for kernel heap map(1 page = 4096 bytes = maps to 32768 pages)
                        status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, 1, &kernel_heap_map);
                        if(EFI_ERROR(status)){
                            Print(L"Can't allocate %d pages for kernel heap map\r\n", 1);
                            while(1);
                        }
                        //allocate memory for kernel heap(32768 pages = 1342117728 bytes = 128 MiB heap)
                        status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, 32768, &kernel_heap);
                        if(EFI_ERROR(status)){
                            Print(L"Can't allocate %d pages for kernel heap\r\n", 32768);
                            while(1);
                        }

                        //get memory map
                        status = uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, &MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
                        uefi_call_wrapper(ST->BootServices->ExitBootServices, 2, ImageHandle, MapKey);
                        goto exit_boot_services;
                        break;
                    }
                    case 1: {
                        //Load kernel and data
                        Print(L"loading kernel and data\r\n");
                        root = openVolume(ST, ImageHandle); //opens root of filesystem of boot device
                        
                        kernel = loadFile(ST, root, L"kernel.bin");
                        bad_apple = loadFile(ST, root, L"bad_apple.nvideo");
                        nuckos_logo = loadFile(ST, root, L"nuckos_logo.nvideo");
                        pointer_icon = loadFile(ST, root, L"pointer.nvideo");

                        Print(L"files loaded successfully\r\n");
                        break;
                    }
                    case 2: {
                        //auto select GOP
                        //set GOP
                        status = uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &GOPGuid, NULL, (void**) &GOP);
                        if(EFI_ERROR(status)){
                            Print(L"No GOP\r\n");
                            while(1);
                        }
                        status = uefi_call_wrapper(GOP->QueryMode, 4, GOP, GOP->Mode==NULL?0:GOP->Mode->Mode, &GOPInfoSize, &GOPInfo);
                        //get the current video mode
                        if(status == EFI_NOT_STARTED){
                            status = uefi_call_wrapper(GOP->SetMode, 2, GOP, 0);
                        }
                        if(EFI_ERROR(status)){
                            Print(L"Unable to get GOP native mode\r\n");
                        }
                        else{
                            GOPNativeMode = GOP->Mode->Mode;
                            GOPNumModes = GOP->Mode->MaxMode;
                        }
                        Print(L"GOP native mode: %d\r\nGOP number of modes: %d\r\n", GOPNativeMode, GOPNumModes);
                        //query GOP modes
                        for(UINTN i = 0;i<GOPNumModes;i++){
                            status = uefi_call_wrapper(GOP->QueryMode, 4, GOP, i, &GOPInfoSize, &GOPInfo);
                            if(EFI_ERROR(status)){
                                Print(L"Get mode %d failed!", i);
                            }
                            else{
                                Print(L"mode %d: %dx%d format %x%s\r\n", i, GOPInfo->HorizontalResolution, GOPInfo->VerticalResolution, GOPInfo->PixelFormat, i == GOPNativeMode ? L"(current)" : L"");                  
                            }
                        }
                        bestModeNum = 0;
                        bestModePixelCount = 0;
                        bestModeWidth = 0;
                        for(UINTN i = 0;i<GOPNumModes;i++){
                            uefi_call_wrapper(GOP->QueryMode, 4, GOP, i, &GOPInfoSize, &GOPInfo);
                            if(GOPInfo->PixelFormat != 1){
                                continue;
                            }
                            UINTN pixelCount = GOPInfo->HorizontalResolution * GOPInfo->VerticalResolution;
                            if(pixelCount > bestModePixelCount){
                                bestModePixelCount = pixelCount;
                                bestModeNum = i;
                                bestModeWidth = GOPInfo->HorizontalResolution;
                            }
                            else if(pixelCount == bestModePixelCount){
                                if(bestModeWidth > GOPInfo->HorizontalResolution){
                                    bestModeNum = i;
                                    bestModeWidth = GOPInfo->HorizontalResolution;
                                }
                            }
                        }
                        uefi_call_wrapper(GOP->QueryMode, 4, GOP, bestModeNum, &GOPInfoSize, &GOPInfo);
                        Print(L"Selected:\r\nmode %d: %dx%d format %x%s\r\n", bestModeNum, GOPInfo->HorizontalResolution, GOPInfo->VerticalResolution, GOPInfo->PixelFormat, bestModeNum == GOPNativeMode ? L"(current)" : L"");
                        break;
                    }
                    case 3: {
                        //manual select GOP
                        //set GOP
                        status = uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &GOPGuid, NULL, (void**) &GOP);
                        if(EFI_ERROR(status)){
                            Print(L"No GOP\r\n");
                            while(1);
                        }
                        status = uefi_call_wrapper(GOP->QueryMode, 4, GOP, GOP->Mode==NULL?0:GOP->Mode->Mode, &GOPInfoSize, &GOPInfo);
                        //get the current video mode
                        if(status == EFI_NOT_STARTED){
                            status = uefi_call_wrapper(GOP->SetMode, 2, GOP, 0);
                        }
                        if(EFI_ERROR(status)){
                            Print(L"Unable to get GOP native mode\r\n");
                        }
                        else{
                            GOPNativeMode = GOP->Mode->Mode;
                            GOPNumModes = GOP->Mode->MaxMode;
                        }
                        Print(L"GOP native mode: %d\r\nGOP number of modes: %d\r\n", GOPNativeMode, GOPNumModes);
                        //query GOP modes
                        for(UINTN i = 0;i<GOPNumModes;i++){
                            status = uefi_call_wrapper(GOP->QueryMode, 4, GOP, i, &GOPInfoSize, &GOPInfo);
                            if(EFI_ERROR(status)){
                                Print(L"Get mode %d failed!", i);
                            }
                            else{
                                Print(L"mode %d: %dx%d format %x%s  ", i, GOPInfo->HorizontalResolution, GOPInfo->VerticalResolution, GOPInfo->PixelFormat, i == GOPNativeMode ? L"(current)" : L"");                  
                            }
                            if(GOPInfo->PixelFormat != 1){
                                Print(L"cannot be used\r\n");
                                continue;
                            }
                            else{
                                Print(L"  press y to select:");
                            }
                            //prompt
                            while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &key) != EFI_SUCCESS);
                            if(key.UnicodeChar == L'y'){
                                UINTN pixelCount = GOPInfo->HorizontalResolution * GOPInfo->VerticalResolution;
                                bestModePixelCount = pixelCount;
                                bestModeNum = i;
                                bestModeWidth = GOPInfo->HorizontalResolution;
                                uefi_call_wrapper(GOP->QueryMode, 4, GOP, bestModeNum, &GOPInfoSize, &GOPInfo);
                                Print(L"\r\nSelected:\r\nmode %d: %dx%d format %x%s\r\n", bestModeNum, GOPInfo->HorizontalResolution, GOPInfo->VerticalResolution, GOPInfo->PixelFormat, bestModeNum == GOPNativeMode ? L"(current)" : L"");
                                break;
                            }
                            else{
                                Print(L"Nuh uh\r\n");
                            }
                        }
                        break;
                    }
                    case 4: {
                        //Boot Nuck OS
                        //GOP info
                        uefi_call_wrapper(GOP->QueryMode, 4, GOP, bestModeNum, &GOPInfoSize, &GOPInfo);
                        //Set GOP mode
                        status = uefi_call_wrapper(GOP->SetMode, 2, GOP, bestModeNum);
                        if(EFI_ERROR(status)){
                            Print(L"Unable to set GOP mode %d\r\n", bestModeNum);
                            while(1);
                        }

                        //allocate memory for backbuffer
                        status = uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, GOP->Mode->FrameBufferSize, &fb2_addr);
                        if(EFI_ERROR(status)){
                            Print(L"Can't allocate pool of %d bytes for video backbuffer\r\n", GOP->Mode->FrameBufferSize);
                            while(1);
                        }
                        //allocate memory for kernel stack(2 MiB)
                        status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, kernel_stack_size, &kernel_stack);
                        if(EFI_ERROR(status)){
                            Print(L"Can't allocate %d pages for kernel stack\r\n", kernel_stack_size);
                            while(1);
                        }
                        //allocate memory for kernel heap map(1 page = 4096 bytes = maps to 32768 pages)
                        status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, 1, &kernel_heap_map);
                        if(EFI_ERROR(status)){
                            Print(L"Can't allocate %d pages for kernel heap map\r\n", 1);
                            while(1);
                        }
                        //allocate memory for kernel heap(32768 pages = 1342117728 bytes = 128 MiB heap)
                        status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, 32768, &kernel_heap);
                        if(EFI_ERROR(status)){
                            Print(L"Can't allocate %d pages for kernel heap\r\n", 32768);
                            while(1);
                        }

                        //get memory map
                        status = uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, &MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
                        uefi_call_wrapper(ST->BootServices->ExitBootServices, 2, ImageHandle, MapKey);
                        goto exit_boot_services;
                        break;
                    }
                    case 5: {
                       //view mem map
                        getMemoryMap(ST, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
                        printMemoryMap(ST, MemoryMapSize, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
                        break;
                    }
                    case 6: {
                        //view config tables
                        printConfigurationTables(ST);
                        break;
                    }
                    case 7: {
                        //EFI shell
                        Print(L"EFI SHELL\r\n");
                        root = openVolume(ST, ImageHandle); //opens root of filesystem of boot device

                        wchar_t* filename = L"EFI\\BOOT\\SHELLX64.EFI";
                        EFI_PHYSICAL_ADDRESS shell_addr = loadFile(ST, root, filename);
                        UINT64 size;
                        {
                            EFI_FILE_PROTOCOL* file;
                            file = openFile(root, filename);
                            size = getFileSize(ST, file, filename);
                        }

                        EFI_HANDLE shell_image = NULL;
                        status = uefi_call_wrapper(ST->BootServices->LoadImage, 6, FALSE, ImageHandle, NULL, (VOID*)shell_addr, size, &shell_image);
                        if(EFI_ERROR(status)){
                            Print(L"Failed to Load EFI Shell image\r\n");
                            while(1);
                        }
                        status = uefi_call_wrapper(ST->BootServices->StartImage, 3, shell_image, NULL, NULL);
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
                }



            }
        }
    }
    exit_boot_services:

    typedef void (*Kernel_entry)(KERNEL_CONTEXT_TABLE*);
    Kernel_entry kernel_main = (void*)kernel;

    KERNEL_HEAP heap = {
        (uint8_t*)kernel_heap_map,
        (uint8_t*)kernel_heap
    };
    KERNEL_CONTEXT_TABLE ctx = {
        ST->FirmwareVendor,
        ST->FirmwareRevision,
        ST->RuntimeServices,
        MemoryMap,
        MemoryMapSize,
        DescriptorSize,
        ST->ConfigurationTable,
        ST->NumberOfTableEntries,
        GOP->Mode,
        fb2_addr,
        kernel_stack,
        kernel_stack_size,
        &heap,
        bad_apple,
        nuckos_logo,
        pointer_icon
    };

    //switch to kernel stack and call start of kernel image
    asm volatile(
        ".intel_syntax noprefix\n"
        "mov rsp, %[stack_top]\n"
        "mov rdi, %[ctx_ptr]\n"
        "call %[kernel_entry]\n"
        ".att_syntax\n"
        :
        : [stack_top] "r"((uint64_t) kernel_stack + kernel_stack_size),
        [ctx_ptr] "r"(&ctx),
        [kernel_entry] "r"(kernel_main)
    );

    while(true);
    return EFI_SUCCESS;
}

void refreshEntries(EFI_SYSTEM_TABLE* ST, wchar_t* menuEntries[], UINTN menuEntriesCount, UINTN selectedEntryIndex, UINTN startColumn, UINTN startRow){
    //sets cursor position
    uefi_call_wrapper(ST->ConOut->SetCursorPosition, 3, ST->ConOut, startColumn, startRow);

    //sets font color
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);
    
    for(UINTN entryIndex = 0;entryIndex < menuEntriesCount;entryIndex++){ //print all menu entries
        if(entryIndex == selectedEntryIndex){
            uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR_SELECTED);
        }
        Print(menuEntries[entryIndex]);
        uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, FONT_COLOR);
        Print(L"\r\n");
    }
}

EFI_PHYSICAL_ADDRESS loadFile(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* root, wchar_t* filename){
    EFI_FILE_PROTOCOL* file;
    UINT64 size;
    EFI_PHYSICAL_ADDRESS addr;

    file = openFile(root, filename);
    size = getFileSize(ST, file, filename);
    uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, size, &addr);
    uefi_call_wrapper(file->Read, 3, file, &size, addr);

    closeFile(file);
    return addr;
}

void closeFile(EFI_FILE_PROTOCOL* file){
    EFI_STATUS status;
    status = uefi_call_wrapper(file->Close, 1, file);
    if(EFI_ERROR(status)){
        Print(L"file close failed");
        while(1);
    }
}

UINT64 getFileSize(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* file, wchar_t* filename){    
    EFI_FILE_INFO* info;
    UINT64 ret;

    info = LibFileInfo(file);
    ret = info->FileSize;
    uefi_call_wrapper(ST->BootServices->FreePool, 1, info);
    Print(filename);
    Print(L" file size: ");
    if(ret >= 1024*1024){
        Print(L"%f MiB/%f MB ", ret/(1048576.0f), ret/(1000000.0f));
    }
    else if(ret >= 1024){
        Print(L"%f KiB/%f KB ", ret/(1024.0f), ret/(1000.0f));
    }
    Print(L"%f pages/%d bytes\r\n", ret/(512.0f), ret);
    return ret;
}

EFI_FILE_PROTOCOL* openFile(EFI_FILE_PROTOCOL* volume, CHAR16* filename){
    EFI_STATUS status;
    EFI_FILE_PROTOCOL* file; //holds file

    status = uefi_call_wrapper(volume->Open, 5, volume, &file, filename, EFI_FILE_MODE_READ, 0); //no need for attributes, only for creating files
    if(EFI_ERROR(status)){
        Print(L"file open failed\r\n");
        while(1);
    }
    return file;
}

EFI_FILE_PROTOCOL* openVolume(EFI_SYSTEM_TABLE* ST, EFI_HANDLE IH){
    //image interface
    EFI_LOADED_IMAGE* loadedImage = NULL; //stores info about current uefi app + disk volume
    EFI_FILE_IO_INTERFACE* fsInterface; 
    EFI_FILE_PROTOCOL* volume;
    EFI_STATUS status;

    EFI_GUID imgGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

    //get loaded image info, puts info into loadedImage
    status= uefi_call_wrapper(ST->BootServices->HandleProtocol, 3, IH, &imgGuid, (void**)&loadedImage);
    //get volume handle, gets fs from the disk
    status= uefi_call_wrapper(ST->BootServices->HandleProtocol, 3, loadedImage->DeviceHandle, &fsGuid, (void*)&fsInterface);
    //open root of the filesystem
    status= uefi_call_wrapper(fsInterface->OpenVolume, 2, fsInterface, &volume);
    return volume;
}

void printConfigurationTables(EFI_SYSTEM_TABLE* ST){

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

    Print(L"Number of configuration table entries: %d\r\n", ST->NumberOfTableEntries);
    while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, NULL) != EFI_SUCCESS);

    for(UINTN entry = 0;entry < ST->NumberOfTableEntries;entry++){
        table = ST->ConfigurationTable[entry];
        Print(L"Table #%d:", entry+1);
        printGUID(&table.VendorGuid);
        for(UINTN guidIndex = 0;guidIndex < (sizeof(GUIDTableKeys)/sizeof(GUIDTableKeys[0]));guidIndex++){
            if(cmpGUID(&table.VendorGuid, &GUIDTableKeys[guidIndex])){
                Print(L"  ");
                Print(GUIDTableValues[guidIndex]);
                break;
            }
        }
        Print(L"  ptr: 0x%lx\r\n", table.VendorTable);
        while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, NULL) != EFI_SUCCESS);
    }
    Print(L"----------END----------\r\n");
}

void printMemoryMap(EFI_SYSTEM_TABLE* ST, UINTN MemoryMapSize, EFI_MEMORY_DESCRIPTOR* MemoryMap, UINTN MapKey, UINTN DescriptorSize, UINT32 DescriptorVersion){
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
    L"EfiMemoryMappedIO",
    L"EfiMemoryMappedIOPortSpace",
    L"EfiPalCode",
    L"EfiPersistentMemory", //yes
    L"EfiUnacceptedMemoryType",
    L"EfiMaxMemoryType"
    };

    UINTN entries = MemoryMapSize / DescriptorSize;
    EFI_MEMORY_DESCRIPTOR* MM = MemoryMap;
    
    //size of conventional memory in number of 4 KiB pages
    UINTN totalMapped = 0;
    UINTN totalUsable = 0;
    UINTN totalConventional = 0;

    //print other info
    Print(L"\r\nMemory Map Size: %lu\r\nMap Key: %lu\r\nSize of each entry: %lu\r\nVer: %u\r\nTotal entries: %lu\r\n", MemoryMapSize, MapKey, DescriptorSize, DescriptorVersion, entries);
    Print(L"----------START----------\r\n");
    for(UINTN i = 0;i < entries;i++){
        Print(L"#%lu - ", i+1);
        
        if(MM->Type < sizeof(type_arr)/sizeof(type_arr[0])){
            //add to mem size counters
            totalMapped += MM->NumberOfPages;
            if(MM->Type == EfiLoaderCode || MM->Type == EfiBootServicesCode || MM->Type == EfiBootServicesData || MM->Type == EfiConventionalMemory || MM->Type == EfiPersistentMemory){
                uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_GREEN, EFI_GREEN));
                totalUsable += MM->NumberOfPages;
            }
            else if(MM->Type == EfiLoaderData){
                uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_BLUE, EFI_BLUE));
            }
            else{
                uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_RED, EFI_RED));
            }
            if(MM->Type == EfiConventionalMemory){
                totalConventional += MM->NumberOfPages;
            }
            Print(L" ");
            //reset color
            uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
            Print(L"%s ", type_arr[MM->Type]);
        }
        else{
            uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_MAGENTA, EFI_MAGENTA));
            Print(L" ");
            //reset color
            uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
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
        MM = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MM + DescriptorSize);
        //break here
        while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) != EFI_SUCCESS);
    }
    Print(L"\r\n----------END----------\r\n");
    Print(L"Total mapped memory: %d pages/%f GB/%f GiB\r\n", totalMapped, totalMapped/250000.0f, totalMapped/262144.0f);
    Print(L"Total usable memory: %d pages/%f GB/%f GiB\r\n", totalUsable, totalUsable/250000.0f, totalUsable/262144.0f);
    Print(L"Total conventional memory: %d pages/%f GB/%f GiB\r\n", totalConventional, totalConventional/250000.0f, totalConventional/262144.0f);

}

void getMemoryMap(EFI_SYSTEM_TABLE* ST, UINTN* MemoryMapSize, EFI_MEMORY_DESCRIPTOR** MemoryMap, UINTN* MapKey, UINTN* DescriptorSize, UINT32* DescriptorVersion){
    EFI_STATUS status;
    *MemoryMapSize = 0;
    //get memory size of memory map
    status = uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, MemoryMapSize, NULL, MapKey, DescriptorSize, DescriptorVersion); //all are type*
    if(status != EFI_BUFFER_TOO_SMALL){
        Print(L"Error when getting size of memory map\r\n");
        while(1);
    }
    //allocate space by allocating a pool
    //10 extra entries is added to size if memory map changes
    status = uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, (*MemoryMapSize) + (*DescriptorSize) * 10, (void**)MemoryMap); //here MemoryMap is a void**
    if(EFI_ERROR(status)){
        Print(L"Failed to allocate pool of: (%lu + %lu) bytes for Memory Map\r\n", (*MemoryMapSize), (*DescriptorSize) * 10);
        while(1);
    }
    *MemoryMapSize += *DescriptorSize * 5;
    //get memory map
    status = uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, MemoryMapSize, *MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
    if(EFI_ERROR(status)){
        Print(L"Failed to get memory map\r\n");
        while(1);
    }
}

void printLogo(EFI_SYSTEM_TABLE* ST){
    //CHAR16* oslogo = L"                                   _   _    ___\r\n                                  | | | |  / _ \\\r\n    _   _                  _      | |_| | |  __/\r\n   | \\ | |  _   _    ___  | | __   \\__,_|  \\___|   / _ \\  / ___| \r\n   |  \\| | | | | |  / __| | |/ /      __   _      | | | | \\___ \\ \r\n   | |\\  | | |_| | | (__  |   <      / _| (_)     | |_| |  ___) |\r\n   |_| \\_|  \\__,_|  \\___| |_|\\_\\    | |_  | |      \\___/  |____/ \r\n                                    |  _| | |                    \r\n                                    |_|   |_|                    \r\n               \"operating system of the future\" (TM)\r\n";
    CHAR16* oah = L"                                   _   _    ___\r\n                                  | | | |  / _ \\\r\n    _   _                  _      | |_| | |  __/   ____              _\r\n   | \\ | |  _   _    ___  | | __   \\__,_|  \\___|  | __ )  ___   ___ | |\r\n   |  \\| | | | | |  / __| | |/ /      __   _      |  _ \\ / _ \\ / _ \\| __| \r\n   | |\\  | | |_| | | (__  |   <      / _| (_)     | |_) | (_) | (_) | |_\r\n   |_| \\_|  \\__,_|  \\___| |_|\\_\\    | |_  | |     |____/ \\___/ \\___/ \\__|\r\n                                    |  _| | |\r\n                                    |_|   |_|\r\n                   \"operating system of the future\" (TM)\r\n";
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, oah);
}

void printInfo(EFI_SYSTEM_TABLE* ST){
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
    Print(L"0");
    Print(L"\r\n");
}

void printGUID(EFI_GUID* guid){
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

uint8_t cmpGUID(EFI_GUID* guid1, EFI_GUID* guid2){
    if(guid1->Data1 != guid2->Data1)return 0;
    if(guid1->Data2 != guid2->Data2)return 0;
    if(guid1->Data3 != guid2->Data3)return 0;
    for(uint8_t c = 0;c < 8;c++){
        if(guid1->Data4[c] != guid2->Data4[c])return 0;
    }
    return 1;
}
