#include "../include/nuckdef.h"
#include "../include/kernel.h"
#include <efi.h>
#include <efilib.h>


/*

typedef struct{
    CHAR16*                                 FirmwareVendor;
    UINT32                                  FirmwareRevision;
    EFI_RUNTIME_SERVICES*                   RuntimeServices;
    EFI_MEMORY_DESCRIPTOR*                  MemoryMap;
    UINTN                                   MemoryMapSize;
    UINTN                                   MemoryMapDescriptorSize;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE*      GOP;
} KERNEL_CONTEXT_TABLE;


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
    
    extern uint8_t VGAfont[];



    while(ctx->GOP->Info->PixelFormat != 1);
    
    GOPDrawRect(ctx->GOP, 0, 0, ctx->GOP->Info->HorizontalResolution-1, ctx->GOP->Info->VerticalResolution-1, rgba(60, 60, 60, 0), true);
    
    GOPDrawRect(ctx->GOP, 0, 0, 99, 99, rgba(255, 0, 0, 0), true); //red
    GOPDrawRect(ctx->GOP, 100, 0, 199, 199, rgba(0, 255, 0, 0), true);
    GOPDrawRect(ctx->GOP, 200, 0, 299, 299, rgba(0, 0, 255, 0), true);
    GOPDrawRect(ctx->GOP, 300, 0, 399, 399, rgba(255, 255, 0, 0), true);
    GOPDrawRect(ctx->GOP, 400, 0, 499, 499, rgba(255, 0, 255, 0), true);
    GOPDrawRect(ctx->GOP, 500, 0, 599, 599, rgba(0, 255, 255, 0), true);
    

    KERNEL_TEXT_OUTPUT ConOut = {VGAfont, 8, 16, 0, 0, rgba(0, 255, 0, 0), rgba(0, 0, 255, 0)};
    //for(uint8_t i=32;i<45;i++){
    //    drawChar(ctx->GOP, &ConOut, i);
    //}
    //drawChar(ctx->GOP, &ConOut, '#');

    while(true);
}

void drawChar(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t ascii_char){
    uint32_t maxWidth = GOP->Info->HorizontalResolution / ConOut->charWidth;
    uint32_t maxHeight = GOP->Info->VerticalResolution / ConOut->charHeight;
    if(ascii_char == 0){ //null character
        return;
    }
    if(ascii_char < 32 && ascii_char > 126){ //non printable
        ascii_char = ' '; //space
    }

    //print ascii character at cursorX, cursorY
    uint32_t screenX = ConOut->cursorX * ConOut->charWidth; //screen position to print character to
    uint32_t screenY = ConOut->cursorY * ConOut->charHeight;

    uint32_t startingIndex = (ascii_char-32)*ConOut->charHeight; //starting index of the font array for the character
    for(uint32_t dy = 0;dy < ConOut->charHeight;dy++){
        uint8_t row = ConOut->font[startingIndex + dy];
        for(uint32_t dx = 0;dx < ConOut->charWidth;dx++){
            uint32_t color = (row & (1 << (7 - dx))) ? ConOut->frontColor : ConOut->backColor;
            GOPPutPixel(GOP, screenX+dx, screenY+dy, color);
        }
    }
    //advance cursor position
    ConOut->cursorX++;
    if(ConOut->cursorX >= maxWidth){
        ConOut->cursorX = 0;
        ConOut->cursorY++;
    }
    if(ConOut->cursorY >= maxHeight){
        ConOut->cursorY = 0; //TODO: replace this line with scroll
    }

}

void sortMemoryMap(EFI_MEMORY_DESCRIPTOR** MM, uint64_t MM_size, uint64_t MM_desc_size){

}

void GOPDrawRect(EFI_GOP* GOP, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color, uint8_t fill){
    //convert x, y to memory address
    uint32_t xmax = ((x1 > x2) ? x1 : x2);
    uint32_t xmin = ((x1 > x2) ? x2 : x1);
    uint32_t ymax = ((y1 > y2) ? y1 : y2);
    uint32_t ymin = ((y1 > y2) ? y2 : y1);

    if(!fill){
        for(uint32_t x = xmin;x <= xmax;x++){
            GOPPutPixel(GOP, x, ymin, color);
            GOPPutPixel(GOP, x, ymax, color);
        }
        for(uint32_t y = ymin+1;y < ymax;y++){
            GOPPutPixel(GOP, xmin, y, color);
            GOPPutPixel(GOP, xmax, y, color);
        }
    }
    else{
        for(uint32_t y = ymin;y <= ymax;y++){
            for(uint32_t x = xmin;x <= xmax;x++){
                GOPPutPixel(GOP, x, y, color);
            }
        }
    }
}

void GOPPutPixel(EFI_GOP* GOP, uint32_t x, uint32_t y, uint32_t color){
    //check if x and y are legal
    uint32_t xMax = GOP->Info->HorizontalResolution;
    uint32_t yMax = GOP->Info->VerticalResolution;
    if(x >= xMax || y >= yMax){
        return;
    }
    uint8_t* fb = (uint8_t*) GOP->FrameBufferBase;
    uint32_t ppl = (uint32_t) GOP->Info->PixelsPerScanLine;
    uint8_t* fb_addr = (uint8_t*)(fb + 4 * y * ppl + 4 * x);
    *((uint32_t*)fb_addr) = color;

}

static inline uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    return ((uint32_t)b) | ((uint32_t)g << 8) | ((uint32_t)r << 16) | ((uint32_t)a << 24);
}
