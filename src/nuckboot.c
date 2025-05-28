#include "../include/nuckdef.h"
#include "../include/nuckboot.h"
#include <efi.h>
#include <efilib.h>

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
    EFI_SYSTEM_TABLE* ST = SystemTable;
    EFI_INPUT_KEY key;
    EFI_STATUS status;

    CHAR16 buff[2] = u"\0\0";

    InitializeLib(ImageHandle, ST); //initialize runtime pointers

    //disable watchdog timer
    uefi_call_wrapper(ST->BootServices->SetWatchdogTimer, 0, 0, 0x10000, 0, NULL);

    //simple keyboard loop to determine to reset or not to reset the display
    Print(L"Reset display to standard VGA(80x25)(safe graphics)? (y/n/r/s):");
    while(true){
        //keyboard input
        if(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) == EFI_SUCCESS){
            if(key.UnicodeChar == L'y'){
                status = uefi_call_wrapper(ST->ConOut->SetMode, 2, ST->ConOut, 2);
                if(EFI_ERROR(status)){
                    status = uefi_call_wrapper(ST->ConOut->SetMode, 2, ST->ConOut, 1);
                }
                if(EFI_ERROR(status)){
                    uefi_call_wrapper(ST->ConOut->SetMode, 2, ST->ConOut, 0);
                }
                break;
            }
            else if(key.UnicodeChar == L'n'){
                break;
            }
            else if(key.UnicodeChar == L'r'){
                uefi_call_wrapper(ST->ConOut->Reset, 2, ST->ConOut, false);
                break;
            }
            else if(key.UnicodeChar == L's'){
                uefi_call_wrapper(ST->ConOut->SetMode, 2, ST->ConOut, 0);
                break;
            }
        }
    }

    //clear console output, sets background color, cursor goes to 0, 0
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_CYAN, EFI_CYAN));
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

    //sets font color to light green on black
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
    Print(L"hello world\r\n");

    printLogo(ST);

    printInfo(ST);

    //sets font color to red on white
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));

    //very good message
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"F1 to shutdown\r\nF2 to reset text input\r\nF3 to view memory map\r\nF4 to load Nuck OS kernel\r\nF5 to select GOP mode\r\nF6 to set GOP\r\nF7 to get memory map and run Nuck OS kernel\r\n");

    //variables used in main logic
    UINTN MemoryMapSize = 0; //size of the memory map in bytes
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    //used to store the volume on the disk
    EFI_FILE_PROTOCOL* root;
    EFI_FILE_PROTOCOL* kernel_file;
    UINT64 kernel_size;
    EFI_PHYSICAL_ADDRESS kernel_addr;

    //GOP variables
    EFI_GRAPHICS_OUTPUT_PROTOCOL* GOP;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* GOPInfo;
    UINTN GOPInfoSize;
    UINTN GOPNumModes;
    UINTN GOPNativeMode;

    UINTN bestModeNum = 0;
    UINTN bestModePixelCount = 0;
    UINTN bestModeWidth = 0;
    UINTN bestModeHeight = 0;
    

    while(true){
        //keyboard input
        if(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) == EFI_SUCCESS){
            //Basic text output
            if(key.UnicodeChar == L'\r'){
                uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"\r\n"); 
            }
            else{
                buff[0] = key.UnicodeChar;
                uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, buff);
            }
            //Function keys
            switch(key.ScanCode){
                case 0x0B:
                    uefi_call_wrapper(ST->RuntimeServices->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);
                    Print(L"Success!\r\n");
                    break;
                case 0x0C:
                    uefi_call_wrapper(ST->ConIn->Reset, 2, ST->ConIn, false);
                    Print(L"Success!\r\n");
                    break;
                case 0x0D:
                    getMemoryMap(ST, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
                    printMemoryMap(ST, MemoryMapSize, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
                    break;
                case 0x0E:
                    Print(L"loading kernel\r\n");
                    root = openVolume(ST, ImageHandle); //opens root of filesystem of boot device
                    //open file
                    kernel_file = openFile(root, L"kernel.bin"); 
                    //get file size
                    kernel_size = getFileSize(ST, kernel_file);

                    //load kernel binary
                    status = uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, kernel_size, &kernel_addr);
                    //read file to kernel address
                    uefi_call_wrapper(kernel_file->Read, 3, kernel_file, &kernel_size, kernel_addr);  
                    
                    Print(L"kernel load success\r\n");
                    //close file
                    closeFile(kernel_file);
                    break;
                case 0x0F:
                    //set GOP
                    EFI_GUID GOPGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
                    status = uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &GOPGuid, NULL, (void**) &GOP);
                    if(EFI_ERROR(status)){
                        Print(L"No GOP\r\n");
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
                            bestModeHeight = GOPInfo->VerticalResolution;
                        }
                        else if(pixelCount == bestModePixelCount){
                            if(bestModeWidth > GOPInfo->HorizontalResolution){
                                bestModeNum = i;
                                bestModeWidth = GOPInfo->HorizontalResolution;
                                bestModeHeight = GOPInfo->VerticalResolution;
                            }
                        }
                    }
                    uefi_call_wrapper(GOP->QueryMode, 4, GOP, bestModeNum, &GOPInfoSize, &GOPInfo);
                    Print(L"Selected:\r\nmode %d: %dx%d format %x%s\r\n", bestModeNum, GOPInfo->HorizontalResolution, GOPInfo->VerticalResolution, GOPInfo->PixelFormat, bestModeNum == GOPNativeMode ? L"(current)" : L"");
                    break;
                case 0x10:
                    //GOP info
                    uefi_call_wrapper(GOP->QueryMode, 4, GOP, bestModeNum, &GOPInfoSize, &GOPInfo);
                    //Set GOP mode
                    status = uefi_call_wrapper(GOP->SetMode, 2, GOP, bestModeNum);
                    if(EFI_ERROR(status)){
                        Print(L"Unable to set GOP mode %d\r\n", bestModeNum);
                    }
                    break;
                case 0x11:
                    //get memory map
                    status = uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, &MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
                    uefi_call_wrapper(ST->BootServices->ExitBootServices, 2, ImageHandle, MapKey);
                    goto exit_boot_services;
                    break;
                case 0x12:
                    EFI_PHYSICAL_ADDRESS a;
                    status = uefi_call_wrapper(ST->BootServices->AllocatePool, 8, 4096, &a);
                    break;
            }
        }
    }
    exit_boot_services:

    typedef void (*Kernel_entry)(KERNEL_CONTEXT_TABLE*);
    Kernel_entry kernel_main = (Kernel_entry) kernel_addr;

    KERNEL_CONTEXT_TABLE ctx = {
        ST->FirmwareVendor,
        ST->FirmwareRevision,
        ST->RuntimeServices,
        MemoryMap,
        MemoryMapSize,
        DescriptorSize,
        GOP->Mode
    };
    kernel_main(&ctx);

    while(true);
    return EFI_SUCCESS;
}

