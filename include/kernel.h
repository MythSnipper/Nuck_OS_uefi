#ifndef KERNEL_H
#define KERNEL_H

#include "../include/nuckdef.h"
#include <efi.h>
#include <efilib.h>




typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t segment;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} IDT_Entry;

typedef struct __attribute__((packed)) {
    uint16_t size; //idt size - 1
    uint64_t offset; //idt start
} IDT_Descriptor;

typedef struct __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  limit__flags;
    uint8_t  base_high;
} GDT_Entry;

typedef struct __attribute__((packed)) {
    uint16_t size; //gdt size - 1
    uint64_t offset; //gdt start
} GDT_Descriptor;

typedef struct{
    uint8_t* font;
    uint32_t charWidth;
    uint32_t charHeight;
    uint32_t scaleX;
    uint32_t scaleY;
    uint32_t cursorX;
    uint32_t cursorY;
    uint32_t offsetX;
    uint32_t offsetY;
    uint32_t frontColor;
    uint32_t backColor;
    uint8_t useAbsolutePosition;
} KERNEL_TEXT_OUTPUT;

typedef struct{
    uint8_t*                           map;
    uint8_t*                           heap;
} KERNEL_HEAP;

typedef struct{
    CHAR16*                            FirmwareVendor;
    uint32_t                           FirmwareRevision;
    EFI_RUNTIME_SERVICES*              RuntimeServices;
    EFI_MEMORY_DESCRIPTOR*             MemoryMap;
    uint64_t                           MemoryMapSize;
    uint64_t                           MemoryMapDescriptorSize;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* GOP;
    EFI_PHYSICAL_ADDRESS               fb; //backbuffer in bootloader, frontbuffer in kernel
    EFI_PHYSICAL_ADDRESS               kernelStack;
    uint64_t                           kernelStackSize;
    KERNEL_HEAP*                       heap;
    EFI_PHYSICAL_ADDRESS               file;
} KERNEL_CONTEXT_TABLE;

typedef EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE EFI_GOP;

void PIC_sendEOI(uint8_t irq);
void PIC_remap(uint8_t offset1, uint8_t offset2);
void PIC_disable();
void IRQ_set_mask(uint8_t IRQline);
void IRQ_clear_mask(uint8_t IRQline);
static uint16_t __pic_get_irq_reg(int ocw3);
uint16_t pic_get_irr();
uint16_t pic_get_isr();

