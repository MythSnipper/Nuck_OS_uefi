#include "../include/kernel.h"

//global vars
KERNEL_CONTEXT_TABLE* global_ctx;

__attribute__((aligned(0x10)))
GDT_Entry GDT[3];
GDT_Descriptor GDTR;


//IDT --------------------------
__attribute__((aligned(0x10)))IDT_Entry IDT[256];
IDT_Descriptor IDTR;

isr_stub isr_table[256] = {
    isr_stub_0, isr_stub_1, isr_stub_2, isr_stub_3, isr_stub_4, isr_stub_5, isr_stub_6, isr_stub_7, isr_stub_8, isr_stub_9, isr_stub_10, isr_stub_11, isr_stub_12, isr_stub_13, isr_stub_14, isr_stub_15, isr_stub_16, isr_stub_17, isr_stub_18, isr_stub_19, isr_stub_20, isr_stub_21, isr_stub_22, isr_stub_23, isr_stub_24, isr_stub_25, isr_stub_26, isr_stub_27, isr_stub_28, isr_stub_29, isr_stub_30, isr_stub_31, isr_stub_32, isr_stub_33, isr_stub_34, isr_stub_35, isr_stub_36, isr_stub_37, isr_stub_38, isr_stub_39, isr_stub_40, isr_stub_41, isr_stub_42, isr_stub_43, isr_stub_44, isr_stub_45, isr_stub_46, isr_stub_47, isr_stub_48, isr_stub_49, isr_stub_50, isr_stub_51, isr_stub_52, isr_stub_53, isr_stub_54, isr_stub_55, isr_stub_56, isr_stub_57, isr_stub_58, isr_stub_59, isr_stub_60, isr_stub_61, isr_stub_62, isr_stub_63, isr_stub_64, isr_stub_65, isr_stub_66, isr_stub_67, isr_stub_68, isr_stub_69, isr_stub_70, isr_stub_71, isr_stub_72, isr_stub_73, isr_stub_74, isr_stub_75, isr_stub_76, isr_stub_77, isr_stub_78, isr_stub_79, isr_stub_80, isr_stub_81, isr_stub_82, isr_stub_83, isr_stub_84, isr_stub_85, isr_stub_86, isr_stub_87, isr_stub_88, isr_stub_89, isr_stub_90, isr_stub_91, isr_stub_92, isr_stub_93, isr_stub_94, isr_stub_95, isr_stub_96, isr_stub_97, isr_stub_98, isr_stub_99, isr_stub_100, isr_stub_101, isr_stub_102, isr_stub_103, isr_stub_104, isr_stub_105, isr_stub_106, isr_stub_107, isr_stub_108, isr_stub_109, isr_stub_110, isr_stub_111, isr_stub_112, isr_stub_113, isr_stub_114, isr_stub_115, isr_stub_116, isr_stub_117, isr_stub_118, isr_stub_119, isr_stub_120, isr_stub_121, isr_stub_122, isr_stub_123, isr_stub_124, isr_stub_125, isr_stub_126, isr_stub_127, isr_stub_128, isr_stub_129, isr_stub_130, isr_stub_131, isr_stub_132, isr_stub_133, isr_stub_134, isr_stub_135, isr_stub_136, isr_stub_137, isr_stub_138, isr_stub_139, isr_stub_140, isr_stub_141, isr_stub_142, isr_stub_143, isr_stub_144, isr_stub_145, isr_stub_146, isr_stub_147, isr_stub_148, isr_stub_149, isr_stub_150, isr_stub_151, isr_stub_152, isr_stub_153, isr_stub_154, isr_stub_155, isr_stub_156, isr_stub_157, isr_stub_158, isr_stub_159, isr_stub_160, isr_stub_161, isr_stub_162, isr_stub_163, isr_stub_164, isr_stub_165, isr_stub_166, isr_stub_167, isr_stub_168, isr_stub_169, isr_stub_170, isr_stub_171, isr_stub_172, isr_stub_173, isr_stub_174, isr_stub_175, isr_stub_176, isr_stub_177, isr_stub_178, isr_stub_179, isr_stub_180, isr_stub_181, isr_stub_182, isr_stub_183, isr_stub_184, isr_stub_185, isr_stub_186, isr_stub_187, isr_stub_188, isr_stub_189, isr_stub_190, isr_stub_191, isr_stub_192, isr_stub_193, isr_stub_194, isr_stub_195, isr_stub_196, isr_stub_197, isr_stub_198, isr_stub_199, isr_stub_200, isr_stub_201, isr_stub_202, isr_stub_203, isr_stub_204, isr_stub_205, isr_stub_206, isr_stub_207, isr_stub_208, isr_stub_209, isr_stub_210, isr_stub_211, isr_stub_212, isr_stub_213, isr_stub_214, isr_stub_215, isr_stub_216, isr_stub_217, isr_stub_218, isr_stub_219, isr_stub_220, isr_stub_221, isr_stub_222, isr_stub_223, isr_stub_224, isr_stub_225, isr_stub_226, isr_stub_227, isr_stub_228, isr_stub_229, isr_stub_230, isr_stub_231, isr_stub_232, isr_stub_233, isr_stub_234, isr_stub_235, isr_stub_236, isr_stub_237, isr_stub_238, isr_stub_239, isr_stub_240, isr_stub_241, isr_stub_242, isr_stub_243, isr_stub_244, isr_stub_245, isr_stub_246, isr_stub_247, isr_stub_248, isr_stub_249, isr_stub_250, isr_stub_251, isr_stub_252, isr_stub_253, isr_stub_254, isr_stub_255
};

