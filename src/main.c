#include "main.h"
#include <efi.h>
#include <efilib.h>


EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
    InitializeLib(ImageHandle, SystemTable);




    uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLUE));

    uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);

    uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));

    print_logo();
    

    CHAR16 buffer[] = L"Also Allen is gay\r\n";
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, buffer);


    uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, EFI_TEXT_ATTR(EFI_RED, EFI_WHITE));

    CHAR16 buff[] = L"Press any key to shutdown...\r\n";
    uefi_call_wrapper(
        ST->ConOut->OutputString, 
        2, 
        ST->ConOut, 
        buff
    );

    EFI_INPUT_KEY key;
    while(uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke,  2, SystemTable->ConIn, &key) != EFI_SUCCESS);


    //shutdown
    uefi_call_wrapper(SystemTable->RuntimeServices->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);


    return EFI_SUCCESS;
}

void print_logo(){
    Print(L"                                 _   _    ___\r\n");
    Print(L"                                | | | |  / _ \\\r\n");
    Print(L"  _   _                  _      | |_| | |  __/    ___    ____  \r\n");
    Print(L" | \\ | |  _   _    ___  | | __   \\__,_|  \\___|   / _ \\  / ___| \r\n");
    Print(L" |  \\| | | | | |  / __| | |/ /      __   _      | | | | \\___ \\ \r\n");
    Print(L" | |\\  | | |_| | | (__  |   <      / _| (_)     | |_| |  ___) |\r\n");
    Print(L" |_| \\_|  \\__,_|  \\___| |_|\\_\\    | |_  | |      \\___/  |____/ \r\n");
    Print(L"                                  |  _| | |\r\n");
    Print(L"                                  |_|   |_|\r\n");
    Print(L"             \"operating system of the future\" (TM)\r\n");

}


