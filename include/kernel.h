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


void printFloat(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec);
void printUfloat(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, double num, uint8_t prec);
void printInt(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, int64_t num, uint8_t base);
void printUint(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint64_t num, uint8_t base);
void printf(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t* str, ...);
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




__attribute__((noreturn)) void isr_0();
__attribute__((noreturn)) void isr_1();
__attribute__((noreturn)) void isr_2();
__attribute__((noreturn)) void isr_3();
__attribute__((noreturn)) void isr_4();
__attribute__((noreturn)) void isr_5();
__attribute__((noreturn)) void isr_6();
__attribute__((noreturn)) void isr_7();
__attribute__((noreturn)) void isr_8();
__attribute__((noreturn)) void isr_9();
__attribute__((noreturn)) void isr_10();
__attribute__((noreturn)) void isr_11();
__attribute__((noreturn)) void isr_12();
__attribute__((noreturn)) void isr_13();
__attribute__((noreturn)) void isr_14();
__attribute__((noreturn)) void isr_15();
__attribute__((noreturn)) void isr_16();
__attribute__((noreturn)) void isr_17();
__attribute__((noreturn)) void isr_18();
__attribute__((noreturn)) void isr_19();
__attribute__((noreturn)) void isr_20();
__attribute__((noreturn)) void isr_21();
__attribute__((noreturn)) void isr_22();
__attribute__((noreturn)) void isr_23();
__attribute__((noreturn)) void isr_24();
__attribute__((noreturn)) void isr_25();
__attribute__((noreturn)) void isr_26();
__attribute__((noreturn)) void isr_27();
__attribute__((noreturn)) void isr_28();
__attribute__((noreturn)) void isr_29();
__attribute__((noreturn)) void isr_30();
__attribute__((noreturn)) void isr_31();
__attribute__((noreturn)) void isr_32();
__attribute__((noreturn)) void isr_33();
__attribute__((noreturn)) void isr_34();
__attribute__((noreturn)) void isr_35();
__attribute__((noreturn)) void isr_36();
__attribute__((noreturn)) void isr_37();
__attribute__((noreturn)) void isr_38();
__attribute__((noreturn)) void isr_39();
__attribute__((noreturn)) void isr_40();
__attribute__((noreturn)) void isr_41();
__attribute__((noreturn)) void isr_42();
__attribute__((noreturn)) void isr_43();
__attribute__((noreturn)) void isr_44();
__attribute__((noreturn)) void isr_45();
__attribute__((noreturn)) void isr_46();
__attribute__((noreturn)) void isr_47();
__attribute__((noreturn)) void isr_48();
__attribute__((noreturn)) void isr_49();
__attribute__((noreturn)) void isr_50();
__attribute__((noreturn)) void isr_51();
__attribute__((noreturn)) void isr_52();
__attribute__((noreturn)) void isr_53();
__attribute__((noreturn)) void isr_54();
__attribute__((noreturn)) void isr_55();
__attribute__((noreturn)) void isr_56();
__attribute__((noreturn)) void isr_57();
__attribute__((noreturn)) void isr_58();
__attribute__((noreturn)) void isr_59();
__attribute__((noreturn)) void isr_60();
__attribute__((noreturn)) void isr_61();
__attribute__((noreturn)) void isr_62();
__attribute__((noreturn)) void isr_63();
__attribute__((noreturn)) void isr_64();
__attribute__((noreturn)) void isr_65();
__attribute__((noreturn)) void isr_66();
__attribute__((noreturn)) void isr_67();
__attribute__((noreturn)) void isr_68();
__attribute__((noreturn)) void isr_69();
__attribute__((noreturn)) void isr_70();
__attribute__((noreturn)) void isr_71();
__attribute__((noreturn)) void isr_72();
__attribute__((noreturn)) void isr_73();
__attribute__((noreturn)) void isr_74();
__attribute__((noreturn)) void isr_75();
__attribute__((noreturn)) void isr_76();
__attribute__((noreturn)) void isr_77();
__attribute__((noreturn)) void isr_78();
__attribute__((noreturn)) void isr_79();
__attribute__((noreturn)) void isr_80();
__attribute__((noreturn)) void isr_81();
__attribute__((noreturn)) void isr_82();
__attribute__((noreturn)) void isr_83();
__attribute__((noreturn)) void isr_84();
__attribute__((noreturn)) void isr_85();
__attribute__((noreturn)) void isr_86();
__attribute__((noreturn)) void isr_87();
__attribute__((noreturn)) void isr_88();
__attribute__((noreturn)) void isr_89();
__attribute__((noreturn)) void isr_90();
__attribute__((noreturn)) void isr_91();
__attribute__((noreturn)) void isr_92();
__attribute__((noreturn)) void isr_93();
__attribute__((noreturn)) void isr_94();
__attribute__((noreturn)) void isr_95();
__attribute__((noreturn)) void isr_96();
__attribute__((noreturn)) void isr_97();
__attribute__((noreturn)) void isr_98();
__attribute__((noreturn)) void isr_99();
__attribute__((noreturn)) void isr_100();
__attribute__((noreturn)) void isr_101();
__attribute__((noreturn)) void isr_102();
__attribute__((noreturn)) void isr_103();
__attribute__((noreturn)) void isr_104();
__attribute__((noreturn)) void isr_105();
__attribute__((noreturn)) void isr_106();
__attribute__((noreturn)) void isr_107();
__attribute__((noreturn)) void isr_108();
__attribute__((noreturn)) void isr_109();
__attribute__((noreturn)) void isr_110();
__attribute__((noreturn)) void isr_111();
__attribute__((noreturn)) void isr_112();
__attribute__((noreturn)) void isr_113();
__attribute__((noreturn)) void isr_114();
__attribute__((noreturn)) void isr_115();
__attribute__((noreturn)) void isr_116();
__attribute__((noreturn)) void isr_117();
__attribute__((noreturn)) void isr_118();
__attribute__((noreturn)) void isr_119();
__attribute__((noreturn)) void isr_120();
__attribute__((noreturn)) void isr_121();
__attribute__((noreturn)) void isr_122();
__attribute__((noreturn)) void isr_123();
__attribute__((noreturn)) void isr_124();
__attribute__((noreturn)) void isr_125();
__attribute__((noreturn)) void isr_126();
__attribute__((noreturn)) void isr_127();
__attribute__((noreturn)) void isr_128();
__attribute__((noreturn)) void isr_129();
__attribute__((noreturn)) void isr_130();
__attribute__((noreturn)) void isr_131();
__attribute__((noreturn)) void isr_132();
__attribute__((noreturn)) void isr_133();
__attribute__((noreturn)) void isr_134();
__attribute__((noreturn)) void isr_135();
__attribute__((noreturn)) void isr_136();
__attribute__((noreturn)) void isr_137();
__attribute__((noreturn)) void isr_138();
__attribute__((noreturn)) void isr_139();
__attribute__((noreturn)) void isr_140();
__attribute__((noreturn)) void isr_141();
__attribute__((noreturn)) void isr_142();
__attribute__((noreturn)) void isr_143();
__attribute__((noreturn)) void isr_144();
__attribute__((noreturn)) void isr_145();
__attribute__((noreturn)) void isr_146();
__attribute__((noreturn)) void isr_147();
__attribute__((noreturn)) void isr_148();
__attribute__((noreturn)) void isr_149();
__attribute__((noreturn)) void isr_150();
__attribute__((noreturn)) void isr_151();
__attribute__((noreturn)) void isr_152();
__attribute__((noreturn)) void isr_153();
__attribute__((noreturn)) void isr_154();
__attribute__((noreturn)) void isr_155();
__attribute__((noreturn)) void isr_156();
__attribute__((noreturn)) void isr_157();
__attribute__((noreturn)) void isr_158();
__attribute__((noreturn)) void isr_159();
__attribute__((noreturn)) void isr_160();
__attribute__((noreturn)) void isr_161();
__attribute__((noreturn)) void isr_162();
__attribute__((noreturn)) void isr_163();
__attribute__((noreturn)) void isr_164();
__attribute__((noreturn)) void isr_165();
__attribute__((noreturn)) void isr_166();
__attribute__((noreturn)) void isr_167();
__attribute__((noreturn)) void isr_168();
__attribute__((noreturn)) void isr_169();
__attribute__((noreturn)) void isr_170();
__attribute__((noreturn)) void isr_171();
__attribute__((noreturn)) void isr_172();
__attribute__((noreturn)) void isr_173();
__attribute__((noreturn)) void isr_174();
__attribute__((noreturn)) void isr_175();
__attribute__((noreturn)) void isr_176();
__attribute__((noreturn)) void isr_177();
__attribute__((noreturn)) void isr_178();
__attribute__((noreturn)) void isr_179();
__attribute__((noreturn)) void isr_180();
__attribute__((noreturn)) void isr_181();
__attribute__((noreturn)) void isr_182();
__attribute__((noreturn)) void isr_183();
__attribute__((noreturn)) void isr_184();
__attribute__((noreturn)) void isr_185();
__attribute__((noreturn)) void isr_186();
__attribute__((noreturn)) void isr_187();
__attribute__((noreturn)) void isr_188();
__attribute__((noreturn)) void isr_189();
__attribute__((noreturn)) void isr_190();
__attribute__((noreturn)) void isr_191();
__attribute__((noreturn)) void isr_192();
__attribute__((noreturn)) void isr_193();
__attribute__((noreturn)) void isr_194();
__attribute__((noreturn)) void isr_195();
__attribute__((noreturn)) void isr_196();
__attribute__((noreturn)) void isr_197();
__attribute__((noreturn)) void isr_198();
__attribute__((noreturn)) void isr_199();
__attribute__((noreturn)) void isr_200();
__attribute__((noreturn)) void isr_201();
__attribute__((noreturn)) void isr_202();
__attribute__((noreturn)) void isr_203();
__attribute__((noreturn)) void isr_204();
__attribute__((noreturn)) void isr_205();
__attribute__((noreturn)) void isr_206();
__attribute__((noreturn)) void isr_207();
__attribute__((noreturn)) void isr_208();
__attribute__((noreturn)) void isr_209();
__attribute__((noreturn)) void isr_210();
__attribute__((noreturn)) void isr_211();
__attribute__((noreturn)) void isr_212();
__attribute__((noreturn)) void isr_213();
__attribute__((noreturn)) void isr_214();
__attribute__((noreturn)) void isr_215();
__attribute__((noreturn)) void isr_216();
__attribute__((noreturn)) void isr_217();
__attribute__((noreturn)) void isr_218();
__attribute__((noreturn)) void isr_219();
__attribute__((noreturn)) void isr_220();
__attribute__((noreturn)) void isr_221();
__attribute__((noreturn)) void isr_222();
__attribute__((noreturn)) void isr_223();
__attribute__((noreturn)) void isr_224();
__attribute__((noreturn)) void isr_225();
__attribute__((noreturn)) void isr_226();
__attribute__((noreturn)) void isr_227();
__attribute__((noreturn)) void isr_228();
__attribute__((noreturn)) void isr_229();
__attribute__((noreturn)) void isr_230();
__attribute__((noreturn)) void isr_231();
__attribute__((noreturn)) void isr_232();
__attribute__((noreturn)) void isr_233();
__attribute__((noreturn)) void isr_234();
__attribute__((noreturn)) void isr_235();
__attribute__((noreturn)) void isr_236();
__attribute__((noreturn)) void isr_237();
__attribute__((noreturn)) void isr_238();
__attribute__((noreturn)) void isr_239();
__attribute__((noreturn)) void isr_240();
__attribute__((noreturn)) void isr_241();
__attribute__((noreturn)) void isr_242();
__attribute__((noreturn)) void isr_243();
__attribute__((noreturn)) void isr_244();
__attribute__((noreturn)) void isr_245();
__attribute__((noreturn)) void isr_246();
__attribute__((noreturn)) void isr_247();
__attribute__((noreturn)) void isr_248();
__attribute__((noreturn)) void isr_249();
__attribute__((noreturn)) void isr_250();
__attribute__((noreturn)) void isr_251();
__attribute__((noreturn)) void isr_252();
__attribute__((noreturn)) void isr_253();
__attribute__((noreturn)) void isr_254();
__attribute__((noreturn)) void isr_255();





#endif