void IDT_initialize(uint16_t segment, uint8_t IST){
    IDTR.size = sizeof(IDT)-1;
    IDTR.offset = (uint64_t)&IDT;

    for(int i=0;i<256;i++){
        IDT_set_entry(i, global_ctx->kernelImageStart + isr_table[i], 0x8E, segment, IST);
    }
}

//sets entry in the IDT table, uses global IDT
void IDT_set_entry(uint8_t vector, void* isr, uint8_t attrs, uint16_t segment, uint8_t IST){
    IDT_Entry* descriptor = &IDT[vector];

    descriptor->offset_low = (uint64_t)isr & 0xFFFF;
    descriptor->segment = segment;
    descriptor->ist = IST & 0b111;
    descriptor->attributes = attrs;
    descriptor->offset_mid = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->offset_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}

//IDT --------------------------


void kernel_main(KERNEL_CONTEXT_TABLE* ctx){

    //disable PIC because not useful in the big 2025
    PIC_disable();
    //swap the display buffers so the GOP framebuffer is actually the backbuffer
    {
        EFI_PHYSICAL_ADDRESS backbuf = ctx->fb;
        ctx->fb = ctx->GOP->FrameBufferBase; //set fb to vram
        ctx->GOP->FrameBufferBase = backbuf; //set the GOP address to backbuffer
    }

    //make ctx global
    global_ctx = ctx;

    //set GDT entries
    GDTR.size = sizeof(GDT)-1;
    GDTR.offset = GDT;

    GDT_set_entry(&GDT[0], 0, 0, 0, 0); //null descriptor right here le

    GDT_set_entry(&GDT[1], 0, 0, //Code segment, base and limit 0 because long mode
    0b10011010, //Access:present, ring 0, non system segment(code/data segment), executable(code segment), non conforming, readable, access
    0b1010 //granularity: page granularity(not byte), size flag(0 because long mode), long mode code, reserved
    );

    GDT_set_entry(&GDT[2], 0, 0, //Data segment, base and limit 0 because long mode
    0b10010010, //Access:present, ring 0, non system segment(code/data segment), non executable(data segment), up direction, writable, access
    0b1000 //granularity: page granularity(not byte), size flag(0 because long mode data), not long mode code, reserved
    );

    //load the GDT
    asm volatile(
        ".intel_syntax noprefix\n"
        "lgdt [%[gdt]]\n"
        "push 0x08\n"
        "lea rax, [rip+__long_jump_after_loading_gdt]\n"
        "push rax\n"
        "retfq\n"
        "__long_jump_after_loading_gdt:\n"
        "mov ax, 0x10\n"
        "mov ds, ax\n"
        "mov es, ax\n"
        "mov fs, ax\n"
        "mov gs, ax\n"
        "mov ss, ax\n"
        ".att_syntax\n"
        :
        : [gdt] "r"(&GDTR)
        : "memory", "rax"
    );
    printd("GDT loaded!\r\n");

    uint8_t CODE_SEG = 0x08;
    uint8_t DATA_SEG = 0x10;

    //todo: load idt
    IDT_initialize(CODE_SEG, 0);
    
    asm volatile(
        ".intel_syntax noprefix\n"
        "lidt [%[idt]]\n"
        "sti\n"
        ".att_syntax\n"
        :
        : [idt] "r"(&IDTR)
        : "memory"
    );

    

    printd("\r\nIDT loaded!\r\n");
    for(int i=0;i<400000000;i++);

    uint8_t versionMajor = 1;
    uint8_t versionMinor = 5;


    uint8_t CPUVendor[13];
    cpuid_get_vendor(CPUVendor);

    KERNEL_TEXT_OUTPUT title = {Terminus8x16_Bold, 8, 16, 2, 2, 0, 0, 20, 20, hex(0xFF10F0), hex(0x000000), true};
    KERNEL_TEXT_OUTPUT ConOut = {Terminus8x16_Normal, 8, 16, 1, 1, 0, 8, 0, 0, hex(0xFF10F0), hex(0x000000), false};

    /*
    for(int i=0;i<256;i++){
        printf(ctx->GOP, &ConOut, "stub %lx: %lx\r\n", &isr_stub_0, (uint64_t)((uint64_t)isr_stub_0)+i*(((uint64_t)isr_stub_1)-(uint64_t)isr_stub_0));
        memcpy((void*)ctx->GOP->FrameBufferBase, (void*)ctx->fb, ctx->GOP->FrameBufferSize);
    }
    while(1);
    */


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

    print_memory_map(ctx, &ConOut);

    while(true){
        //display
        title = (KERNEL_TEXT_OUTPUT){Terminus8x16_Bold, 8, 16, 2, 2, 0, 0, 20, 20, hex(0xFF10F0), hex(0x000000), true};
        ConOut = (KERNEL_TEXT_OUTPUT){Terminus8x16_Normal, 8, 16, 1, 1, 0, 8, 0, 0, hex(0xFF10F0), hex(0x000000), false};
        //clear screen
        GOPDrawRect(ctx->GOP, 0, 0, ctx->GOP->Info->HorizontalResolution-1, ctx->GOP->Info->VerticalResolution-1, rgba(0, 0, 0, 0), true);
        /*

        bool fill = true;
        uint32_t screenX = ctx->GOP->Info->HorizontalResolution - 1;
        uint32_t screenYFraction = ctx->GOP->Info->VerticalResolution / 5;
        GOPDrawRect(ctx->GOP, 0, 0, screenX, screenYFraction - 1, hex(0x55CDFC), fill);
        GOPDrawRect(ctx->GOP, 0, screenYFraction, screenX, 2*screenYFraction - 1, hex(0xF7A8B8), fill);
        GOPDrawRect(ctx->GOP, 0, 2*screenYFraction, screenX, 3*screenYFraction - 1, hex(0xFFFFFF), fill);
        GOPDrawRect(ctx->GOP, 0, 3*screenYFraction, screenX, 4*screenYFraction - 1, hex(0xF7A8B8), fill);
        GOPDrawRect(ctx->GOP, 0, 4*screenYFraction, screenX, 5*screenYFraction - 1, hex(0x55CDFC), fill);
        */
        GOPDrawRect(ctx->GOP, 0, 0, ctx->GOP->Info->HorizontalResolution-1, ctx->GOP->Info->VerticalResolution-1, hex(0x00807F), true);

        GOPPlayVideo(ctx->GOP, ctx->GOP->Info->HorizontalResolution - bad_apple.width, 0, &bad_apple, true);
        
        printf(ctx->GOP, &ConOut, "(operating system of the future)\r\n");
        printf(ctx->GOP, &ConOut, "Display pixel format: %d\r\n", ctx->GOP->Info->PixelFormat);
        printf(ctx->GOP, &ConOut, "CPU Vendor: %s\r\n", CODE_SEG, DATA_SEG, &CPUVendor);
        printf(ctx->GOP, &ConOut, "Video resolution: %dx%d / format %d / frame %d/%d\r\n", bad_apple.width, bad_apple.height, bad_apple.format, bad_apple.frameCounter+1, bad_apple.frameCount);

        printf(ctx->GOP, &title, "N");
        title.frontColor = 0xFF8D00;title.backColor = 0x000000;
        printf(ctx->GOP, &title, "u");
        title.frontColor = 0xFFEE00;title.backColor = 0x000000;
        printf(ctx->GOP, &title, "c");
        title.frontColor = 0x028121;title.backColor = 0x000000;
        printf(ctx->GOP, &title, "k");
        title.frontColor = 0xFF10F0;title.backColor = 0x000000;
        printf(ctx->GOP, &title, " ");
        title.frontColor = 0x004CFF;title.backColor = 0x000000;
        printf(ctx->GOP, &title, "O");
        title.frontColor = 0x770088;title.backColor = 0x000000;
        printf(ctx->GOP, &title, "S");
        title.frontColor = 0xFF10F0;title.backColor = 0x000000;

        printf(ctx->GOP, &title, "\r\n Version %u.%u!\r\n", versionMajor, versionMinor);

        /*
        heap_display(ctx->heap, ctx->GOP, &HeapOut);
        printf(ctx->GOP, &ConOut, "\r\nheap page allocator: \r\n%lx, +32768 pages, %ld MB\r\n", ctx->heap->heap, 32768*4*1024/1024/1024);
        printf(ctx->GOP, &ConOut, "\r\n\nsubpage allocator: \r\n%lx to %lx\r\n", alloc.freeListStart, alloc.freeListEnd);
        printf(ctx->GOP, &ConOut, "1st subpage: %lx\r\n", subPtr);
        printf(ctx->GOP, &ConOut, "2nd subpage: %lx\r\n", subPtr2);
        */

        viewConfigTables(ctx->GOP, &ConOut, ctx->ConfigTable, ctx->ConfigTableEntriesCount);

        printf(ctx->GOP, &ConOut, "size of stuff: %u + %u\r\n", sizeof(*ctx), ctx->MemoryMapSizeBytes);

        //logo
        GOPDrawImage(ctx->GOP, ctx->GOP->Info->HorizontalResolution - nuckos_logo.width - 10, ctx->GOP->Info->VerticalResolution - nuckos_logo.height - 10, &nuckos_logo);

        //PS/2 input
        /*
        uint8_t is_mouse = PS2_poll(&scancode, &dx, &dy, &lrm);
        
        if(!is_mouse){ //keyboard
            if(scancode & 0x80){
                printf(ctx->GOP, &ConOut, "BREAK\r\n");
            }
            else{
                printf(ctx->GOP, &ConOut, "MAKE\r\n");
            }
            printf(ctx->GOP, &ConOut, ":%x\r\n", scancode);
        }
        else{ //mouse
            if(mouseInitError){
                printf(ctx->GOP, &ConOut, "mouse init error\r\n");
            }
            else if(mouseSetSampleError){
                printf(ctx->GOP, &ConOut, "mouse set sample rate error\r\n");
            }
            else{
                printf(ctx->GOP, &ConOut, "mouse input: %d, %d, %x\r\n", dx, dy, lrm);
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
        GOPDrawImage(ctx->GOP, pointerX, pointerY, &pointer_icon);

        //copy framebuffer
        memcpy((void*)ctx->GOP->FrameBufferBase, (void*)ctx->fb, ctx->GOP->FrameBufferSize);
        
        //do this to test interrupts i guess
        break;
    }

    GOPDrawRect(ctx->GOP, 0, 0, ctx->GOP->Info->HorizontalResolution-1, ctx->GOP->Info->VerticalResolution-1, hex(0x20207F), true);
    printf(ctx->GOP, &title, "interrupting...\r\n");
    memcpy((void*)ctx->GOP->FrameBufferBase, (void*)ctx->fb, ctx->GOP->FrameBufferSize);
    
    asm volatile(
        ".intel_syntax noprefix\n"
        "int 6\n"
        "int 7\n"
        "int 67\n"
        ".att_syntax\n"
    );


    //printf(ctx->GOP, &title, "Done!\r\n");
    memcpy((void*)ctx->GOP->FrameBufferBase, (void*)ctx->fb, ctx->GOP->FrameBufferSize);
    while(true);
}























































































//NEW physical memory manager related functions
void print_memory_map(KERNEL_CONTEXT_TABLE* ctx, KERNEL_TEXT_OUTPUT* Con){
    uint64_t memory_map_size = ctx->MemoryMapSizeBytes;
    uint64_t memory_map_size_pages = ctx->MemoryMapSizePages;
    uint64_t memory_map_descriptor_size = ctx->MemoryMapDescriptorSize;

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
    EFI_MEMORY_DESCRIPTOR* MM = ctx->MemoryMap;
    
    //size of conventional memory in number of 4 KiB pages
    uint64_t totalMapped = 0;
    uint64_t totalUsable = 0;

    uint32_t oldColor = Con->frontColor;
    uint32_t oldBackColor = Con->backColor;

    //print other info
    printf(ctx->GOP, Con, "Memory Map Size: %lu\r\nSize of each entry: %lu\r\nTotal entries: %lu\r\n", memory_map_size, memory_map_descriptor_size, entries);
    for(uint32_t i = 0;i < entries;i++){
        printf(ctx->GOP, Con, "#%u - ", i+1);
        
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
            printf(ctx->GOP, Con, " ");
            //reset color
            Con->frontColor = oldColor;
            Con->backColor = oldBackColor;
            printf(ctx->GOP, Con, "%s ", type_arr[MM->Type]);
        }
        else{
            Con->frontColor = hex(0xFFFF00);
            Con->backColor = hex(0xFFFF00); //yellow
            printf(ctx->GOP, Con, " ");
            //reset color
            Con->frontColor = oldColor;
            Con->backColor = oldBackColor;
            printf(ctx->GOP, Con, "0x%x ", MM->Type);
        }
        printf(ctx->GOP, Con, " ");

        printf(ctx->GOP, Con, "Range:0x%lx - 0x%lx ", MM->PhysicalStart, (MM->PhysicalStart + (MM->NumberOfPages*4096) - 1));
        if(MM->Attribute & 0x1)printf(ctx->GOP, Con, "UC ");
        if(MM->Attribute & 0x2)printf(ctx->GOP, Con, "WC ");
        if(MM->Attribute & 0x4)printf(ctx->GOP, Con, "WT ");
        if(MM->Attribute & 0x8)printf(ctx->GOP, Con, "WB ");
        if(MM->Attribute & 0x10)printf(ctx->GOP, Con, "UCE ");
        if(MM->Attribute & 0x1000)printf(ctx->GOP, Con, "WP ");
        if(MM->Attribute & 0x2000)printf(ctx->GOP, Con, "RP ");
        if(MM->Attribute & 0x4000)printf(ctx->GOP, Con, "XP ");
        if(MM->Attribute & 0x8000)printf(ctx->GOP, Con, "NV ");
        if(MM->Attribute & 0x10000)printf(ctx->GOP, Con, "MORE_RELIABLE ");
        if(MM->Attribute & 0x20000)printf(ctx->GOP, Con, "RO ");
        if(MM->Attribute & 0x40000)printf(ctx->GOP, Con, "SP ");
        if(MM->Attribute & 0x80000)printf(ctx->GOP, Con, "CRYPTO ");
        if(MM->Attribute & 0x8000000000000000)printf(ctx->GOP, Con, "RUNTIME ");
        if(MM->Attribute & 0x4000000000000000)printf(ctx->GOP, Con, "ISA_VALID ");
        if(MM->Attribute & 0x0FFFF00000000000)printf(ctx->GOP, Con, "ISA_MASK ");

        printf(ctx->GOP, Con, "\r\n");
        //go to next one
        MM = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)MM + memory_map_descriptor_size);
        memcpy((void*)ctx->GOP->FrameBufferBase, (void*)ctx->fb, ctx->GOP->FrameBufferSize);
        //break here
        for(uint32_t i=0;i<400000;i++);
    }
    printf(ctx->GOP, Con, "Total mapped memory: %d pages/%f GB/%f GiB\r\n", totalMapped, totalMapped/250000.0f, totalMapped/262144.0f);
    printf(ctx->GOP, Con, "Total usable memory: %d pages/%f GB/%f GiB\r\n", totalUsable, totalUsable/250000.0f, totalUsable/262144.0f);
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
void viewConfigTables(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* con, EFI_CONFIGURATION_TABLE* tablePtr, uint64_t entries){

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

    printf(GOP, con, "Number of configuration table entries: %u\r\n", entries);
    for(uint64_t entry = 0;entry < entries;entry++){
        table = tablePtr[entry];
        printf(GOP, con, "Table #%u:", entry+1);
        printGUID(GOP, con, &table.VendorGuid);
        for(uint32_t guidIndex = 0;guidIndex < (sizeof(GUIDTableKeys)/sizeof(GUIDTableKeys[0]));guidIndex++){
            if(cmpGUID(&table.VendorGuid, &GUIDTableKeys[guidIndex])){
                printf(GOP, con, "  %s", GUIDTableValues[guidIndex]);
                break;
            }
        }
        printf(GOP, con, "  ptr: 0x%lx\r\n", tablePtr->VendorTable);
    }
    printf(GOP, con, "----------END----------\r\n");
}

void printGUID(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* con, EFI_GUID* guid){
    printf(GOP, con, "GUID: %x-%x-%x-", guid->Data1, guid->Data2, guid->Data3);
    printf(GOP, con, "%x-%x-%x-%x-%x-%x-%x-%x",
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


//GDT/IDT functions, general functions
void GDT_set_entry(GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    entry->limit_low = (uint16_t)(limit & 0xFFFF);
    entry->base_low = (uint16_t)(base & 0xFFFF);
    entry->base_mid = (uint8_t)((base >> 16) & 0xFF);
    entry->access = access;
    entry->limit__flags = (uint8_t)(((limit >> 16) & 0xF) | (flags << 4));
    entry->base_high = (uint8_t)(base >> 24);
}
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
void GOPPlayVideo(EFI_GOP* GOP, uint32_t x, uint32_t y, KERNEL_NVIDEO* video, bool loop){
    if(video->frameCounter >= video->frameCount){
        return;
    }
    uint8_t* addr = (uint8_t*)(video->addr);
    switch(video->format){
        case 0: { //black and white, packed
            video->addr = video->addr + ((video->frameCounter) * (video->width * video->height / 8));
            GOPDrawImage(GOP, x, y, video);
            video->addr = addr;
            break;
        }
    }
    (video->frameCounter)++;
    if(video->frameCounter >= video->frameCount && loop){
        video->frameCounter = 0;
    }
}
void GOPDrawImage(EFI_GOP* GOP, uint32_t x, uint32_t y, KERNEL_NVIDEO* img){
    switch(img->format){
        case 0: { //black and white, packed, imgwidth is number of pixels, imgheight is number of pixels
            uint32_t bpr = img->width/8; //bytes per row
            for(uint32_t row = 0;row < img->height;row++){
                for(uint32_t byte = 0;byte < bpr;byte++){
                    //calculate byte
                    uint8_t b = img->addr[row * bpr + byte];
                    uint32_t draw_x = x + byte * 8;
                    uint32_t draw_y = y + row;
                    //draw 8 pixels
                    for(int8_t shift = 7;shift >= 0;shift--){
                        uint32_t mask = 1 << shift;
                        uint32_t color = (b & mask) ? 0xFFFFFFFF : 0xFF000000;
                        GOPPutPixel(GOP, draw_x + (7-shift), draw_y, color);
                    }
                }
            }
            break;
        }
        case 1: { //RGB 3 byte per pixel
            uint32_t bpr = img->width * 3; //bytes per row
            for(uint32_t row = 0;row < img->height;row++){
                for(uint32_t col = 0;col < img->width;col++){
                    //row, col is pixel position
                    //get pixel byte position
                    uint32_t color_byte = *(uint32_t*)(img->addr + row * bpr + col * 3);
                    uint32_t draw_x = x + col;
                    uint32_t draw_y = y + row;
                    GOPPutPixel(GOP, draw_x, draw_y, hex(color_byte)); //color converted from RGB to ARGB
                }
            }
            break;
        }
        case 2: { //ARGB 4 byte per pixel, alpha 0 = transparent
            uint32_t bpr = img->width * 4; //bytes per row
            for(uint32_t row = 0;row < img->height;row++){
                for(uint32_t col = 0;col < img->width;col++){
                    //row, col is pixel position
                    //get pixel byte position
                    uint32_t color_byte = *(uint32_t*)(img->addr + row * bpr + col * 4);
                    uint32_t draw_x = x + col;
                    uint32_t draw_y = y + row;
                    if(color_byte & 0xFF000000){
                        GOPPutPixel(GOP, draw_x, draw_y, color_byte); //color is ARGB
                    }
                }
            }
            break;
        }
    }
}
void printf(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, char* str, ...){
    va_list args;
    va_start(args, str);

    bool longType = false;
    while (*str) {
        if(*str != '%'){ //if not a format specifier
            printChar(GOP, ConOut, *str++);
            continue;
        }
        str++; //skip the '%'

        if(*str == 0){
            break;
        }
        if(*str == '%'){
            printChar(GOP, ConOut, '%');
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
                printChar(GOP, ConOut, (uint8_t)va_arg(args, int32_t));
                break;
            case 's': //string
                printString(GOP, ConOut, (char*)va_arg(args, uint8_t*));
                break;

            case 'd':
            case 'i': //signed int
                if(longType){
                    printInt(GOP, ConOut, (int64_t)va_arg(args, int64_t), 10);
                }
                else{
                    printInt(GOP, ConOut, (int64_t)va_arg(args, int32_t), 10); 
                }
                break;
            
            case 'u': //unsigned int
                if(longType){
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint64_t), 10);
                }
                else{
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint32_t), 10);
                }
                break;

            case 'o': //octal
                if(longType){
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint64_t), 8);
                }
                else{
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint32_t), 8);
                }
                break;
            case 'x': //hex
            case 'X': //hex
                if(longType){
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint64_t), 16);
                }
                else{
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint32_t), 16);
                }
                break;

            case 'p': //pointer
                printUint(GOP, ConOut, (uint64_t)va_arg(args, void*), 16);
                break;
            
            case 'f': //float
                if(longType){
                    printFloat(GOP, ConOut, (double) va_arg(args, double), 16); 
                }
                else{
                    printFloat(GOP, ConOut, (double) va_arg(args, double), 6);
                }
                break;
            case 'l':
                printChar(GOP, ConOut, 'l');
                break;
            case 'n': //nothing
                break;
            default:
                //nuh uh, print the character itself
                printChar(GOP, ConOut, '%');
                printChar(GOP, ConOut, format);
                str--;
                break;
        }
        str++;
    }
    va_end(args);
}
void printFloat(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec){
    if(num < 0.0){
        printChar(GOP, ConOut, '-');
        num = -num;
    }
    printUfloat(GOP, ConOut, num, prec);
}
void printUfloat(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec){
    printUint(GOP, ConOut, (uint64_t)num, 10); //print integer part
    printChar(GOP, ConOut, '.');
    num -= (double)(uint64_t)num;
    uint8_t digit;
    for(uint8_t c = 0;c < prec;c++){
        num *= 10.0;
        digit = (uint8_t)num;
        printChar(GOP, ConOut, digit + '0');
        num -= digit;
    }
}
void printInt(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, int64_t num, uint8_t base){
    if(num < 0){
        printChar(GOP, ConOut, '-');
        printUint(GOP, ConOut, (uint64_t)(-num), base);
        return;
    }
    printUint(GOP, ConOut, num, base);
}
void printUint(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint64_t num, uint8_t base){
    if(base < 2 || base > 16)return;

    if(base == 2)printString(GOP, ConOut, "0b");
    else if(base == 8)printString(GOP, ConOut, "0o");
    else if(base == 16)printString(GOP, ConOut, "0x");

    if(num == 0){
        printChar(GOP, ConOut, '0');
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
    printString(GOP, ConOut, (char*)&buff[index]);
}
void printString(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, char* string){
    while(*string){ //while it's not null
        printChar(GOP, ConOut, *string++);
    }
}
void printChar(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, char ascii_char){
    uint32_t maxWidth = GOP->Info->HorizontalResolution / (ConOut->charWidth*ConOut->scaleX);
    uint32_t maxHeight = GOP->Info->VerticalResolution / (ConOut->charHeight*ConOut->scaleY);
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
                    GOPPutPixel(GOP, screenX+(dx*ConOut->scaleX)+scaleXOff, screenY+(dy*ConOut->scaleY)+scaleYOff, color);
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
            printChar(GOP, ConOut, *str++);
            continue;
        }
        str++; //skip the '%'

        if(*str == 0){
            break;
        }
        if(*str == '%'){
            printChar(GOP, ConOut, '%');
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
                printChar(GOP, ConOut, (uint8_t)va_arg(args, int32_t));
                break;
            case 's': //string
                printString(GOP, ConOut, (char*)va_arg(args, uint8_t*));
                break;

            case 'd':
            case 'i': //signed int
                if(longType){
                    printInt(GOP, ConOut, (int64_t)va_arg(args, int64_t), 10);
                }
                else{
                    printInt(GOP, ConOut, (int64_t)va_arg(args, int32_t), 10); 
                }
                break;
            
            case 'u': //unsigned int
                if(longType){
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint64_t), 10);
                }
                else{
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint32_t), 10);
                }
                break;

            case 'o': //octal
                if(longType){
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint64_t), 8);
                }
                else{
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint32_t), 8);
                }
                break;
            case 'x': //hex
            case 'X': //hex
                if(longType){
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint64_t), 16);
                }
                else{
                    printUint(GOP, ConOut, (uint64_t) va_arg(args, uint32_t), 16);
                }
                break;

            case 'p': //pointer
                printUint(GOP, ConOut, (uint64_t)va_arg(args, void*), 16);
                break;
            
            case 'f': //float
                if(longType){
                    printFloat(GOP, ConOut, (double) va_arg(args, double), 16); 
                }
                else{
                    printFloat(GOP, ConOut, (double) va_arg(args, double), 6);
                }
                break;
            case 'l':
                printChar(GOP, ConOut, 'l');
                break;
            case 'n': //nothing
                break;
            default:
                //nuh uh, print the character itself
                printChar(GOP, ConOut, '%');
                printChar(GOP, ConOut, format);
                str--;
                break;
        }
        str++;
    }
    va_end(args);
    memcpy((void*)GOP->FrameBufferBase, (void*)global_ctx->fb, GOP->FrameBufferSize);    
}

//general functions
void* memcpy(void* source, void* dest, uint64_t size){
    uint8_t* d = (uint8_t*) dest;
    uint8_t* s = (uint8_t*) source;
    while (size >= 64 &&
          ((uintptr_t)d % 64 == 0) &&
          ((uintptr_t)s % 64 == 0)
          ){
        *(uint64_t*)d = *(uint64_t*)s;
        d += 64;
        s += 64;
        size -= 64;
    }
    // Copy remaining bytes one by one
    while (size--) {
        *d++ = *s++;
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
