#include "main.h"
#include <efi.h>
#include <efilib.h>


EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
    EFI_SYSTEM_TABLE* ST = SystemTable;
    EFI_INPUT_KEY key;
    EFI_STATUS status;

    InitializeLib(ImageHandle, ST); //initialize runtime pointers

    //Reset console output
    uefi_call_wrapper(ST->ConOut->Reset, 2, ST->ConOut, false);

    //clear console output, sets background color, cursor goes to 0, 0
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_CYAN, EFI_CYAN));
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

    //sets font color to light green on black
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
    Print(u"hello world\r\n");

    print_logo();

    Print(L"EFI ST conforms to UEFI revision: %d.%d\r\n", (ST->Hdr.Revision >> 16) && 0xFFFF, ST->Hdr.Revision & 0xFFFF);

    Print(L"Firmware Vendor: %s\r\n", ST->FirmwareVendor);

    Print(L"System UEFI firmware revision: %d.%d\r\n", (ST->FirmwareRevision >> 16) && 0xFFFF, ST->FirmwareRevision & 0xFFFF);

    //sets font color to red on white
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_RED, EFI_BLUE));

    //very good message
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"Do a keystroke to shutdown...\r\n");

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
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_LIGHTGRAY));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"Z");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTBLUE, EFI_DARKGRAY));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"E");
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"!");


    while(uefi_call_wrapper(ST->ConIn->ReadKeyStroke,  2, ST->ConIn, &key) != EFI_SUCCESS);

    //shutdown
    uefi_call_wrapper(ST->RuntimeServices->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);


    return EFI_SUCCESS;
}

void print_logo(){
    CHAR16* oah = L"                                 _   _    ___\r\n                                | | | |  / _ \\\r\n  _   _                  _      | |_| | |  __/    ___    ____  \r\n | \\ | |  _   _    ___  | | __   \\__,_|  \\___|   / _ \\  / ___| \r\n |  \\| | | | | |  / __| | |/ /      __   _      | | | | \\___ \\ \r\n | |\\  | | |_| | | (__  |   <      / _| (_)     | |_| |  ___) |\r\n |_| \\_|  \\__,_|  \\___| |_|\\_\\    | |_  | |      \\___/  |____/ \r\n                                  |  _| | |\r\n                                  |_|   |_|\r\n             \"operating system of the future\" (TM)\r\n";
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, oah);
}


