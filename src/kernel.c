#include "../include/kernel.h"

//global vars
KERNEL_CONTEXT_TABLE* global_ctx;



void kernel_main(KERNEL_CONTEXT_TABLE* ctx){

    //disable PIC
    PIC_disable();
    
    //make ctx global
    global_ctx = ctx;

    //fill in some ctx values
    ctx->pitch = (uint32_t) ctx->GOP->Info->PixelsPerScanLine;
    ctx->width = (uint32_t) ctx->GOP->Info->HorizontalResolution;
    ctx->height = (uint32_t) ctx->GOP->Info->VerticalResolution;

    uint8_t CODE_SEG;
    uint8_t DATA_SEG;

    GDT_initialize(&CODE_SEG, &DATA_SEG);

    IDT_initialize(CODE_SEG, 0);

    uint8_t versionMajor = 1;
    uint8_t versionMinor = 6;
  
    uint8_t CPUVendor[13];
    cpuid_get_vendor(CPUVendor);

    KERNEL_TEXT_OUTPUT title = {Terminus8x16_Bold, 8, 16, 2, 2, 0, 0, 20, 20, hex(0xFF10F0), hex(0x000000), true};
    KERNEL_TEXT_OUTPUT ConOut = {Terminus8x16_Normal, 8, 16, 1, 1, 0, 8, 0, 0, hex(0xFF10F0), hex(0x000000), false};

    //NVIDEO RESOURCES
    KERNEL_NVIDEO bad_apple;
    KERNEL_NVIDEO nuckos_logo;
    KERNEL_NVIDEO pointer_icon;

    //parse media headers
    NVIDEOParseHeader(&bad_apple, (uint8_t*) ctx->kernel_resource_addrs[0]);
    NVIDEOParseHeader(&nuckos_logo, (uint8_t*) ctx->kernel_resource_addrs[1]);
    NVIDEOParseHeader(&pointer_icon, (uint8_t*) ctx->kernel_resource_addrs[2]);

    //MEM ALLOC TESTS
    /*
    heap_init(ctx->heap);   
    void* testPtr = heap_alloc(ctx->heap, 2);
    void* testPtr2 = heap_alloc(ctx->heap, 1);
    void* testPtr3 = heap_alloc(ctx->heap, 3);
    
    KERNEL_SUBPAGE_ALLOCATOR alloc = {ctx->heap, NULL, NULL};
    subpage_alloc_init(&alloc);
    subpage_alloc_expand(&alloc);
    void* subPtr = subpage_alloc(&alloc);
    void* subPtr2 = subpage_alloc(&alloc);
    heap_free(ctx->heap, testPtr, 1);

    */

    //setup input devices
    uint8_t scancode;

    uint8_t mouseInitError = PS2_mouse_init();
    uint8_t mouseSetSampleError = PS2_mouse_set_sample_rate(100);
    int8_t dx;
    int8_t dy;
    uint8_t lrm;

    //mouse pointer position
    int32_t pointerX = 0;
    int32_t pointerY = 0;

    //print_memory_map(&ConOut);


    //static UI

    //clear screen
    GOPDrawRect(0, 0, ctx->width, ctx->height, hex(0x34e5eb), true);

    while(true){
        //display
        title = (KERNEL_TEXT_OUTPUT){Terminus8x16_Bold, 8, 16, 2, 2, 0, 0, 20, 20, hex(0xFF10F0), hex(0x000000), true};
        ConOut = (KERNEL_TEXT_OUTPUT){Terminus8x16_Normal, 8, 16, 1, 1, 0, 8, 0, 0, hex(0xFF10F0), hex(0x000000), false};
        
        /*
        bool fill = true;
        uint32_t screenX = ctx->GOP->Info->HorizontalResolution - 1;
        uint32_t screenYFraction = ctx->GOP->Info->VerticalResolution / 5;
        GOPDrawRect(0, 0, screenX, screenYFraction - 1, hex(0x55CDFC), fill);
        GOPDrawRect(0, screenYFraction, screenX, 2*screenYFraction - 1, hex(0xF7A8B8), fill);
        GOPDrawRect(0, 2*screenYFraction, screenX, 3*screenYFraction - 1, hex(0xFFFFFF), fill);
        GOPDrawRect(0, 3*screenYFraction, screenX, 4*screenYFraction - 1, hex(0xF7A8B8), fill);
        GOPDrawRect(4*screenYFraction, screenX, 5*screenYFraction - 1, hex(0x55CDFC), fill);
        */
        printf(&ConOut, "Display pixel format: %d\r\n", ctx->GOP->Info->PixelFormat);
        printf(&ConOut, "CPU Vendor: %s\r\n", &CPUVendor);
        printf(&ConOut, "Video resolution: %dx%d / format %d \r\n/ frame %d/%d\r\n", bad_apple.width, bad_apple.height, bad_apple.format, bad_apple.frameCounter+1, bad_apple.frameCount);

        printf(&title, "N");
        title.frontColor = 0xFF8D00;title.backColor = 0x000000;
        printf(&title, "u");
        title.frontColor = 0xFFEE00;title.backColor = 0x000000;
        printf(&title, "c");
        title.frontColor = 0x028121;title.backColor = 0x000000;
        printf(&title, "k");
        title.frontColor = 0xFF10F0;title.backColor = 0x000000;
        printf(&title, " ");
        title.frontColor = 0x004CFF;title.backColor = 0x000000;
        printf(&title, "O");
        title.frontColor = 0x770088;title.backColor = 0x000000;
        printf(&title, "S");
        title.frontColor = 0xFF10F0;title.backColor = 0x000000;

        printf(&title, "\r\n Version %u.%u!\r\n", versionMajor, versionMinor);

        /*
        heap_display(ctx->heap, ctx->GOP, &HeapOut);
        printf(&ConOut, "\r\nheap page allocator: \r\n%lx, +32768 pages, %ld MB\r\n", ctx->heap->heap, 32768*4*1024/1024/1024);
        printf(&ConOut, "\r\n\nsubpage allocator: \r\n%lx to %lx\r\n", alloc.freeListStart, alloc.freeListEnd);
        printf(&ConOut, "1st subpage: %lx\r\n", subPtr);
        printf(&ConOut, "2nd subpage: %lx\r\n", subPtr2);
        */

        viewConfigTables(&ConOut, ctx->ConfigTable, ctx->ConfigTableEntriesCount);

        printf(&ConOut, "size of stuff: %u + %u\r\n", sizeof(*ctx), ctx->MemoryMapSizeBytes);

        //logo
        //GOPDrawImage(ctx->GOP->Info->HorizontalResolution - nuckos_logo.width - 10, ctx->GOP->Info->VerticalResolution - nuckos_logo.height - 10, &nuckos_logo);

        //PS/2 input
        /*
        uint8_t is_mouse = PS2_poll(&scancode, &dx, &dy, &lrm);
        
        if(!is_mouse){ //keyboard
            if(scancode & 0x80){
                printf(&ConOut, "BREAK\r\n");
            }
            else{
                printf(&ConOut, "MAKE\r\n");
            }
            printf(&ConOut, ":%x\r\n", scancode);
        }
        else{ //mouse
            if(mouseInitError){
                printf(&ConOut, "mouse init error\r\n");
            }
            else if(mouseSetSampleError){
                printf(&ConOut, "mouse set sample rate error\r\n");
            }
            else{
                printf(&ConOut, "mouse input: %d, %d, %x\r\n", dx, dy, lrm);
                pointerX += dx;
                pointerY += dy;
                if(lrm & 1){
                    pointerX = 0;
                    pointerY = 0;
                }
                //limits
                if(pointerX < 0)pointerX = 0;
                if(pointerX > (int32_t)(ctx->GOP->Info->HorizontalResolution-1))pointerX = ctx->GOP->Info->HorizontalResolution-1;
                if(pointerY < 0)pointerY = 0;
                if(pointerY > (int32_t)(ctx->GOP->Info->VerticalResolution-1))pointerY = ctx->GOP->Info->VerticalResolution-1;
            }
        }
        */

        //pointer icon
        //GOPDrawImage(pointerX, pointerY, &pointer_icon);

        GOPPlayVideo(ctx->GOP->Info->HorizontalResolution - bad_apple.width, 0, &bad_apple, true);

        //copy framebuffer
        update_framebuffer();

        //do this to test interrupts i guess
        //break;
    }

    GOPDrawRect(0, 0, ctx->GOP->Info->HorizontalResolution-1, ctx->GOP->Info->VerticalResolution-1, hex(0x20207F), true);
    printf(&title, "interrupting...\r\n");
    update_framebuffer();
    
    asm volatile(
        ".intel_syntax noprefix\n"
        "int 1\n"
        "int 6\n"
        "int 7\n"
        "int 67\n"
        "int 18\n"
        "int 27\n"
        "int 95\n"
        "int 21\n"
        "int 29\n"
        "int 1\n"
        ".att_syntax\n"
    );
    
    //printf(ctx->GOP, &title, "Done!\r\n");
    update_framebuffer();
    while(true);
}

