#include "main.h"
#include <efi.h>
#include <efilib.h>


EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
    EFI_INPUT_KEY key;
    EFI_STATUS status;
    EFI_SYSTEM_TABLE* ST = SystemTable;

    InitializeLib(ImageHandle, ST);

    //sets background to BLACK
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_BLUE, EFI_BLUE));
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

    //sets font color to light green on black
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
    
    //print logo
    print_logo();
    
    //very good message
    CHAR16 buffer[] = L"Also Allen is very, very annoying\r\n";
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, buffer);

    //sets font color to red on white
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTRED, EFI_WHITE));

    //very good message
    CHAR16 buff[] = L"Press keystroke to shutdown...\r\n";
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, buff);

    while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) != EFI_SUCCESS);


    //shutdown
    uefi_call_wrapper(ST->RuntimeServices->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);


    return EFI_SUCCESS;
}

void print_logo(){
    CHAR16* oah = L"                                 _   _    ___\r\n                                | | | |  / _ \\\r\n  _   _                  _      | |_| | |  __/    ___    ____  \r\n | \\ | |  _   _    ___  | | __   \\__,_|  \\___|   / _ \\  / ___| \r\n |  \\| | | | | |  / __| | |/ /      __   _      | | | | \\___ \\ \r\n | |\\  | | |_| | | (__  |   <      / _| (_)     | |_| |  ___) |\r\n |_| \\_|  \\__,_|  \\___| |_|\\_\\    | |_  | |      \\___/  |____/ \r\n                                  |  _| | |\r\n                                  |_|   |_|\r\n             \"operating system of the future\" (TM)\r\n";
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, oah);
}