void closeFile(EFI_FILE_PROTOCOL* file){
    EFI_STATUS status;
    status = uefi_call_wrapper(file->Close, 1, file);
    if(EFI_ERROR(status)){
        Print(L"file close failed");
    }
}

UINT64 getFileSize(EFI_SYSTEM_TABLE* ST, EFI_FILE_PROTOCOL* file){    
    EFI_FILE_INFO* info;
    UINT64 ret;

    info = LibFileInfo(file);
    ret = info->FileSize;
    uefi_call_wrapper(ST->BootServices->FreePool, 1, info);
    Print(L"kernel size: %d\r\n", ret);
    return ret;
}

EFI_FILE_PROTOCOL* openFile(EFI_FILE_PROTOCOL* volume, CHAR16* filename){
    EFI_STATUS status;
    EFI_FILE_PROTOCOL* file; //holds file

    status = uefi_call_wrapper(volume->Open, 5, volume, &file, filename, EFI_FILE_MODE_READ, 0); //no need for attributes, only for creating files
    if(EFI_ERROR(status)){
        Print(L"file open failed\r\n");
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
    status = uefi_call_wrapper(ST->BootServices->HandleProtocol, 3, IH, &imgGuid, (void**)&loadedImage);
    //get volume handle, gets fs from the disk
    status = uefi_call_wrapper(ST->BootServices->HandleProtocol, 3, loadedImage->DeviceHandle, &fsGuid, (void*)&fsInterface);
    //open root of the filesystem
    status = uefi_call_wrapper(fsInterface->OpenVolume, 2, fsInterface, &volume);
    return volume;
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
        while(true);
    }
    //allocate space by allocating a pool
    //10 extra entries is added to size if memory map changes
    status = uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, (*MemoryMapSize) + (*DescriptorSize) * 10, (void**)MemoryMap); //here MemoryMap is a void**
    if(EFI_ERROR(status)){
        Print(L"Failed to allocate pool of: (%lu + %lu) bytes for Memory Map\r\n", (*MemoryMapSize), (*DescriptorSize) * 10);
        while(true);
    }
    *MemoryMapSize += *DescriptorSize * 5;
    //get memory map
    status = uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, MemoryMapSize, *MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
    if(EFI_ERROR(status)){
        Print(L"Failed to get memory map\r\n");
        while(true);
    }
}

