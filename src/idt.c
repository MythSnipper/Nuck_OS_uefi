#include "../include/idt.h"
#include "../include/kernel.h"

__attribute__((aligned(0x10)))IDT_Entry IDT[256];
IDT_Descriptor IDTR;

isr_stub isr_table[256] = {
    isr_stub_0, isr_stub_1, isr_stub_2, isr_stub_3, isr_stub_4, isr_stub_5, isr_stub_6, isr_stub_7, isr_stub_8, isr_stub_9, isr_stub_10, isr_stub_11, isr_stub_12, isr_stub_13, isr_stub_14, isr_stub_15, isr_stub_16, isr_stub_17, isr_stub_18, isr_stub_19, isr_stub_20, isr_stub_21, isr_stub_22, isr_stub_23, isr_stub_24, isr_stub_25, isr_stub_26, isr_stub_27, isr_stub_28, isr_stub_29, isr_stub_30, isr_stub_31, isr_stub_32, isr_stub_33, isr_stub_34, isr_stub_35, isr_stub_36, isr_stub_37, isr_stub_38, isr_stub_39, isr_stub_40, isr_stub_41, isr_stub_42, isr_stub_43, isr_stub_44, isr_stub_45, isr_stub_46, isr_stub_47, isr_stub_48, isr_stub_49, isr_stub_50, isr_stub_51, isr_stub_52, isr_stub_53, isr_stub_54, isr_stub_55, isr_stub_56, isr_stub_57, isr_stub_58, isr_stub_59, isr_stub_60, isr_stub_61, isr_stub_62, isr_stub_63, isr_stub_64, isr_stub_65, isr_stub_66, isr_stub_67, isr_stub_68, isr_stub_69, isr_stub_70, isr_stub_71, isr_stub_72, isr_stub_73, isr_stub_74, isr_stub_75, isr_stub_76, isr_stub_77, isr_stub_78, isr_stub_79, isr_stub_80, isr_stub_81, isr_stub_82, isr_stub_83, isr_stub_84, isr_stub_85, isr_stub_86, isr_stub_87, isr_stub_88, isr_stub_89, isr_stub_90, isr_stub_91, isr_stub_92, isr_stub_93, isr_stub_94, isr_stub_95, isr_stub_96, isr_stub_97, isr_stub_98, isr_stub_99, isr_stub_100, isr_stub_101, isr_stub_102, isr_stub_103, isr_stub_104, isr_stub_105, isr_stub_106, isr_stub_107, isr_stub_108, isr_stub_109, isr_stub_110, isr_stub_111, isr_stub_112, isr_stub_113, isr_stub_114, isr_stub_115, isr_stub_116, isr_stub_117, isr_stub_118, isr_stub_119, isr_stub_120, isr_stub_121, isr_stub_122, isr_stub_123, isr_stub_124, isr_stub_125, isr_stub_126, isr_stub_127, isr_stub_128, isr_stub_129, isr_stub_130, isr_stub_131, isr_stub_132, isr_stub_133, isr_stub_134, isr_stub_135, isr_stub_136, isr_stub_137, isr_stub_138, isr_stub_139, isr_stub_140, isr_stub_141, isr_stub_142, isr_stub_143, isr_stub_144, isr_stub_145, isr_stub_146, isr_stub_147, isr_stub_148, isr_stub_149, isr_stub_150, isr_stub_151, isr_stub_152, isr_stub_153, isr_stub_154, isr_stub_155, isr_stub_156, isr_stub_157, isr_stub_158, isr_stub_159, isr_stub_160, isr_stub_161, isr_stub_162, isr_stub_163, isr_stub_164, isr_stub_165, isr_stub_166, isr_stub_167, isr_stub_168, isr_stub_169, isr_stub_170, isr_stub_171, isr_stub_172, isr_stub_173, isr_stub_174, isr_stub_175, isr_stub_176, isr_stub_177, isr_stub_178, isr_stub_179, isr_stub_180, isr_stub_181, isr_stub_182, isr_stub_183, isr_stub_184, isr_stub_185, isr_stub_186, isr_stub_187, isr_stub_188, isr_stub_189, isr_stub_190, isr_stub_191, isr_stub_192, isr_stub_193, isr_stub_194, isr_stub_195, isr_stub_196, isr_stub_197, isr_stub_198, isr_stub_199, isr_stub_200, isr_stub_201, isr_stub_202, isr_stub_203, isr_stub_204, isr_stub_205, isr_stub_206, isr_stub_207, isr_stub_208, isr_stub_209, isr_stub_210, isr_stub_211, isr_stub_212, isr_stub_213, isr_stub_214, isr_stub_215, isr_stub_216, isr_stub_217, isr_stub_218, isr_stub_219, isr_stub_220, isr_stub_221, isr_stub_222, isr_stub_223, isr_stub_224, isr_stub_225, isr_stub_226, isr_stub_227, isr_stub_228, isr_stub_229, isr_stub_230, isr_stub_231, isr_stub_232, isr_stub_233, isr_stub_234, isr_stub_235, isr_stub_236, isr_stub_237, isr_stub_238, isr_stub_239, isr_stub_240, isr_stub_241, isr_stub_242, isr_stub_243, isr_stub_244, isr_stub_245, isr_stub_246, isr_stub_247, isr_stub_248, isr_stub_249, isr_stub_250, isr_stub_251, isr_stub_252, isr_stub_253, isr_stub_254, isr_stub_255
};

void IDT_initialize(uint16_t segment, uint8_t IST){
    IDTR.size = sizeof(IDT)-1;
    IDTR.offset = IDT;

    for(int i=0;i<256;i++){
        IDT_set_entry(&IDT[i], i, isr_table[i], 0x8E, segment, IST);
    }

    //load the IDT
    
    asm volatile(
        ".intel_syntax noprefix\n"
        "lidt [%[idt]]\n"
        ".att_syntax\n"
        :
        : [idt] "r"(&IDTR)
        : "memory"
    );
    printd("idt ptr: %p\r\n", &IDTR);
}

void IDT_set_entry(IDT_Entry* idt, uint8_t vector, void* isr, uint8_t attrs, uint16_t segment, uint8_t IST){
    IDT_Entry* descriptor = &idt[vector];

    descriptor->offset_low = (uint64_t)isr & 0xFFFF;
    descriptor->segment = segment;
    descriptor->ist = IST & 0b111;
    descriptor->attributes = attrs;
    descriptor->offset_mid = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->offset_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}





