#include "../include/nuckdef.h"
#include "../include/kernel.h"
#include <efi.h>
#include <efilib.h>


/*

typedef struct {
  UINT32                                 MaxMode;
  UINT32                                 Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION   *Info;
  UINTN                                  SizeOfInfo;
  EFI_PHYSICAL_ADDRESS                   FrameBufferBase;
  UINTN                                  FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;
 
typedef struct {
  UINT32                     Version;
  UINT32                     HorizontalResolution;
  UINT32                     VerticalResolution;
  EFI_GRAPHICS_PIXEL_FORMAT  PixelFormat;
  EFI_PIXEL_BITMASK          PixelInformation;
  UINT32                     PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;
*/

void main(KERNEL_CONTEXT_TABLE* ctx){
    putPixel(ctx->GOP, 0, 0, );



    while(true);
}
void putPixel(EFI_GOP* GOP, uint32_t x, uint32_t y){
    //check if x and y are legal
    uint32_t xMax = GOP->Info->HorizontalResolution - 1;
    uint32_t yMax = GOP->Info->VerticalResolution - 1;
    if(x < 0 || x > xMax || y < 0 || y > yMax){
        return;
    }
    char* fb = (char*) GOP->FrameBufferBase;
    uint32_t ppl = GOP->Info->PixelsPerScanLine;


}