void printLogo(EFI_SYSTEM_TABLE* ST){
    CHAR16* oslogo = L"                                   _   _    ___\r\n                                  | | | |  / _ \\\r\n    _   _                  _      | |_| | |  __/\r\n   | \\ | |  _   _    ___  | | __   \\__,_|  \\___|   / _ \\  / ___| \r\n   |  \\| | | | | |  / __| | |/ /      __   _      | | | | \\___ \\ \r\n   | |\\  | | |_| | | (__  |   <      / _| (_)     | |_| |  ___) |\r\n   |_| \\_|  \\__,_|  \\___| |_|\\_\\    | |_  | |      \\___/  |____/ \r\n                                    |  _| | |                    \r\n                                    |_|   |_|                    \r\n               \"operating system of the future\" (TM)\r\n";
    CHAR16* oah = L"                                   _   _    ___\r\n                                  | | | |  / _ \\\r\n    _   _                  _      | |_| | |  __/   ____              _\r\n   | \\ | |  _   _    ___  | | __   \\__,_|  \\___|  | __ )  ___   ___ | |\r\n   |  \\| | | | | |  / __| | |/ /      __   _      |  _ \\ / _ \\ / _ \\| __| \r\n   | |\\  | | |_| | | (__  |   <      / _| (_)     | |_) | (_) | (_) | |_\r\n   |_| \\_|  \\__,_|  \\___| |_|\\_\\    | |_  | |     |____/ \\___/ \\___/ \\__|\r\n                                    |  _| | |\r\n                                    |_|   |_|\r\n               \"operating system of the future\" (TM)\r\n";
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, oah);
}

void printInfo(EFI_SYSTEM_TABLE* ST){
    Print(L"Firmware Vendor: %s\r\n", ST->FirmwareVendor);
    Print(L"System UEFI firmware revision: %d.%d\r\n", (ST->FirmwareRevision >> 16) && 0xFFFF, ST->FirmwareRevision & 0xFFFF);

    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_CYAN, EFI_CYAN));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"\r\n                 ");

    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_RED));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"M");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTMAGENTA, EFI_YELLOW));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"E");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTRED, EFI_GREEN));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"S");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTRED, EFI_CYAN));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"M");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLUE));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"E");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_MAGENTA));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"R");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BROWN));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"I");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTBLUE, EFI_LIGHTGRAY));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"Z");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_DARKGRAY));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"E");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"!\r\n");
}
