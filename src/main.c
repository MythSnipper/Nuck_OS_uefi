#include "main.h"
#include <efi.h>
#include <efilib.h>


EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
    EFI_SYSTEM_TABLE* ST = SystemTable;
    EFI_INPUT_KEY key;
    EFI_STATUS status;

    CHAR16 buff[2] = L"\0\0";
    InitializeLib(ImageHandle, ST); //initialize runtime pointers

    //simple keyboard loop to determine to reset or not to reset the display
    Print(L"Reset display to standard VGA(80x25)(safe graphics)? (y/n):");
    while(true){
        //keyboard input
        if(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) == EFI_SUCCESS){
            if(key.UnicodeChar == L'y'){
                uefi_call_wrapper(ST->ConOut->Reset, 2, ST->ConOut, false);
                break;
            }
            else if(key.UnicodeChar == L'n'){
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
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"F1 to shutdown\r\nF2 to reset text input\r\nF3 to get memory map\r\n");


    //variables used in main logic
    UINTN MemoryMapSize = 0; //size of the memory map in bytes
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;


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
                buff[0] = (key.ScanCode / 10) % 10 + 48;
                uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, buff);
                buff[0] = key.ScanCode % 10 + 48;
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
            }
        }
    }

    return EFI_SUCCESS;
}

void printMemoryMap(EFI_SYSTEM_TABLE* ST, UINTN MemoryMapSize, EFI_MEMORY_DESCRIPTOR* MemoryMap, UINTN MapKey, UINTN DescriptorSize, UINT32 DescriptorVersion){
    EFI_INPUT_KEY key;
    UINT16* type_arr[] = {
    L"EfiReservedMemoryType",
    L"EfiLoaderCode",
    L"EfiLoaderData",
    L"EfiBootServicesCode",
    L"EfiBootServicesData",
    L"EfiRuntimeServicesCode",
    L"EfiRuntimeServicesData",
    L"EfiConventionalMemory",
    L"EfiUnusableMemory",
    L"EfiACPIReclaimMemory",
    L"EfiACPIMemoryNVS",
    L"EfiMemoryMappedIO",
    L"EfiMemoryMappedIOPortSpace",
    L"EfiPalCode",
    L"EfiPersistentMemory",
    L"EfiUnacceptedMemoryType",
    L"EfiMaxMemoryType"
    };


    UINTN entries = MemoryMapSize / DescriptorSize;
    EFI_MEMORY_DESCRIPTOR* MM = MemoryMap;
    
    //print other info
    Print(L"\r\nMemory Map Size: %lu\r\nMap Key: %lu\r\nSize of each entry: %lu\r\nVer: %u\r\nTotal entries: %lu\r\n", MemoryMapSize, MapKey, DescriptorSize, DescriptorVersion, entries);

    for(UINTN i = 0;i < entries;i++){
        Print(L"\r\nEntry %lu:\r\n", i+1);
        if(MM->Type >= 0 && MM->Type < 17){ //17 elements
            Print(L"Type: %s\r\n", type_arr[MM->Type]);
        }
        else{
            Print(L"Type: %u\r\n", MM->Type);
        }
        Print(L"Physical Start: 0x%lx\r\n", MM->PhysicalStart);
        Print(L"Virtual Start: 0x%lx\r\n", MM->VirtualStart);
        Print(L"Number of 4KiB pages: %lu\r\n", MM->NumberOfPages);
        Print(L"Attribute: 0x%lx\r\n", MM->Attribute);
        //go to next one
        MM = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MM + DescriptorSize);
        //break here
        while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) != EFI_SUCCESS);
    }
    Print(L"End-------------------------------------------------------");
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
    *MemoryMapSize += *DescriptorSize * 10;
    //get memory map
    status = uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, MemoryMapSize, *MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
    if(EFI_ERROR(status)){
        Print(L"Failed to get memory map\r\n");
        while(true);
    }
}


void printLogo(EFI_SYSTEM_TABLE* ST){
    CHAR16* oah = L"                                 _   _    ___\r\n                                | | | |  / _ \\\r\n  _   _                  _      | |_| | |  __/    ___    ____  \r\n | \\ | |  _   _    ___  | | __   \\__,_|  \\___|   / _ \\  / ___| \r\n |  \\| | | | | |  / __| | |/ /      __   _      | | | | \\___ \\ \r\n | |\\  | | |_| | | (__  |   <      / _| (_)     | |_| |  ___) |\r\n |_| \\_|  \\__,_|  \\___| |_|\\_\\    | |_  | |      \\___/  |____/ \r\n                                  |  _| | |\r\n                                  |_|   |_|\r\n             \"operating system of the future\" (TM)\r\n";
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, oah);
}

void printInfo(EFI_SYSTEM_TABLE* ST){
    Print(L"EFI ST conforms to UEFI revision: %d.%d\r\n", (ST->Hdr.Revision >> 16) && 0xFFFF, ST->Hdr.Revision & 0xFFFF);

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