void update_framebuffer(){
    bool debug = false;
    bool debug2 = true;

    if(debug){
        uint8_t* dst = (uint8_t*)global_ctx->GOP->FrameBufferBase;
        uint8_t* src = (uint8_t*)global_ctx->fb;

        for(uint32_t ty=0;ty < global_ctx->dirty_tiles_y;ty++){
            for(uint64_t tx=0;tx < global_ctx->dirty_tiles_x;tx++){
                uint64_t i = ty * global_ctx->dirty_tiles_x + tx;
                if(!global_ctx->dirty_tilemap[i]){
                    continue;
                }
                uint64_t x = tx * global_ctx->dirty_tile_size;
                uint64_t y = ty * global_ctx->dirty_tile_size;
                uint64_t w = global_ctx->dirty_tile_size;
                uint64_t h = global_ctx->dirty_tile_size;
                //restrict
                if(x+w > global_ctx->width){
                    w = global_ctx->width - x;
                }
                if(y+h > global_ctx->height){
                    h = global_ctx->height - y;
                }

                // Copy tile row by row
                for(uint64_t row=0;row < h;row++){
                    uint8_t* dst_row = dst + ((y + row) * global_ctx->pitch * 4) + (x * 4);
                    kmemset32((uint32_t*)dst_row, hex(0xff0000), w);
                }
            
            }
        }
        volatile uint64_t hang = 0;
        for(uint64_t i=0;i<30000000;i++){
            hang+=i;
        }
    }

    uint8_t* dst = (uint8_t*)global_ctx->GOP->FrameBufferBase;
    uint8_t* src = (uint8_t*)global_ctx->fb;

    for(uint32_t ty=0;ty < global_ctx->dirty_tiles_y;ty++){
        for(uint64_t tx=0;tx < global_ctx->dirty_tiles_x;tx++){
            uint64_t i = ty * global_ctx->dirty_tiles_x + tx;
            if(!global_ctx->dirty_tilemap[i]){
                continue;
            }
            uint64_t x = tx * global_ctx->dirty_tile_size;
            uint64_t y = ty * global_ctx->dirty_tile_size;
            uint64_t w = global_ctx->dirty_tile_size;
            uint64_t h = global_ctx->dirty_tile_size;
            //restrict
            if(x+w > global_ctx->width){
                w = global_ctx->width - x;
            }
            if(y+h > global_ctx->height){
                h = global_ctx->height - y;
            }
            if(debug2){
                // Copy tile row by row
                for(uint64_t row=0;row < h;row++){
                    uint8_t* dst_row = dst + ((y + row) * global_ctx->pitch * 4) + (x * 4);
                    kmemset32((uint32_t*)dst_row, hex(0xff0000), w);
                }

                volatile uint64_t hang = 0;
                for(uint64_t i=0;i<4000000;i++){
                    hang+=i;
                }
            }

            // Copy tile row by row
            for(uint64_t row=0;row < h;row++){
                uint8_t* dst_row = dst + ((y + row) * global_ctx->pitch * 4) + (x * 4);
                uint8_t* src_row = src + ((y + row) * global_ctx->pitch * 4) + (x * 4);

                kmemcpy(dst_row, src_row, w*4);
            }

            // Clear dirty flag
            global_ctx->dirty_tilemap[i] = 0;

        }
    }

    if(debug){
        volatile uint64_t hang = 0;
        for(uint64_t i=0;i<30000000;i++){
            hang+=i;
        }
    }

}

//NEW physical memory manager related functions
void print_memory_map(KERNEL_TEXT_OUTPUT* Con){
    uint64_t memory_map_size = global_ctx->MemoryMapSizeBytes;
    uint64_t memory_map_size_pages = global_ctx->MemoryMapSizePages;
    uint64_t memory_map_descriptor_size = global_ctx->MemoryMapDescriptorSize;

    char* type_arr[] = {
    "EfiReservedMemoryType",
    "EfiLoaderCode", //yes
    "EfiLoaderData", //for safety best to avoid - might contain memory map
    "EfiBootServicesCode", //yes
    "EfiBootServicesData", //yes
    "EfiRuntimeServicesCode",
    "EfiRuntimeServicesData",
    "EfiConventionalMemory", //yes
    "EfiUnusableMemory",
    "EfiACPIReclaimMemory",
    "EfiACPIMemoryNVS",
    "Efimemory_mappedIO",
    "Efimemory_mappedIOPortSpace",
    "EfiPalCode",
    "EfiPersistentMemory", //yes
    "EfiUnacceptedMemoryType",
    "EfiMaxMemoryType"
    };

    uint32_t entries = memory_map_size / memory_map_descriptor_size;
    EFI_MEMORY_DESCRIPTOR* MM = global_ctx->MemoryMap;
    
    //size of conventional memory in number of 4 KiB pages
    uint64_t totalMapped = 0;
    uint64_t totalUsable = 0;

    uint32_t oldColor = Con->frontColor;
    uint32_t oldBackColor = Con->backColor;

    //print other info
    printf(Con, "Memory Map Size: %lu\r\nSize of each entry: %lu\r\nTotal entries: %lu\r\n", memory_map_size, memory_map_descriptor_size, entries);
    for(uint32_t i = 0;i < entries;i++){
        printf(Con, "#%u - ", i+1);
        
        if(MM->Type < sizeof(type_arr)/sizeof(type_arr[0])){
            //add to mem size counters
            totalMapped += MM->NumberOfPages;
            if(MM->Type == EfiConventionalMemory || MM->Type == EfiPersistentMemory){
                Con->frontColor = hex(0x00FF00);
                Con->backColor = hex(0x00FF00); //green
                totalUsable += MM->NumberOfPages;
            }
            else{
                Con->frontColor = hex(0xFF0000);
                Con->backColor = hex(0xFF0000); //red
            }
            printf(Con, " ");
            //reset color
            Con->frontColor = oldColor;
            Con->backColor = oldBackColor;
            printf(Con, "%s ", type_arr[MM->Type]);
        }
        else{
            Con->frontColor = hex(0xFFFF00);
            Con->backColor = hex(0xFFFF00); //yellow
            printf(Con, " ");
            //reset color
            Con->frontColor = oldColor;
            Con->backColor = oldBackColor;
            printf(Con, "0x%x ", MM->Type);
        }
        printf(Con, " ");

        printf(Con, "Range:0x%lx - 0x%lx ", MM->PhysicalStart, (MM->PhysicalStart + (MM->NumberOfPages*4096) - 1));
        if(MM->Attribute & 0x1)printf(Con, "UC ");
        if(MM->Attribute & 0x2)printf(Con, "WC ");
        if(MM->Attribute & 0x4)printf(Con, "WT ");
        if(MM->Attribute & 0x8)printf(Con, "WB ");
        if(MM->Attribute & 0x10)printf(Con, "UCE ");
        if(MM->Attribute & 0x1000)printf(Con, "WP ");
        if(MM->Attribute & 0x2000)printf(Con, "RP ");
        if(MM->Attribute & 0x4000)printf(Con, "XP ");
        if(MM->Attribute & 0x8000)printf(Con, "NV ");
        if(MM->Attribute & 0x10000)printf(Con, "MORE_RELIABLE ");
        if(MM->Attribute & 0x20000)printf(Con, "RO ");
        if(MM->Attribute & 0x40000)printf(Con, "SP ");
        if(MM->Attribute & 0x80000)printf(Con, "CRYPTO ");
        if(MM->Attribute & 0x8000000000000000)printf(Con, "RUNTIME ");
        if(MM->Attribute & 0x4000000000000000)printf(Con, "ISA_VALID ");
        if(MM->Attribute & 0x0FFFF00000000000)printf(Con, "ISA_MASK ");

        printf(Con, "\r\n");
        //go to next one
        MM = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)MM + memory_map_descriptor_size);
        update_framebuffer();
        //break here
        //for(uint32_t i=0;i<400000;i++);
    }
    printf(Con, "Total mapped memory: %d pages/%f GB/%f GiB\r\n", totalMapped, totalMapped/250000.0f, totalMapped/262144.0f);
    printf(Con, "Total usable memory: %d pages/%f GB/%f GiB\r\n", totalUsable, totalUsable/250000.0f, totalUsable/262144.0f);
}

