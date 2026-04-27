#include <efi.h>
#include <efilib.h>

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);

    Print(u"Hello World!\r\n");

    Print(u"From Group: Sudo Make Me a Sandwich!\r\n");

    while (1);

    return EFI_SUCCESS;
}