void setIDTEntry(IDT_Entry* entry, uint16_t segment, uint64_t offset, uint8_t ISTOffset, uint8_t attributes);
void setGDTEntry(GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void triple_fault();


//dynamic memory allocation functions
void heap_init(KERNEL_HEAP* heap);
void* heap_alloc(KERNEL_HEAP* heap, uint32_t pages);
void heap_free(KERNEL_HEAP* heap, void* addr, uint32_t pages);
void heap_display(KERNEL_HEAP* heap, EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut);


//graphical functions
void playVideo(EFI_GOP* GOP, uint32_t x, uint32_t y, uint32_t format, uint8_t* addr, uint32_t frameWidth, uint32_t frameHeight, uint32_t frameCount, uint32_t* frameCounter, bool loop, uint8_t skips);
void GOPDrawImage(EFI_GOP* GOP, uint32_t x, uint32_t y, uint32_t imgwidth, uint32_t imgheight, uint8_t* imgaddr, uint32_t format);
void printf(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t* str, ...);
void printFloat(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec);
void printUfloat(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec);
void printInt(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, int64_t num, uint8_t base);
void printUint(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint64_t num, uint8_t base);
void printString(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t* string);
void printChar(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t ascii_char);
void GOPDrawRect(EFI_GOP* GOP, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color, uint8_t fill);
void GOPPutPixel(EFI_GOP* GOP, uint32_t x, uint32_t y, uint32_t color);

void* memcpy(void* source, void* dest, uint64_t size);
static inline uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
static inline uint32_t hex(uint32_t hex);
static inline void outb(uint16_t port, uint8_t value);
static inline void outw(uint16_t port, uint16_t value);
static inline void outl(uint16_t port, uint32_t value);
static inline uint8_t inb(uint16_t port);
static inline uint16_t inw(uint16_t port);
static inline uint32_t inl(uint16_t port);
static inline void io_wait();
static inline void cpuid(int code, uint32_t* a, uint32_t* d);
static inline int cpuid_string(int code, uint32_t where[4]);
uint64_t rdtsc();




void isr_0();
void isr_1();
void isr_2();
void isr_3();
void isr_4();
void isr_5();
void isr_6();
void isr_7();
void isr_8();
void isr_9();
void isr_10();
void isr_11();
void isr_12();
void isr_13();
void isr_14();
void isr_15();
void isr_16();
void isr_17();
void isr_18();
void isr_19();
void isr_20();
void isr_21();
void isr_22();
void isr_23();
void isr_24();
void isr_25();
void isr_26();
void isr_27();
void isr_28();
void isr_29();
void isr_30();
void isr_31();
void isr_32();
void isr_33();
void isr_34();
void isr_35();
void isr_36();
void isr_37();
void isr_38();
void isr_39();
void isr_40();
void isr_41();
void isr_42();
void isr_43();
void isr_44();
void isr_45();
void isr_46();
void isr_47();
void isr_48();
void isr_49();
void isr_50();
void isr_51();
void isr_52();
void isr_53();
void isr_54();
void isr_55();
void isr_56();
void isr_57();
void isr_58();
void isr_59();
void isr_60();
void isr_61();
void isr_62();
void isr_63();
void isr_64();
void isr_65();
void isr_66();
void isr_67();
void isr_68();
void isr_69();
void isr_70();
void isr_71();
void isr_72();
void isr_73();
void isr_74();
void isr_75();
void isr_76();
void isr_77();
void isr_78();
void isr_79();
void isr_80();
void isr_81();
void isr_82();
void isr_83();
void isr_84();
void isr_85();
void isr_86();
void isr_87();
void isr_88();
void isr_89();
void isr_90();
void isr_91();
void isr_92();
void isr_93();
void isr_94();
void isr_95();
void isr_96();
void isr_97();
void isr_98();
void isr_99();
void isr_100();
void isr_101();
void isr_102();
void isr_103();
void isr_104();
void isr_105();
void isr_106();
void isr_107();
void isr_108();
void isr_109();
void isr_110();
void isr_111();
void isr_112();
void isr_113();
void isr_114();
void isr_115();
void isr_116();
void isr_117();
void isr_118();
void isr_119();
void isr_120();
void isr_121();
void isr_122();
void isr_123();
void isr_124();
void isr_125();
void isr_126();
void isr_127();
void isr_128();
void isr_129();
void isr_130();
void isr_131();
void isr_132();
void isr_133();
void isr_134();
void isr_135();
void isr_136();
void isr_137();
void isr_138();
void isr_139();
void isr_140();
void isr_141();
void isr_142();
void isr_143();
void isr_144();
void isr_145();
void isr_146();
void isr_147();
void isr_148();
void isr_149();
void isr_150();
void isr_151();
void isr_152();
void isr_153();
void isr_154();
void isr_155();
void isr_156();
void isr_157();
void isr_158();
void isr_159();
void isr_160();
void isr_161();
void isr_162();
void isr_163();
void isr_164();
void isr_165();
void isr_166();
void isr_167();
void isr_168();
void isr_169();
void isr_170();
void isr_171();
void isr_172();
void isr_173();
void isr_174();
void isr_175();
void isr_176();
void isr_177();
void isr_178();
void isr_179();
void isr_180();
void isr_181();
void isr_182();
void isr_183();
void isr_184();
void isr_185();
void isr_186();
void isr_187();
void isr_188();
void isr_189();
void isr_190();
void isr_191();
void isr_192();
void isr_193();
void isr_194();
void isr_195();
void isr_196();
void isr_197();
void isr_198();
void isr_199();
void isr_200();
void isr_201();
void isr_202();
void isr_203();
void isr_204();
void isr_205();
void isr_206();
void isr_207();
void isr_208();
void isr_209();
void isr_210();
void isr_211();
void isr_212();
void isr_213();
void isr_214();
void isr_215();
void isr_216();
void isr_217();
void isr_218();
void isr_219();
void isr_220();
void isr_221();
void isr_222();
void isr_223();
void isr_224();
void isr_225();
void isr_226();
void isr_227();
void isr_228();
void isr_229();
void isr_230();
void isr_231();
void isr_232();
void isr_233();
void isr_234();
void isr_235();
void isr_236();
void isr_237();
void isr_238();
void isr_239();
void isr_240();
void isr_241();
void isr_242();
void isr_243();
void isr_244();
void isr_245();
void isr_246();
void isr_247();
void isr_248();
void isr_249();
void isr_250();
void isr_251();
void isr_252();
void isr_253();
void isr_254();
void isr_255();





#endif