//config table related functions
void* getConfigTable(EFI_CONFIGURATION_TABLE* tablePtr, uint64_t entries, uint8_t tableindex){

    static EFI_GUID GUIDTableKeys[] = {
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
    char* GUIDTableValues[] = {
        "ACPI 2.0 TABLE",
        "ACPI TABLE",
        "SAL SYSTEM TABLE",
        "SMBIOS TABLE",
        "SMBIOS3 TABLE",
        "MPS TABLE",
        "JSON CONFIG DATA TABLE",
        "JSON CAPSULE DATA TABLE",
        "JSON CAPSULE RESULT TABLE",
        "DTB TABLE",
        "RT PROPERTIES TABLE",
        "MEMORY ATTRIBUTES TABLE",
        "CONFORMANCE PROFILE TABLE",
        "CONFORMANCE PROFILES UEFI SPEC",
        "MEMORY RANGE CAPSULE",
        "DEBUG IMAGE INFO TABLE",
        "SYSTEM RESOURCE TABLE"
        "IMAGE SECURITY DATABASE"
    };
    EFI_CONFIGURATION_TABLE table;
    for(uint64_t entry = 0;entry < entries;entry++){
        table = tablePtr[entry]; 
        if(cmpGUID(&table.VendorGuid, &GUIDTableKeys[tableindex])){
            return tablePtr->VendorTable;
        }
    }
    return NULL;
}
void viewConfigTables(KERNEL_TEXT_OUTPUT* con, EFI_CONFIGURATION_TABLE* tablePtr, uint64_t entries){

    static EFI_GUID GUIDTableKeys[] = {
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
    char* GUIDTableValues[] = {
        "ACPI 2.0 TABLE",
        "ACPI TABLE",
        "SAL SYSTEM TABLE",
        "SMBIOS TABLE",
        "SMBIOS3 TABLE",
        "MPS TABLE",
        "JSON CONFIG DATA TABLE",
        "JSON CAPSULE DATA TABLE",
        "JSON CAPSULE RESULT TABLE",
        "DTB TABLE",
        "RT PROPERTIES TABLE",
        "MEMORY ATTRIBUTES TABLE",
        "CONFORMANCE PROFILE TABLE",
        "CONFORMANCE PROFILES UEFI SPEC",
        "MEMORY RANGE CAPSULE",
        "DEBUG IMAGE INFO TABLE",
        "SYSTEM RESOURCE TABLE"
        "IMAGE SECURITY DATABASE"
    };
    EFI_CONFIGURATION_TABLE table;

    printf(con, "Number of configuration table entries: %u\r\n", entries);
    for(uint64_t entry = 0;entry < entries;entry++){
        table = tablePtr[entry];
        printf(con, "Table #%u:", entry+1);
        printGUID(con, &table.VendorGuid);
        for(uint32_t guidIndex = 0;guidIndex < (sizeof(GUIDTableKeys)/sizeof(GUIDTableKeys[0]));guidIndex++){
            if(cmpGUID(&table.VendorGuid, &GUIDTableKeys[guidIndex])){
                printf(con, "  %s", GUIDTableValues[guidIndex]);
                break;
            }
        }
        printf(con, "  ptr: %lx\r\n", tablePtr->VendorTable);
    }
    printf(con, "----------END----------\r\n");
}

void printGUID(KERNEL_TEXT_OUTPUT* con, EFI_GUID* guid){
    printf(con, "GUID: %x-%x-%x-", guid->Data1, guid->Data2, guid->Data3);
    printf(con, "%x-%x-%x-%x-%x-%x-%x-%x",
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

//PIC functions
static inline void PIC_disable(){
    outb(0x21, 0xff); //mask master PIC
    outb(0xA1, 0xff); //mask slave PIC
}


/*
//dynamic memory allocation functions
void subpage_alloc_init(KERNEL_SUBPAGE_ALLOCATOR* alloc){
    alloc->freeListStart = (uint64_t*)heap_alloc(alloc->heap, 1);

    uint64_t prev_addr = (uint64_t)alloc->freeListStart;
    uint64_t curr_addr;
    for(uint8_t offset = 1;offset < 64;offset++){
        curr_addr = (uint64_t)(alloc->freeListStart + 64 * offset); //current block
        *(uint64_t*)prev_addr = curr_addr; //prev points to current block
        prev_addr = curr_addr; //update prev pointer
    }
    *(uint64_t*)prev_addr = 0; //last page(node in linked list)
    alloc->freeListEnd = (uint64_t*)prev_addr;
}
void* subpage_alloc(KERNEL_SUBPAGE_ALLOCATOR* alloc){
    if(alloc->freeListStart == NULL){
        return NULL;
    }
    void* ret = (void*)alloc->freeListStart;
    alloc->freeListStart = (uint64_t*)(*alloc->freeListStart);
    return ret;
}
void subpage_free(KERNEL_SUBPAGE_ALLOCATOR* alloc, void* addr){
    if(addr == NULL){
        return;
    }
    *((uint64_t*)addr) = (uint64_t)alloc->freeListStart;
    alloc->freeListStart = (uint64_t*)addr;
}
void subpage_alloc_expand(KERNEL_SUBPAGE_ALLOCATOR* alloc){
    uint8_t* newListStart = heap_alloc(alloc->heap, 1);

    uint64_t prev_addr = (uint64_t)newListStart;
    uint64_t curr_addr;
    for(uint8_t offset = 1;offset < 64;offset++){
        curr_addr = (uint64_t)(newListStart + 64 * offset); //current block
        *(uint64_t*)prev_addr = curr_addr; //prev points to current block
        prev_addr = curr_addr; //update prev pointer
    }
    *(uint64_t*)prev_addr = 0; //last page(node in linked list)
    uint8_t* newListEnd = (uint8_t*)prev_addr;

    *alloc->freeListEnd = (uint64_t)newListStart; //end of old last subpage points to first new subpage
    alloc->freeListEnd = (uint64_t*)newListEnd;
}



void heap_init(KERNEL_HEAP* heap){
    //zero out heap map
    for(uint16_t offset = 0;offset < 4096;offset++){
        *(heap->map + offset) = 0;
    }
}
void* heap_alloc(KERNEL_HEAP* heap, uint64_t pages){
    //two pointers to check start block to end block
    uint64_t start_block = 0;
    uint64_t end_block = 0;
    //go through heap map
    for(uint16_t offset = 0;offset < 4096;offset++){
        //loop through every bit
        for(int8_t shift = 7;shift >= 0;shift--){
            uint8_t byte = *(uint8_t*)(heap->map + offset);
            uint8_t bit = byte & (1 << shift);
            //move end block
            end_block++;
            if(bit){ //if allocated, move start to end
                start_block = end_block;
            }
            //count if free block size is enough to store the requested amount
            if(end_block - start_block >= pages){
                goto found_free_pages;
            }
        }
    }
    return (void*)0; //return null
    found_free_pages:
    //write 1 to the heap map range start_block to end_block
    for(uint64_t free_block = start_block;free_block < end_block;free_block++){
        //calculate heap map address and shift from free_block
        uint8_t heap_map_offset = free_block / 8;
        uint8_t heap_map_shift = 7 - (free_block % 8);
        //write 1
        heap->map[heap_map_offset] |= 1 << heap_map_shift;
    }
    //return address of start_block in the heap
    return (void*)(heap->heap + 4096 * start_block);
}
void heap_free(KERNEL_HEAP* heap, void* addr, uint64_t pages){
    //calculate start block
    uint64_t start_block = ((uint8_t*)addr - heap->heap)/4096;
    for(uint64_t c = start_block;c < start_block + pages;c++){
        //calculate heap map address and shift from free_block
        uint8_t heap_map_offset = c / 8;
        uint8_t heap_map_shift = 7 - (c % 8);
        //write 1
        heap->map[heap_map_offset] &= ~(1 << heap_map_shift);
    }
}
void heap_display(KERNEL_HEAP* heap, EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut){
    uint64_t displayed = 0;
    uint64_t limit = 30;
    //go through heap map
    for(uint64_t offset = 0;offset < 4096;offset++){
        //loop through every bit
        for(int8_t shift = 7;shift >= 0;shift--){
            uint8_t byte = *(uint8_t*)(heap->map + offset);
            uint8_t bit = byte & (1 << shift);
            if(bit){
                printf(GOP, ConOut, "1");
            }
            else{
                printf(GOP, ConOut, "0");
            }
            displayed++;
            if(displayed >= limit){
                return;
            }
        }
    }
}
*/

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

//graphical functions
void NVIDEOParseHeader(KERNEL_NVIDEO* video, uint8_t* addr){
    video->addr = addr + 16;
    video->format = *(uint32_t*)addr;
    video->width = *(uint32_t*)(addr+4);
    video->height = *(uint32_t*)(addr+8);
    video->frameCount = *(uint32_t*)(addr+12);
    video->frameCounter = 0;
}
void GOPPlayVideo(uint32_t x, uint32_t y, KERNEL_NVIDEO* video, bool loop){
    if(video->frameCounter >= video->frameCount){
        return;
    }
    switch(video->format){
        case 0: { //black and white, bitmap
            uint8_t* addr = (uint8_t*)(video->addr); //store original address
            video->addr = video->addr + ((video->frameCounter) * (((video->width+7) / 8) * video->height));
            GOPDrawImage(x, y, video);
            video->addr = addr;

            (video->frameCounter)++;
            if(video->frameCounter >= video->frameCount && loop){
                video->frameCounter = 0;
            }
            break;
        }
        case 1: { //black and white, RLE
            uint8_t* p = video->addr; //start of stream

            //skip frames before target frame
            for(uint32_t f = 0; f < video->frameCounter; f++){
                uint32_t pixels = 0;
                while(pixels < video->width * video->height){
                    int16_t run = *(int16_t*)p;
                    p += sizeof(int16_t);

                    int count = run < 0 ? -run : run;
                    pixels += count;
                }
            }

            //Now p points to start of current frame’s runs
            uint8_t* addr = (uint8_t*)(video->addr);
            video->addr = p;
            GOPDrawImage(x, y, video);

            video->addr = addr; //restore original address

            (video->frameCounter)++;
            if(video->frameCounter >= video->frameCount && loop){
                video->frameCounter = 0;
            }
            break;
        }
    }
}
void GOPDrawImage(uint32_t x, uint32_t y, KERNEL_NVIDEO* img){
    switch(img->format){
        case 0: { //black and white, packed, imgwidth is number of pixels, imgheight is number of pixels
            uint32_t bpr = (img->width + 7) / 8; // bytes per row

            for(uint32_t row = 0; row < img->height; row++){
                for(uint32_t byte = 0; byte < bpr; byte++){
                    uint8_t b = img->addr[row * bpr + byte];
                    uint32_t draw_x = x + byte * 8;
                    uint32_t draw_y = y + row;

                    //compute how many bits to draw in this byte
                    uint8_t bits_in_byte = 8;

                    //if this is the last byte in the row, clamp to remaining pixels
                    if(byte == bpr - 1){
                        uint32_t rem = img->width % 8;
                        if (rem != 0) bits_in_byte = rem;
                    }

                    // draw only the valid bits
                    for(int8_t shift = 7; shift >= 8 - bits_in_byte; shift--){
                        uint32_t mask = 1 << shift;
                        uint32_t color = (b & mask) ? 0xFFFFFFFF : 0xFF000000;
                        GOPPutPixel(draw_x + (7 - shift), draw_y, color);
                    }
                }
            }
            break;
        }
        case 1: { //RLE, cutoff at frame end
            uint8_t* p = img->addr;
            uint32_t pixel = 0;
            uint32_t total = img->width * img->height;

            while(pixel < total){
                int16_t run = *(int16_t*)p;
                p += sizeof(int16_t);

                int count = run < 0 ? -run : run;
                uint32_t color = (run < 0) ? 0xFF000000 : 0xFFFFFFFF;

                for(int i=0; i < count && pixel < total; i++, pixel++){
                    uint32_t px = pixel % img->width;
                    uint32_t py = pixel / img->width;
                    GOPPutPixel(x + px, y + py, color);
                }
            }
            break;
        }
        case 2: { //RGB 3 byte per pixel
            uint32_t bpr = img->width * 3; //bytes per row
            for(uint32_t row = 0;row < img->height;row++){
                for(uint32_t col = 0;col < img->width;col++){
                    //row, col is pixel position
                    //get pixel byte position
                    uint32_t color_byte = *(uint32_t*)(img->addr + row * bpr + col * 3);
                    uint32_t draw_x = x + col;
                    uint32_t draw_y = y + row;
                    GOPPutPixel(draw_x, draw_y, hex(color_byte)); //color converted from RGB to ARGB
                }
            }
            break;
        }
        case 3: { //ARGB 4 byte per pixel, alpha 0 = transparent
            uint32_t bpr = img->width * 4; //bytes per row
            for(uint32_t row = 0;row < img->height;row++){
                for(uint32_t col = 0;col < img->width;col++){
                    //row, col is pixel position
                    //get pixel byte position
                    uint32_t color_byte = *(uint32_t*)(img->addr + row * bpr + col * 4);
                    uint32_t draw_x = x + col;
                    uint32_t draw_y = y + row;
                    if(color_byte & 0xFF000000){
                        GOPPutPixel(draw_x, draw_y, color_byte); //color is ARGB
                    }
                }
            }
            break;
        }
    }
}
void printf(KERNEL_TEXT_OUTPUT* ConOut, char* str, ...){
    va_list args;
    va_start(args, str);

    bool longType = false;
    while (*str) {
        if(*str != '%'){ //if not a format specifier
            printChar(ConOut, *str++);
            continue;
        }
        str++; //skip the '%'

        if(*str == 0){
            break;
        }
        if(*str == '%'){
            printChar(ConOut, '%');
            str++;
            continue;
        }

        longType = false;
        if(*str == 'l'){
            longType = true;
            str++;
            if(*str == 0){
                break;
            }
        }

        char format = *str; //character
        switch(format){
            case 'c': //char
                printChar(ConOut, (uint8_t)va_arg(args, int32_t));
                break;
            case 's': //string
                printString(ConOut, (char*)va_arg(args, uint8_t*));
                break;

            case 'd':
            case 'i': //signed int
                if(longType){
                    printInt(ConOut, (int64_t)va_arg(args, int64_t), 10);
                }
                else{
                    printInt(ConOut, (int64_t)va_arg(args, int32_t), 10); 
                }
                break;
            
            case 'u': //unsigned int
                if(longType){
                    printUint(ConOut, (uint64_t) va_arg(args, uint64_t), 10);
                }
                else{
                    printUint(ConOut, (uint64_t) va_arg(args, uint32_t), 10);
                }
                break;

            case 'o': //octal
                if(longType){
                    printUint(ConOut, (uint64_t) va_arg(args, uint64_t), 8);
                }
                else{
                    printUint(ConOut, (uint64_t) va_arg(args, uint32_t), 8);
                }
                break;
            case 'x': //hex
            case 'X': //hex
                if(longType){
                    printUint(ConOut, (uint64_t) va_arg(args, uint64_t), 16);
                }
                else{
                    printUint(ConOut, (uint64_t) va_arg(args, uint32_t), 16);
                }
                break;

            case 'p': //pointer
                printUint(ConOut, (uint64_t)va_arg(args, void*), 16);
                break;
            
            case 'f': //float
                if(longType){
                    printFloat(ConOut, (double) va_arg(args, double), 16); 
                }
                else{
                    printFloat(ConOut, (double) va_arg(args, double), 6);
                }
                break;
            case 'l':
                printChar(ConOut, 'l');
                break;
            case 'n': //nothing
                break;
            default:
                //nuh uh, print the character itself
                printChar(ConOut, '%');
                printChar(ConOut, format);
                str--;
                break;
        }
        str++;
    }
    va_end(args);
}
void printFloat(KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec){
    if(num < 0.0){
        printChar(ConOut, '-');
        num = -num;
    }
    printUfloat(ConOut, num, prec);
}
void printUfloat(KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec){
    printUint(ConOut, (uint64_t)num, 10); //print integer part
    printChar(ConOut, '.');
    num -= (double)(uint64_t)num;
    uint8_t digit;
    for(uint8_t c = 0;c < prec;c++){
        num *= 10.0;
        digit = (uint8_t)num;
        printChar(ConOut, digit + '0');
        num -= digit;
    }
}
void printInt(KERNEL_TEXT_OUTPUT* ConOut, int64_t num, uint8_t base){
    if(num < 0){
        printChar(ConOut, '-');
        printUint(ConOut, (uint64_t)(-num), base);
        return;
    }
    printUint(ConOut, num, base);
}
void printUint(KERNEL_TEXT_OUTPUT* ConOut, uint64_t num, uint8_t base){
    if(base < 2 || base > 16)return;

    if(base == 2)printString(ConOut, "0b");
    else if(base == 8)printString(ConOut, "0o");
    else if(base == 16)printString(ConOut, "0x");

    if(num == 0){
        printChar(ConOut, '0');
        return;
    }

    uint8_t buff[65]; //buffer to store digits
    buff[64] = 0;
    uint8_t index = 64;

    uint8_t* charmap = (uint8_t*)"0123456789ABCDEF"; //character map

    while(num != 0){
        index--;
        buff[index] = charmap[num % base]; //push digit to buffer
        num /= base;
    }
    printString(ConOut, (char*)&buff[index]);
}
void printString(KERNEL_TEXT_OUTPUT* ConOut, char* string){
    while(*string){ //while it's not null
        printChar(ConOut, *string++);
    }
}
void printChar(KERNEL_TEXT_OUTPUT* ConOut, char ascii_char){
    uint32_t maxWidth = global_ctx->GOP->Info->HorizontalResolution / (ConOut->charWidth*ConOut->scaleX);
    uint32_t maxHeight = global_ctx->GOP->Info->VerticalResolution / (ConOut->charHeight*ConOut->scaleY);
    if(ascii_char == 0){ //null character
        return;
    }
    if(ascii_char == '\r'){
        ConOut->cursorX = 0;
        return;
    }
    if(ascii_char == '\n'){
        ConOut->cursorY++;
        if(ConOut->cursorY >= maxHeight){
            ConOut->cursorY = 0; //TODO: replace this line with scroll
        }
        return;
    }
    if(ascii_char < 32 || ascii_char > 126){ //non printable
        ascii_char = ' '; //space
    }

    //print ascii character at cursorX, cursorY
    uint32_t screenX = ConOut->cursorX * ConOut->charWidth * ConOut->scaleX; //screen position to print character to
    uint32_t screenY = ConOut->cursorY * ConOut->charHeight * ConOut->scaleY;

    screenX += (ConOut->useAbsolutePosition) ? ConOut->offsetX : 0;
    screenY += (ConOut->useAbsolutePosition) ? ConOut->offsetY : 0;

    uint32_t startingIndex = (ascii_char-32)*ConOut->charHeight; //starting index of the font array for the character
    for(uint32_t dy = 0;dy < ConOut->charHeight;dy++){
        uint8_t row = ConOut->font[startingIndex + dy];
        for(uint32_t dx = 0;dx < ConOut->charWidth;dx++){
            uint32_t color = (row & (1 << (7 - dx))) ? ConOut->frontColor : ConOut->backColor;
            for(uint32_t scaleYOff = 0;scaleYOff < ConOut->scaleY;scaleYOff++){
                for(uint32_t scaleXOff = 0;scaleXOff < ConOut->scaleX;scaleXOff++){
                    GOPPutPixel(screenX+(dx*ConOut->scaleX)+scaleXOff, screenY+(dy*ConOut->scaleY)+scaleYOff, color);
                }
            }
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

void GOPDrawRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t pixel, uint8_t fill){
    if(x >= global_ctx->width || y >= global_ctx->height){
        return;
    }
    if(w == 0 || h == 0){
        return;
    }
    if(x + w > global_ctx->width){
        w = global_ctx->width - x;
    }
    if(y + h > global_ctx->height){
        h = global_ctx->height - y;
    }

    if(fill){
        for(uint32_t row=0;row<h;row++){
            uint32_t* ptr = (uint32_t*)global_ctx->fb + (y + row) * global_ctx->pitch + x;
            kmemset32(ptr, pixel, w);
        }
        mark_dirty_rect(x, y, w, h);
    }
    else{
        //top edge
        uint32_t* top = (uint32_t*)global_ctx->fb + y * global_ctx->pitch + x;
        kmemset32(top, pixel, w);
        //bottom edge (only if height > 1)
        if(h > 1){
            uint32_t* bottom = (uint32_t*)global_ctx->fb + (y + h - 1) * global_ctx->pitch + x;
            kmemset32(bottom, pixel, w);
        }
        //LR edges
        for(uint32_t row=1;row < h-1;row++){
            uint32_t* ptr = (uint32_t*)global_ctx->fb + (y + row) * global_ctx->pitch + x;
            ptr[0] = pixel;
            if(w > 1){
                ptr[w - 1] = pixel;
            }
        }

        mark_dirty_rect(x, y, w, h);
    }
}

static inline void mark_dirty(uint32_t x, uint32_t y){
    if(x >= global_ctx->width || y >= global_ctx->height){
        return;
    }
    uint64_t tile_size = global_ctx->dirty_tile_size;

    uint64_t tile_x = x / tile_size;
    uint64_t tile_y = y / tile_size;

    if(tile_x >= global_ctx->dirty_tiles_x || tile_y >= global_ctx->dirty_tiles_y){
        return;
    }

    global_ctx->dirty_tilemap[tile_y * global_ctx->dirty_tiles_x + tile_x] = 1;
}

void mark_dirty_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h){
    if(w == 0 || h == 0){
        return;
    }
    uint64_t tile_size = global_ctx->dirty_tile_size;

    uint64_t tx0 = x / tile_size;
    uint64_t ty0 = y / tile_size;

    uint64_t tx1 = (x + w - 1) / tile_size;
    uint64_t ty1 = (y + h - 1) / tile_size;

    if(tx1 >= global_ctx->dirty_tiles_x){
        tx1 = global_ctx->dirty_tiles_x - 1;
    }
    if(ty1 >= global_ctx->dirty_tiles_y){
        ty1 = global_ctx->dirty_tiles_y - 1;
    }

    for(uint64_t ty = ty0;ty<=ty1;ty++){
        for(uint64_t tx = tx0;tx<=tx1;tx++){
            global_ctx->dirty_tilemap[ty * global_ctx->dirty_tiles_x + tx] = 1;
        }
    }
}

static inline void GOPPutPixel(uint32_t x, uint32_t y, uint32_t pixel){
    /*
    //check if x and y are legal
    uint32_t xMax = GOP->Info->HorizontalResolution;
    uint32_t yMax = GOP->Info->VerticalResolution;
    if(x >= xMax || y >= yMax){
        return;
    }

    uint32_t* fb = (uint32_t*) GOP->FrameBufferBase;
    uint32_t pitch = (uint32_t) GOP->Info->PixelsPerScanLine;

    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    uint32_t pixel;
    if(GOP->Info->PixelFormat == PixelBitMask){
        pixel =
            ((r << GOP->Info->PixelInformation.RedMask)   & GOP->Info->PixelInformation.RedMask) |
            ((g << GOP->Info->PixelInformation.GreenMask) & GOP->Info->PixelInformation.GreenMask) |
            ((b << GOP->Info->PixelInformation.BlueMask)   & GOP->Info->PixelInformation.BlueMask);
    }
    else{
        //ARGB 32-bit
        pixel = (b) | (g << 8) | (r << 16);
    }
    
    */

    *(uint32_t*)(global_ctx->fb + y * global_ctx->pitch * 4 + x * 4) = pixel;
    mark_dirty(x, y);
}
uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    return ((uint32_t)b) | ((uint32_t)g << 8) | ((uint32_t)r << 16) | ((uint32_t)a << 24);
}
uint32_t hex(uint32_t hex){
    return hex | 0xFF000000;
}

void printd(char* str, ...){
    EFI_GOP* GOP = global_ctx->GOP;
    KERNEL_TEXT_OUTPUT* ConOut = &(KERNEL_TEXT_OUTPUT){Terminus8x16_Normal, 8, 16, 1, 1, 0, 8, 0, 0, hex(0xFF10F0), hex(0x000000), false};

    va_list args;
    va_start(args, str);

    bool longType = false;
    while (*str) {
        if(*str != '%'){ //if not a format specifier
            printChar(ConOut, *str++);
            continue;
        }
        str++; //skip the '%'

        if(*str == 0){
            break;
        }
        if(*str == '%'){
            printChar(ConOut, '%');
            str++;
            continue;
        }

        longType = false;
        if(*str == 'l'){
            longType = true;
            str++;
            if(*str == 0){
                break;
            }
        }

        char format = *str; //character
        switch(format){
            case 'c': //char
                printChar(ConOut, (uint8_t)va_arg(args, int32_t));
                break;
            case 's': //string
                printString(ConOut, (char*)va_arg(args, uint8_t*));
                break;

            case 'd':
            case 'i': //signed int
                if(longType){
                    printInt(ConOut, (int64_t)va_arg(args, int64_t), 10);
                }
                else{
                    printInt(ConOut, (int64_t)va_arg(args, int32_t), 10); 
                }
                break;
            
            case 'u': //unsigned int
                if(longType){
                    printUint(ConOut, (uint64_t) va_arg(args, uint64_t), 10);
                }
                else{
                    printUint(ConOut, (uint64_t) va_arg(args, uint32_t), 10);
                }
                break;

            case 'o': //octal
                if(longType){
                    printUint(ConOut, (uint64_t) va_arg(args, uint64_t), 8);
                }
                else{
                    printUint(ConOut, (uint64_t) va_arg(args, uint32_t), 8);
                }
                break;
            case 'x': //hex
            case 'X': //hex
                if(longType){
                    printUint(ConOut, (uint64_t) va_arg(args, uint64_t), 16);
                }
                else{
                    printUint(ConOut, (uint64_t) va_arg(args, uint32_t), 16);
                }
                break;

            case 'p': //pointer
                printUint(ConOut, (uint64_t)va_arg(args, void*), 16);
                break;
            
            case 'f': //float
                if(longType){
                    printFloat(ConOut, (double) va_arg(args, double), 16); 
                }
                else{
                    printFloat(ConOut, (double) va_arg(args, double), 6);
                }
                break;
            case 'l':
                printChar(ConOut, 'l');
                break;
            case 'n': //nothing
                break;
            default:
                //nuh uh, print the character itself
                printChar(ConOut, '%');
                printChar(ConOut, format);
                str--;
                break;
        }
        str++;
    }
    va_end(args);
    update_framebuffer();
}

//general functions
void* kmemcpy(void* dest, const void* source, uint64_t size){
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)source;

    // Align to 8 bytes first
    while(size && ((uintptr_t)d & 7)){
        *d++ = *s++;
        size--;
    }

    // Copy 8 bytes at a time
    uint64_t* d64 = (uint64_t*)d;
    const uint64_t* s64 = (const uint64_t*)s;

    while (size >= 8) {
        *d64++ = *s64++;
        size -= 8;
    }

    d = (uint8_t*)d64;
    s = (const uint8_t*)s64;

    // Copy remainder
    while (size--) {
        *d++ = *s++;
    }

    return dest;
}
void* kmemset(void* dest, int value, uint64_t size){
    uint8_t* d = (uint8_t*)dest;
    uint8_t byte = (uint8_t)value;

    // Align to 8 bytes
    while(size && ((uintptr_t)d & 7)){
        *d++ = byte;
        size--;
    }

    // Create 64-bit pattern
    uint64_t pattern = byte;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    pattern |= pattern << 32;

    uint64_t* d64 = (uint64_t*)d;

    while (size >= 8) {
        *d64++ = pattern;
        size -= 8;
    }

    d = (uint8_t*)d64;

    while (size--) {
        *d++ = byte;
    }

    return dest;
}
void kmemset32(uint32_t* dest, uint32_t value, uint64_t count){
    while (count >= 8) {
        dest[0] = value;
        dest[1] = value;
        dest[2] = value;
        dest[3] = value;
        dest[4] = value;
        dest[5] = value;
        dest[6] = value;
        dest[7] = value;
        dest += 8;
        count -= 8;
    }

    while (count--) {
        *dest++ = value;
    }
}
int kmemcmp(const void* a, const void* b, uint64_t size){
    const uint8_t* p1 = (const uint8_t*)a;
    const uint8_t* p2 = (const uint8_t*)b;

    while(size--){
        if (*p1 != *p2)
            return (*p1 < *p2) ? -1 : 1;
        p1++;
        p2++;
    }

    return 0;
}
void* kmemmove(void* dest, const void* source, uint64_t size){
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)source;

    if(d < s){
        // Forward copy
        while (size--)
            *d++ = *s++;
    } else {
        // Backward copy
        d += size;
        s += size;
        while (size--)
            *--d = *--s;
    }

    return dest;
}

void cpuid(int code, uint32_t* a, uint32_t* d){
    asm volatile("cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx");
}
void cpuid_get_vendor(uint8_t* CPUVendor){
    uint32_t CPUVendor_r[4];
    asm volatile("cpuid":"=a"(*CPUVendor_r),"=b"(*(CPUVendor_r+1)),
                "=c"(*(CPUVendor_r+2)),"=d"(*(CPUVendor_r+3)):"a"(0));

    ((uint32_t*)CPUVendor)[0] = CPUVendor_r[1];
    ((uint32_t*)CPUVendor)[1] = CPUVendor_r[3];
    ((uint32_t*)CPUVendor)[2] = CPUVendor_r[2];
    CPUVendor[12] = 0;

}
uint64_t rdtsc(){
    uint32_t low, high;
    asm volatile("rdtsc":"=a"(low),"=d"(high));
    return ((uint64_t)high << 32) | low;
}

//fonts
    uint8_t VGAfont[] = {
        //32
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //33
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        //34
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010100,
        0b00010100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //35
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100100,
        0b00100100,
        0b01111110,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00100100,
        0b01111110,
        0b00100100,
        0b00100100,
        0b00000000,
        0b00000000,
        0b00000000,
        //36
        0b00000000,
        0b00010000,
        0b00010000,
        0b01111100,
        0b01010000,
        0b01010000,
        0b01010000,
        0b01111100,
        0b00010100,
        0b00010100,
        0b00010100,
        0b01111100,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        //37
        0b00000000,
        0b00000000,
        0b00000000,
        0b01110001,
        0b01010010,
        0b01110010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100111,
        0b01000101,
        0b01000111,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //38
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00100100,
        0b00100100,
        0b00011000,
        0b00010001,
        0b00101010,
        0b01001010,
        0b01000100,
        0b01001100,
        0b00110010,
        0b00000010,
        0b00000000,
        //39
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //40
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00010000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000000,
        0b00000000,
        0b00000000,
        //41
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        //42
        0b00000000,
        0b00100100,
        0b00011000,
        0b01111110,
        0b00011000,
        0b00100100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //43
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b01111100,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //44
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,
        //45
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //46
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        //47
        0b00000000,
        0b00000000,
        0b00000100,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00100000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //48
        0b00000000,
        0b00000000,
        0b00011000,
        0b00100100,
        0b01000010,
        0b01000010,
        0b01000110,
        0b01001010,
        0b01010010,
        0b01100010,
        0b01000010,
        0b01000010,
        0b00100100,
        0b00011000,
        0b00000000,
        0b00000000,
        //49
        0b00000000,
        0b00001000,
        0b00011000,
        0b00101000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //50
        0b00000000,
        0b00000000,
        0b00011000,
        0b00100100,
        0b01000010,
        0b00000010,
        0b00000010,
        0b00001100,
        0b00110000,
        0b01000000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //51
        0b00000000,
        0b00000000,
        0b00011000,
        0b00100100,
        0b01000010,
        0b00000010,
        0b00000100,
        0b00011000,
        0b00000100,
        0b00000010,
        0b01000010,
        0b00100100,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        //52
        0b00000000,
        0b00000000,
        0b00100010,
        0b00100010,
        0b00100010,
        0b00100010,
        0b00111110,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000000,
        0b00000000,
        0b00000000,
        //53
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01111000,
        0b00000100,
        0b00000010,
        0b00000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //54
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011100,
        0b00100000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        //55
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000010,
        0b00000100,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00100000,
        0b00000000,
        0b00000000,
        0b00000000,
        //56
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        //57
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000010,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //58
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //59
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00001000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,
        //60
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //61
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //62
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //63
        0b00000000,
        0b00000000,
        0b00011000,
        0b00100100,
        0b01000010,
        0b01000010,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00011000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        //64
        0b00000000,
        0b00000000,
        0b00111000,
        0b01000100,
        0b10000010,
        0b10000010,
        0b10010010,
        0b10101010,
        0b10011010,
        0b10000100,
        0b01000001,
        0b00100010,
        0b00011100,
        0b00000000,
        0b00000000,
        0b00000000,
        //65
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00100100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        //66
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        //67
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011110,
        0b00100000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00100000,
        0b00011110,
        0b00000000,
        0b00000000,
        0b00000000,
        //68
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111000,
        0b01000100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000100,
        0b01111000,
        0b00000000,
        0b00000000,
        0b00000000,
        //69
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        //70
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //71
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01001110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        //72
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        //73
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //74
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001110,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        //75
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000100,
        0b01001000,
        0b01010000,
        0b01100000,
        0b01010000,
        0b01001000,
        0b01000100,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        //76
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        //77
        0b00000000,
        0b00000000,
        0b00000000,
        0b11000110,
        0b10101010,
        0b10101010,
        0b10010010,
        0b10010010,
        0b10010010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b00000000,
        0b00000000,
        0b00000000,
        //78
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01100010,
        0b01100010,
        0b01010010,
        0b01010010,
        0b01001010,
        0b01001010,
        0b01000110,
        0b01000110,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        //79
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        //80
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //81
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01001010,
        0b01000110,
        0b00111110,
        0b00000011,
        0b00000001,
        0b00000000,
        //82
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01100000,
        0b01010000,
        0b01001000,
        0b01000100,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        //83
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00111100,
        0b00000010,
        0b00000010,
        0b00000010,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //84
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b01111110,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        //85
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        //86
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00100100,
        0b00100100,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //87
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b10000001,
        0b10000001,
        0b10000001,
        0b10011001,
        0b10011001,
        0b10100101,
        0b11000011,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //88
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00100100,
        0b00011000,
        0b00011000,
        0b00100100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        //89
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00100100,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        //90
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,

        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b00100000,
        0b00100000,
        0b00010000,
        0b00010000,
        0b00001000,
        0b00001000,
        0b00000100,
        0b00000100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,

        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00010100,
        0b00100010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,

        0b00000000,
        0b01000000,
        0b00100000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //97
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111000,
        0b00000100,
        0b00111100,
        0b01000100,
        0b01000100,
        0b00111100,
        0b00000010,
        0b00000000,
        //98
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01111100,
        0b00000000,
        0b00000000,
        //99
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00111110,
        0b00000000,
        0b00000000,
        //100
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000000,
        0b00000000,
        //101
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000000,
        0b00111100,
        0b00000000,
        0b00000000,
        //102
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00010100,
        0b00010000,
        0b00010000,
        0b00111000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00000000,
        0b00000000,
        //103
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001100,
        0b00010010,
        0b00010010,
        0b00001110,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00100010,
        0b00011100,
        0b00000000,
        0b00000000,
        0b00000000,
        //104
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00111000,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //105
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00000000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //106
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00000000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b01001000,
        0b00110000,
        0b00000000,
        0b00000000,
        //107
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100000,
        0b00100000,
        0b00100100,
        0b00101000,
        0b00110000,
        0b00101000,
        0b00100100,
        0b00000000,
        0b00000000,
        0b00000000,
        //108
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00001000,
        0b00000000,
        0b00000000,
        0b00000000,
        //109
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000100,
        0b01101100,
        0b01010100,
        0b01000100,
        0b01000100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //110
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000100,
        0b01100100,
        0b01010100,
        0b01001100,
        0b01000100,
        0b01000100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //111
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00100100,
        0b00100100,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //112
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111000,
        0b00100100,
        0b00100100,
        0b00111000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00000000,
        0b00000000,
        //113
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011100,
        0b00100100,
        0b00100100,
        0b00011100,
        0b00000100,
        0b00000100,
        0b00000110,
        0b00000100,
        0b00000000,
        0b00000000,
        //114
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00101100,
        0b00110000,
        0b00100000,
        0b00100000,
        0b00000000,
        0b00000000,
        0b00000000,
        //115
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00100000,
        0b00100000,
        0b00011000,
        0b00000100,
        0b00000100,
        0b00011000,
        0b00000000,
        0b00000000,
        //116
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00011100,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001100,
        0b00000000,
        0b00000000,
        //117
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00011100,
        0b00000010,
        0b00000000,
        0b00000000,
        //118
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100010,
        0b00100010,
        0b00010100,
        0b00001000,
        0b00000000,
        0b00000000,
        0b00000000,
        //119
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100010,
        0b00101010,
        0b00010100,
        0b00000000,
        0b00000000,
        0b00000000,
        //120
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100010,
        0b00010100,
        0b00001000,
        0b00010100,
        0b00100010,
        0b00000000,
        0b00000000,
        //121
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100010,
        0b00010100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b00000000,
        0b00000000,
        //122
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b00111100,
        0b00000000,
        0b00000000,
        //123
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00001000,
        0b00000000,
        0b00000000,
        //124
        0b00000000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00000000,
        //125
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00000000,
        0b00000000,
        //126
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00110000,
        0b01001001,
        0b00000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000
    };
    uint8_t Terminus8x16_Normal[] = {
        //32
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //33
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //34
        0b00000000,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //35
        0b00000000,
        0b00000000,
        0b00100100,
        0b00100100,
        0b00100100,
        0b01111110,
        0b00100100,
        0b00100100,
        0b01111110,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //36
        0b00000000,
        0b00010000,
        0b00010000,
        0b01111100,
        0b10010010,
        0b10010000,
        0b10010000,
        0b01111100,
        0b00010010,
        0b00010010,
        0b10010010,
        0b01111100,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        //37
        0b00000000,
        0b00000000,
        0b01100100,
        0b10010100,
        0b01101000,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00101100,
        0b01010010,
        0b01001100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //38
        0b00000000,
        0b00000000,
        0b00011000,
        0b00100100,
        0b00100100,
        0b00011000,
        0b00110000,
        0b01001010,
        0b01000100,
        0b01000100,
        0b01000100,
        0b00111010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //39
        0b00000000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //40
        0b00000000,
        0b00000000,
        0b00001000,
        0b00010000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //41
        0b00000000,
        0b00000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00010000,
        0b00100000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //42
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100100,
        0b00011000,
        0b01111110,
        0b00011000,
        0b00100100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //43
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b01111100,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //44
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00000000,
        0b00000000,
        0b00000000,
        //45
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //46
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //47
        0b00000000,
        0b00000000,
        0b00000100,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00100000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //48
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000110,
        0b01001010,
        0b01010010,
        0b01100010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //49
        0b00000000,
        0b00000000,
        0b00001000,
        0b00011000,
        0b00101000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //50
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //51
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b00000010,
        0b00011100,
        0b00000010,
        0b00000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //52
        0b00000000,
        0b00000000,
        0b00000010,
        0b00000110,
        0b00001010,
        0b00010010,
        0b00100010,
        0b01000010,
        0b01111110,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //53
        0b00000000,
        0b00000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111100,
        0b00000010,
        0b00000010,
        0b00000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //54
        0b00000000,
        0b00000000,
        0b00011100,
        0b00100000,
        0b01000000,
        0b01000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //55
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000010,
        0b00000010,
        0b00000100,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //56
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //57
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000010,
        0b00000010,
        0b00000100,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //58
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //59
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00000000,
        0b00000000,
        0b00000000,
        //60
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //61
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //62
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //63
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00000000,
        0b00001000,
        0b00001000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //64
        0b00000000,
        0b00000000,
        0b01111100,
        0b10000010,
        0b10011110,
        0b10100010,
        0b10100010,
        0b10100010,
        0b10100110,
        0b10011010,
        0b10000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //65
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //66
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //67
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //68
        0b00000000,
        0b00000000,
        0b01111000,
        0b01000100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000100,
        0b01111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //69
        0b00000000,
        0b00000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //70
        0b00000000,
        0b00000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //71
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000000,
        0b01000000,
        0b01001110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //72
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //73
        0b00000000,
        0b00000000,
        0b00111000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //74
        0b00000000,
        0b00000000,
        0b00001110,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b01000100,
        0b01000100,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //75
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000100,
        0b01001000,
        0b01010000,
        0b01100000,
        0b01100000,
        0b01010000,
        0b01001000,
        0b01000100,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //76
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //77
        0b00000000,
        0b00000000,
        0b10000010,
        0b11000110,
        0b10101010,
        0b10010010,
        0b10010010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //78
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01100010,
        0b01010010,
        0b01001010,
        0b01000110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //79
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //80
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //81
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01001010,
        0b00111100,
        0b00000010,
        0b00000000,
        0b00000000,
        0b00000000,
        //82
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01010000,
        0b01001000,
        0b01000100,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //83
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000000,
        0b01000000,
        0b00111100,
        0b00000010,
        0b00000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //84
        0b00000000,
        0b00000000,
        0b11111110,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //85
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //86
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //87
        0b00000000,
        0b00000000,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10000010,
        0b10010010,
        0b10010010,
        0b10101010,
        0b11000110,
        0b10000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //88
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b00100100,
        0b00100100,
        0b00011000,
        0b00011000,
        0b00100100,
        0b00100100,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //89
        0b00000000,
        0b00000000,
        0b10000010,
        0b10000010,
        0b01000100,
        0b01000100,
        0b00101000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //90
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000010,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //91
        0b00000000,
        0b00000000,
        0b00111000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //92
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b00100000,
        0b00100000,
        0b00010000,
        0b00010000,
        0b00001000,
        0b00001000,
        0b00000100,
        0b00000100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //93
        0b00000000,
        0b00000000,
        0b00111000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //94
        0b00000000,
        0b00010000,
        0b00101000,
        0b01000100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //95
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,
        //96
        0b00010000,
        0b00001000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //97
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b00000010,
        0b00111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //98
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //99
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //100
        0b00000000,
        0b00000000,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //101
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01111110,
        0b01000000,
        0b01000000,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //102
        0b00000000,
        0b00000000,
        0b00001110,
        0b00010000,
        0b00010000,
        0b01111100,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //103
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000010,
        0b00000010,
        0b00111100,
        0b00000000,
        //104
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //105
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00110000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //106
        0b00000000,
        0b00000000,
        0b00000100,
        0b00000100,
        0b00000000,
        0b00001100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b01000100,
        0b01000100,
        0b00111000,
        0b00000000,
        //107
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000010,
        0b01000100,
        0b01001000,
        0b01110000,
        0b01001000,
        0b01000100,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //108
        0b00000000,
        0b00000000,
        0b00110000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //109
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111100,
        0b10010010,
        0b10010010,
        0b10010010,
        0b10010010,
        0b10010010,
        0b10010010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //110
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //111
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //112
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00000000,
        //113
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000000,
        //114
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01011110,
        0b01100000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //115
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b01000000,
        0b01000000,
        0b00111100,
        0b00000010,
        0b00000010,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //116
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b01111100,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00001110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //117
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //118
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00100100,
        0b00100100,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //119
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b10000010,
        0b10000010,
        0b10010010,
        0b10010010,
        0b10010010,
        0b10010010,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //120
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b00100100,
        0b00011000,
        0b00100100,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //121
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000010,
        0b00000010,
        0b00111100,
        0b00000000,
        //122
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //123
        0b00000000,
        0b00000000,
        0b00001100,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00001100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //124
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //125
        0b00000000,
        0b00000000,
        0b00110000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //126
        0b00000000,
        0b01100010,
        0b10010010,
        0b10001100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    };
    uint8_t Terminus8x16_Bold[] = {
        //32
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //33
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //34
        0b00000000,
        0b01100110,
        0b01100110,
        0b01100110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //35
        0b00000000,
        0b00000000,
        0b01101100,
        0b01101100,
        0b01101100,
        0b11111110,
        0b01101100,
        0b01101100,
        0b11111110,
        0b01101100,
        0b01101100,
        0b01101100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //36
        0b00000000,
        0b00010000,
        0b00010000,
        0b01111100,
        0b11010110,
        0b11010000,
        0b11010000,
        0b01111100,
        0b00010110,
        0b00010110,
        0b11010110,
        0b01111100,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        //37
        0b00000000,
        0b00000000,
        0b01100110,
        0b11010110,
        0b01101100,
        0b00001100,
        0b00011000,
        0b00011000,
        0b00110000,
        0b00110110,
        0b01101011,
        0b01100110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //38
        0b00000000,
        0b00000000,
        0b00111000,
        0b01101100,
        0b01101100,
        0b00111000,
        0b01110110,
        0b11011100,
        0b11001100,
        0b11001100,
        0b11011100,
        0b01110110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //39
        0b00000000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //40
        0b00000000,
        0b00000000,
        0b00001100,
        0b00011000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00011000,
        0b00001100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //41
        0b00000000,
        0b00000000,
        0b00110000,
        0b00011000,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00011000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //42
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01101100,
        0b00111000,
        0b11111110,
        0b00111000,
        0b01101100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //43
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b01111110,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //44
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,
        //45
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //46
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //47
        0b00000000,
        0b00000000,
        0b00000110,
        0b00000110,
        0b00001100,
        0b00001100,
        0b00011000,
        0b00011000,
        0b00110000,
        0b00110000,
        0b01100000,
        0b01100000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //48
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11001110,
        0b11011110,
        0b11110110,
        0b11100110,
        0b11000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //49
        0b00000000,
        0b00000000,
        0b00011000,
        0b00111000,
        0b01111000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //50
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b00000110,
        0b00001100,
        0b00011000,
        0b00110000,
        0b01100000,
        0b11000000,
        0b11111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //51
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b00000110,
        0b00111100,
        0b00000110,
        0b00000110,
        0b11000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //52
        0b00000000,
        0b00000000,
        0b00000110,
        0b00001110,
        0b00011110,
        0b00110110,
        0b01100110,
        0b11000110,
        0b11111110,
        0b00000110,
        0b00000110,
        0b00000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //53
        0b00000000,
        0b00000000,
        0b11111110,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11111100,
        0b00000110,
        0b00000110,
        0b00000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //54
        0b00000000,
        0b00000000,
        0b00111100,
        0b01100000,
        0b11000000,
        0b11000000,
        0b11111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //55
        0b00000000,
        0b00000000,
        0b11111110,
        0b00000110,
        0b00000110,
        0b00001100,
        0b00001100,
        0b00011000,
        0b00011000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //56
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //57
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111110,
        0b00000110,
        0b00000110,
        0b00001100,
        0b01111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //58
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //59
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,
        //60
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000110,
        0b00001100,
        0b00011000,
        0b00110000,
        0b01100000,
        0b00110000,
        0b00011000,
        0b00001100,
        0b00000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //61
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111110,
        0b00000000,
        0b00000000,
        0b11111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //62
        0b00000000,
        0b00000000,
        0b00000000,
        0b01100000,
        0b00110000,
        0b00011000,
        0b00001100,
        0b00000110,
        0b00001100,
        0b00011000,
        0b00110000,
        0b01100000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //63
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b00001100,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //64
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11001110,
        0b11010110,
        0b11010110,
        0b11010110,
        0b11010110,
        0b11001110,
        0b11000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //65
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11111110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //66
        0b00000000,
        0b00000000,
        0b11111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //67
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //68
        0b00000000,
        0b00000000,
        0b11111000,
        0b11001100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11001100,
        0b11111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //69
        0b00000000,
        0b00000000,
        0b11111110,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11111000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //70
        0b00000000,
        0b00000000,
        0b11111110,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11111000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //71
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11000000,
        0b11000000,
        0b11011110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //72
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11111110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //73
        0b00000000,
        0b00000000,
        0b00111100,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //74
        0b00000000,
        0b00000000,
        0b00011110,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b11001100,
        0b11001100,
        0b01111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //75
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11001100,
        0b11011000,
        0b11110000,
        0b11110000,
        0b11011000,
        0b11001100,
        0b11000110,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //76
        0b00000000,
        0b00000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //77
        0b00000000,
        0b00000000,
        0b10000010,
        0b11000110,
        0b11101110,
        0b11111110,
        0b11010110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //78
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11100110,
        0b11110110,
        0b11011110,
        0b11001110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //79
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //80
        0b00000000,
        0b00000000,
        0b11111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11111100,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //81
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11011110,
        0b01111100,
        0b00000110,
        0b00000000,
        0b00000000,
        0b00000000,
        //82
        0b00000000,
        0b00000000,
        0b11111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11111100,
        0b11110000,
        0b11011000,
        0b11001100,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //83
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000000,
        0b11000000,
        0b01111100,
        0b00000110,
        0b00000110,
        0b11000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //84
        0b00000000,
        0b00000000,
        0b11111111,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //85
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //86
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01101100,
        0b01101100,
        0b01101100,
        0b00111000,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //87
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11010110,
        0b11111110,
        0b11101110,
        0b11000110,
        0b10000010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //88
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b01101100,
        0b01101100,
        0b00111000,
        0b00111000,
        0b01101100,
        0b01101100,
        0b11000110,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //89
        0b00000000,
        0b00000000,
        0b11000011,
        0b11000011,
        0b01100110,
        0b01100110,
        0b00111100,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //90
        0b00000000,
        0b00000000,
        0b11111110,
        0b00000110,
        0b00000110,
        0b00001100,
        0b00011000,
        0b00110000,
        0b01100000,
        0b11000000,
        0b11000000,
        0b11111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //91
        0b00000000,
        0b00000000,
        0b00111100,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //92
        0b00000000,
        0b00000000,
        0b01100000,
        0b01100000,
        0b00110000,
        0b00110000,
        0b00011000,
        0b00011000,
        0b00001100,
        0b00001100,
        0b00000110,
        0b00000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //93
        0b00000000,
        0b00000000,
        0b00111100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00001100,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //94
        0b00000000,
        0b00011000,
        0b00111100,
        0b01100110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //95
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111110,
        0b00000000,
        0b00000000,
        //96
        0b00110000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //97
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b00000110,
        0b01111110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //98
        0b00000000,
        0b00000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //99
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //100
        0b00000000,
        0b00000000,
        0b00000110,
        0b00000110,
        0b00000110,
        0b01111110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //101
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11111110,
        0b11000000,
        0b11000000,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //102
        0b00000000,
        0b00000000,
        0b00011110,
        0b00110000,
        0b00110000,
        0b11111100,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //103
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111110,
        0b00000110,
        0b00000110,
        0b01111100,
        0b00000000,
        //104
        0b00000000,
        0b00000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //105
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00111000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //106
        0b00000000,
        0b00000000,
        0b00000110,
        0b00000110,
        0b00000000,
        0b00001110,
        0b00000110,
        0b00000110,
        0b00000110,
        0b00000110,
        0b00000110,
        0b00000110,
        0b01100110,
        0b01100110,
        0b00111100,
        0b00000000,
        //107
        0b00000000,
        0b00000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000110,
        0b11001100,
        0b11011000,
        0b11110000,
        0b11011000,
        0b11001100,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //108
        0b00000000,
        0b00000000,
        0b00111000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //109
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111100,
        0b11010110,
        0b11010110,
        0b11010110,
        0b11010110,
        0b11010110,
        0b11010110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //110
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //111
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //112
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11111100,
        0b11000000,
        0b11000000,
        0b11000000,
        0b00000000,
        //113
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111110,
        0b00000110,
        0b00000110,
        0b00000110,
        0b00000000,
        //114
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11011110,
        0b11110000,
        0b11100000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b11000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //115
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b11000000,
        0b11000000,
        0b01111100,
        0b00000110,
        0b00000110,
        0b11111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //116
        0b00000000,
        0b00000000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b11111100,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00011110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //117
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //118
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01101100,
        0b01101100,
        0b00111000,
        0b00111000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //119
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11010110,
        0b11010110,
        0b11010110,
        0b11010110,
        0b01111100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //120
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b01101100,
        0b00111000,
        0b01101100,
        0b11000110,
        0b11000110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //121
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01111110,
        0b00000110,
        0b00000110,
        0b01111100,
        0b00000000,
        //122
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111110,
        0b00001100,
        0b00011000,
        0b00110000,
        0b01100000,
        0b11000000,
        0b11111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //123
        0b00000000,
        0b00000000,
        0b00011100,
        0b00110000,
        0b00110000,
        0b00110000,
        0b01100000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00011100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //124
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //125
        0b00000000,
        0b00000000,
        0b01110000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00001100,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b01110000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        //126
        0b00000000,
        0b01110011,
        0b11011011,
        0b11001110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    };
