#include "../include/kernel.h"


void kernel_main(KERNEL_CONTEXT_TABLE* ctx){
    
    //set GDT entries
    __attribute__((aligned(0x10)))static GDT_Entry GDT[3];
    static GDT_Descriptor GDTPtr;

    GDTPtr.size = sizeof(GDT)-1;
    GDTPtr.offset = (uint64_t)&GDT;

    setGDTEntry(&GDT[0], 0, 0, 0, 0); //null descriptor right here

    setGDTEntry(&GDT[1], 0, 0, //Code segment, base and limit 0 because long mode
    0b10011010, //Access:present, ring 0, non system segment(code/data segment), executable(code segment), non conforming, readable, access
    0b1010 //granularity: page granularity(not byte), size flag(0 because long mode), long mode code, reserved
    );

    setGDTEntry(&GDT[2], 0, 0, //Data segment, base and limit 0 because long mode
    0b10010010, //Access:present, ring 0, non system segment(code/data segment), non executable(data segment), up direction, writable, access
    0b1000 //granularity: page granularity(not byte), size flag(0 because long mode data), not long mode code, reserved
    );

    //load the GDT
    asm volatile(
        ".intel_syntax noprefix\n"
        "lgdt [%[gdt]]\n"
        "mov ax, 0x10\n"
        "mov ds, ax\n"
        "mov es, ax\n"
        "mov fs, ax\n"
        "mov gs, ax\n"
        "mov ss, ax\n"
        "jmp .gdt_loaded_yiper\n"
        ".gdt_loaded_yiper:\n"
        ".att_syntax\n"
        :
        : [gdt] "r"(&GDTPtr)
        : "memory", "rax"
    );

    uint8_t CODE_SEG = sizeof(GDT[0]);
    uint8_t DATA_SEG = sizeof(GDT[0]) * 2;

    //set IDT entries
    __attribute__((aligned(0x10)))static IDT_Entry IDT[256];
    static IDT_Descriptor IDTPtr;

    IDTPtr.size = sizeof(IDT)-1;
    IDTPtr.offset = (uint64_t)&IDT;


    //interrupts
    {
    //atributes: present, ring 0, 0, gate type(1110 interrupt, 1111 trap)
    setIDTEntry(&IDT[0], CODE_SEG, (uint64_t) &isr_0, 0b000, 0b10001110); //#DE Divide Error
    setIDTEntry(&IDT[1], CODE_SEG, (uint64_t) &isr_1, 0b000, 0b10001110); //#DB Debug Exception
    setIDTEntry(&IDT[2], CODE_SEG, (uint64_t) &isr_2, 0b000, 0b10001110); //NMI nonmaskable external int
    setIDTEntry(&IDT[3], CODE_SEG, (uint64_t) &isr_3, 0b000, 0b10001110); //#BP breakpoint
    setIDTEntry(&IDT[4], CODE_SEG, (uint64_t) &isr_4, 0b000, 0b10001110); //#OF overflow
    setIDTEntry(&IDT[5], CODE_SEG, (uint64_t) &isr_5, 0b000, 0b10001110); //#BR BOUND range exceeded
    setIDTEntry(&IDT[6], CODE_SEG, (uint64_t) &isr_6, 0b000, 0b10001110); //#UD invalid opcode
    setIDTEntry(&IDT[7], CODE_SEG, (uint64_t) &isr_7, 0b000, 0b10001110); //#NM device not available(no math processor)
    setIDTEntry(&IDT[8], CODE_SEG, (uint64_t) &isr_8, 0b000, 0b10001110); //#DF double fault
    setIDTEntry(&IDT[9], CODE_SEG, (uint64_t) &isr_9, 0b000, 0b10001110); // coprocessor segment overrun(reserved)
    setIDTEntry(&IDT[10], CODE_SEG, (uint64_t) &isr_10, 0b000, 0b10001110); //#TS invalid TSS
    setIDTEntry(&IDT[11], CODE_SEG, (uint64_t) &isr_11, 0b000, 0b10001110); //#NP Segment Not Present
    setIDTEntry(&IDT[12], CODE_SEG, (uint64_t) &isr_12, 0b000, 0b10001110); //#SS Stack-Segment fault
    setIDTEntry(&IDT[13], CODE_SEG, (uint64_t) &isr_13, 0b000, 0b10001110); //#GP General Protection
    setIDTEntry(&IDT[14], CODE_SEG, (uint64_t) &isr_14, 0b000, 0b10001110); //#PF Page fault
    setIDTEntry(&IDT[15], CODE_SEG, (uint64_t) &isr_15, 0b000, 0b10001110); // intel reserved, DO NOT USE
    setIDTEntry(&IDT[16], CODE_SEG, (uint64_t) &isr_16, 0b000, 0b10001110); //#MF x87 FPU floating-point error(math fault)
    setIDTEntry(&IDT[17], CODE_SEG, (uint64_t) &isr_17, 0b000, 0b10001110); //#AC alignment check
    setIDTEntry(&IDT[18], CODE_SEG, (uint64_t) &isr_18, 0b000, 0b10001110); //#MC machine check
    setIDTEntry(&IDT[19], CODE_SEG, (uint64_t) &isr_19, 0b000, 0b10001110); //#XM SIMD floating point exception
    setIDTEntry(&IDT[20], CODE_SEG, (uint64_t) &isr_20, 0b000, 0b10001110); //#VE virtualization exception
    setIDTEntry(&IDT[21], CODE_SEG, (uint64_t) &isr_21, 0b000, 0b10001110); //#CP control protection exception
    setIDTEntry(&IDT[22], CODE_SEG, (uint64_t) &isr_22, 0b000, 0b10001110); //reserved vv
    setIDTEntry(&IDT[23], CODE_SEG, (uint64_t) &isr_23, 0b000, 0b10001110);
    setIDTEntry(&IDT[24], CODE_SEG, (uint64_t) &isr_24, 0b000, 0b10001110);
    setIDTEntry(&IDT[25], CODE_SEG, (uint64_t) &isr_25, 0b000, 0b10001110);
    setIDTEntry(&IDT[26], CODE_SEG, (uint64_t) &isr_26, 0b000, 0b10001110);
    setIDTEntry(&IDT[27], CODE_SEG, (uint64_t) &isr_27, 0b000, 0b10001110);
    setIDTEntry(&IDT[28], CODE_SEG, (uint64_t) &isr_28, 0b000, 0b10001110);
    setIDTEntry(&IDT[29], CODE_SEG, (uint64_t) &isr_29, 0b000, 0b10001110);
    setIDTEntry(&IDT[30], CODE_SEG, (uint64_t) &isr_30, 0b000, 0b10001110);
    setIDTEntry(&IDT[31], CODE_SEG, (uint64_t) &isr_31, 0b000, 0b10001110); //reserved ^^
    }

    //external interrupts
    {
    setIDTEntry(&IDT[32], CODE_SEG, (uint64_t) &isr_32, 0b000, 0b10001110);
    setIDTEntry(&IDT[33], CODE_SEG, (uint64_t) &isr_33, 0b000, 0b10001110);
    setIDTEntry(&IDT[34], CODE_SEG, (uint64_t) &isr_34, 0b000, 0b10001110);
    setIDTEntry(&IDT[35], CODE_SEG, (uint64_t) &isr_35, 0b000, 0b10001110);
    setIDTEntry(&IDT[36], CODE_SEG, (uint64_t) &isr_36, 0b000, 0b10001110);
    setIDTEntry(&IDT[37], CODE_SEG, (uint64_t) &isr_37, 0b000, 0b10001110);
    setIDTEntry(&IDT[38], CODE_SEG, (uint64_t) &isr_38, 0b000, 0b10001110);
    setIDTEntry(&IDT[39], CODE_SEG, (uint64_t) &isr_39, 0b000, 0b10001110);
    setIDTEntry(&IDT[40], CODE_SEG, (uint64_t) &isr_40, 0b000, 0b10001110);
    setIDTEntry(&IDT[41], CODE_SEG, (uint64_t) &isr_41, 0b000, 0b10001110);
    setIDTEntry(&IDT[42], CODE_SEG, (uint64_t) &isr_42, 0b000, 0b10001110);
    setIDTEntry(&IDT[43], CODE_SEG, (uint64_t) &isr_43, 0b000, 0b10001110);
    setIDTEntry(&IDT[44], CODE_SEG, (uint64_t) &isr_44, 0b000, 0b10001110);
    setIDTEntry(&IDT[45], CODE_SEG, (uint64_t) &isr_45, 0b000, 0b10001110);
    setIDTEntry(&IDT[46], CODE_SEG, (uint64_t) &isr_46, 0b000, 0b10001110);
    setIDTEntry(&IDT[47], CODE_SEG, (uint64_t) &isr_47, 0b000, 0b10001110);
    setIDTEntry(&IDT[48], CODE_SEG, (uint64_t) &isr_48, 0b000, 0b10001110);
    setIDTEntry(&IDT[49], CODE_SEG, (uint64_t) &isr_49, 0b000, 0b10001110);
    setIDTEntry(&IDT[50], CODE_SEG, (uint64_t) &isr_50, 0b000, 0b10001110);
    setIDTEntry(&IDT[51], CODE_SEG, (uint64_t) &isr_51, 0b000, 0b10001110);
    setIDTEntry(&IDT[52], CODE_SEG, (uint64_t) &isr_52, 0b000, 0b10001110);
    setIDTEntry(&IDT[53], CODE_SEG, (uint64_t) &isr_53, 0b000, 0b10001110);
    setIDTEntry(&IDT[54], CODE_SEG, (uint64_t) &isr_54, 0b000, 0b10001110);
    setIDTEntry(&IDT[55], CODE_SEG, (uint64_t) &isr_55, 0b000, 0b10001110);
    setIDTEntry(&IDT[56], CODE_SEG, (uint64_t) &isr_56, 0b000, 0b10001110);
    setIDTEntry(&IDT[57], CODE_SEG, (uint64_t) &isr_57, 0b000, 0b10001110);
    setIDTEntry(&IDT[58], CODE_SEG, (uint64_t) &isr_58, 0b000, 0b10001110);
    setIDTEntry(&IDT[59], CODE_SEG, (uint64_t) &isr_59, 0b000, 0b10001110);
    setIDTEntry(&IDT[60], CODE_SEG, (uint64_t) &isr_60, 0b000, 0b10001110);
    setIDTEntry(&IDT[61], CODE_SEG, (uint64_t) &isr_61, 0b000, 0b10001110);
    setIDTEntry(&IDT[62], CODE_SEG, (uint64_t) &isr_62, 0b000, 0b10001110);
    setIDTEntry(&IDT[63], CODE_SEG, (uint64_t) &isr_63, 0b000, 0b10001110);
    setIDTEntry(&IDT[64], CODE_SEG, (uint64_t) &isr_64, 0b000, 0b10001110);
    setIDTEntry(&IDT[65], CODE_SEG, (uint64_t) &isr_65, 0b000, 0b10001110);
    setIDTEntry(&IDT[66], CODE_SEG, (uint64_t) &isr_66, 0b000, 0b10001110);
    setIDTEntry(&IDT[67], CODE_SEG, (uint64_t) &isr_67, 0b000, 0b10001110);
    setIDTEntry(&IDT[68], CODE_SEG, (uint64_t) &isr_68, 0b000, 0b10001110);
    setIDTEntry(&IDT[69], CODE_SEG, (uint64_t) &isr_69, 0b000, 0b10001110);
    setIDTEntry(&IDT[70], CODE_SEG, (uint64_t) &isr_70, 0b000, 0b10001110);
    setIDTEntry(&IDT[71], CODE_SEG, (uint64_t) &isr_71, 0b000, 0b10001110);
    setIDTEntry(&IDT[72], CODE_SEG, (uint64_t) &isr_72, 0b000, 0b10001110);
    setIDTEntry(&IDT[73], CODE_SEG, (uint64_t) &isr_73, 0b000, 0b10001110);
    setIDTEntry(&IDT[74], CODE_SEG, (uint64_t) &isr_74, 0b000, 0b10001110);
    setIDTEntry(&IDT[75], CODE_SEG, (uint64_t) &isr_75, 0b000, 0b10001110);
    setIDTEntry(&IDT[76], CODE_SEG, (uint64_t) &isr_76, 0b000, 0b10001110);
    setIDTEntry(&IDT[77], CODE_SEG, (uint64_t) &isr_77, 0b000, 0b10001110);
    setIDTEntry(&IDT[78], CODE_SEG, (uint64_t) &isr_78, 0b000, 0b10001110);
    setIDTEntry(&IDT[79], CODE_SEG, (uint64_t) &isr_79, 0b000, 0b10001110);
    setIDTEntry(&IDT[80], CODE_SEG, (uint64_t) &isr_80, 0b000, 0b10001110);
    setIDTEntry(&IDT[81], CODE_SEG, (uint64_t) &isr_81, 0b000, 0b10001110);
    setIDTEntry(&IDT[82], CODE_SEG, (uint64_t) &isr_82, 0b000, 0b10001110);
    setIDTEntry(&IDT[83], CODE_SEG, (uint64_t) &isr_83, 0b000, 0b10001110);
    setIDTEntry(&IDT[84], CODE_SEG, (uint64_t) &isr_84, 0b000, 0b10001110);
    setIDTEntry(&IDT[85], CODE_SEG, (uint64_t) &isr_85, 0b000, 0b10001110);
    setIDTEntry(&IDT[86], CODE_SEG, (uint64_t) &isr_86, 0b000, 0b10001110);
    setIDTEntry(&IDT[87], CODE_SEG, (uint64_t) &isr_87, 0b000, 0b10001110);
    setIDTEntry(&IDT[88], CODE_SEG, (uint64_t) &isr_88, 0b000, 0b10001110);
    setIDTEntry(&IDT[89], CODE_SEG, (uint64_t) &isr_89, 0b000, 0b10001110);
    setIDTEntry(&IDT[90], CODE_SEG, (uint64_t) &isr_90, 0b000, 0b10001110);
    setIDTEntry(&IDT[91], CODE_SEG, (uint64_t) &isr_91, 0b000, 0b10001110);
    setIDTEntry(&IDT[92], CODE_SEG, (uint64_t) &isr_92, 0b000, 0b10001110);
    setIDTEntry(&IDT[93], CODE_SEG, (uint64_t) &isr_93, 0b000, 0b10001110);
    setIDTEntry(&IDT[94], CODE_SEG, (uint64_t) &isr_94, 0b000, 0b10001110);
    setIDTEntry(&IDT[95], CODE_SEG, (uint64_t) &isr_95, 0b000, 0b10001110);
    setIDTEntry(&IDT[96], CODE_SEG, (uint64_t) &isr_96, 0b000, 0b10001110);
    setIDTEntry(&IDT[97], CODE_SEG, (uint64_t) &isr_97, 0b000, 0b10001110);
    setIDTEntry(&IDT[98], CODE_SEG, (uint64_t) &isr_98, 0b000, 0b10001110);
    setIDTEntry(&IDT[99], CODE_SEG, (uint64_t) &isr_99, 0b000, 0b10001110);
    setIDTEntry(&IDT[100], CODE_SEG, (uint64_t) &isr_100, 0b000, 0b10001110);
    setIDTEntry(&IDT[101], CODE_SEG, (uint64_t) &isr_101, 0b000, 0b10001110);
    setIDTEntry(&IDT[102], CODE_SEG, (uint64_t) &isr_102, 0b000, 0b10001110);
    setIDTEntry(&IDT[103], CODE_SEG, (uint64_t) &isr_103, 0b000, 0b10001110);
    setIDTEntry(&IDT[104], CODE_SEG, (uint64_t) &isr_104, 0b000, 0b10001110);
    setIDTEntry(&IDT[105], CODE_SEG, (uint64_t) &isr_105, 0b000, 0b10001110);
    setIDTEntry(&IDT[106], CODE_SEG, (uint64_t) &isr_106, 0b000, 0b10001110);
    setIDTEntry(&IDT[107], CODE_SEG, (uint64_t) &isr_107, 0b000, 0b10001110);
    setIDTEntry(&IDT[108], CODE_SEG, (uint64_t) &isr_108, 0b000, 0b10001110);
    setIDTEntry(&IDT[109], CODE_SEG, (uint64_t) &isr_109, 0b000, 0b10001110);
    setIDTEntry(&IDT[110], CODE_SEG, (uint64_t) &isr_110, 0b000, 0b10001110);
    setIDTEntry(&IDT[111], CODE_SEG, (uint64_t) &isr_111, 0b000, 0b10001110);
    setIDTEntry(&IDT[112], CODE_SEG, (uint64_t) &isr_112, 0b000, 0b10001110);
    setIDTEntry(&IDT[113], CODE_SEG, (uint64_t) &isr_113, 0b000, 0b10001110);
    setIDTEntry(&IDT[114], CODE_SEG, (uint64_t) &isr_114, 0b000, 0b10001110);
    setIDTEntry(&IDT[115], CODE_SEG, (uint64_t) &isr_115, 0b000, 0b10001110);
    setIDTEntry(&IDT[116], CODE_SEG, (uint64_t) &isr_116, 0b000, 0b10001110);
    setIDTEntry(&IDT[117], CODE_SEG, (uint64_t) &isr_117, 0b000, 0b10001110);
    setIDTEntry(&IDT[118], CODE_SEG, (uint64_t) &isr_118, 0b000, 0b10001110);
    setIDTEntry(&IDT[119], CODE_SEG, (uint64_t) &isr_119, 0b000, 0b10001110);
    setIDTEntry(&IDT[120], CODE_SEG, (uint64_t) &isr_120, 0b000, 0b10001110);
    setIDTEntry(&IDT[121], CODE_SEG, (uint64_t) &isr_121, 0b000, 0b10001110);
    setIDTEntry(&IDT[122], CODE_SEG, (uint64_t) &isr_122, 0b000, 0b10001110);
    setIDTEntry(&IDT[123], CODE_SEG, (uint64_t) &isr_123, 0b000, 0b10001110);
    setIDTEntry(&IDT[124], CODE_SEG, (uint64_t) &isr_124, 0b000, 0b10001110);
    setIDTEntry(&IDT[125], CODE_SEG, (uint64_t) &isr_125, 0b000, 0b10001110);
    setIDTEntry(&IDT[126], CODE_SEG, (uint64_t) &isr_126, 0b000, 0b10001110);
    setIDTEntry(&IDT[127], CODE_SEG, (uint64_t) &isr_127, 0b000, 0b10001110);
    setIDTEntry(&IDT[128], CODE_SEG, (uint64_t) &isr_128, 0b000, 0b10001110);
    setIDTEntry(&IDT[129], CODE_SEG, (uint64_t) &isr_129, 0b000, 0b10001110);
    setIDTEntry(&IDT[130], CODE_SEG, (uint64_t) &isr_130, 0b000, 0b10001110);
    setIDTEntry(&IDT[131], CODE_SEG, (uint64_t) &isr_131, 0b000, 0b10001110);
    setIDTEntry(&IDT[132], CODE_SEG, (uint64_t) &isr_132, 0b000, 0b10001110);
    setIDTEntry(&IDT[133], CODE_SEG, (uint64_t) &isr_133, 0b000, 0b10001110);
    setIDTEntry(&IDT[134], CODE_SEG, (uint64_t) &isr_134, 0b000, 0b10001110);
    setIDTEntry(&IDT[135], CODE_SEG, (uint64_t) &isr_135, 0b000, 0b10001110);
    setIDTEntry(&IDT[136], CODE_SEG, (uint64_t) &isr_136, 0b000, 0b10001110);
    setIDTEntry(&IDT[137], CODE_SEG, (uint64_t) &isr_137, 0b000, 0b10001110);
    setIDTEntry(&IDT[138], CODE_SEG, (uint64_t) &isr_138, 0b000, 0b10001110);
    setIDTEntry(&IDT[139], CODE_SEG, (uint64_t) &isr_139, 0b000, 0b10001110);
    setIDTEntry(&IDT[140], CODE_SEG, (uint64_t) &isr_140, 0b000, 0b10001110);
    setIDTEntry(&IDT[141], CODE_SEG, (uint64_t) &isr_141, 0b000, 0b10001110);
    setIDTEntry(&IDT[142], CODE_SEG, (uint64_t) &isr_142, 0b000, 0b10001110);
    setIDTEntry(&IDT[143], CODE_SEG, (uint64_t) &isr_143, 0b000, 0b10001110);
    setIDTEntry(&IDT[144], CODE_SEG, (uint64_t) &isr_144, 0b000, 0b10001110);
    setIDTEntry(&IDT[145], CODE_SEG, (uint64_t) &isr_145, 0b000, 0b10001110);
    setIDTEntry(&IDT[146], CODE_SEG, (uint64_t) &isr_146, 0b000, 0b10001110);
    setIDTEntry(&IDT[147], CODE_SEG, (uint64_t) &isr_147, 0b000, 0b10001110);
    setIDTEntry(&IDT[148], CODE_SEG, (uint64_t) &isr_148, 0b000, 0b10001110);
    setIDTEntry(&IDT[149], CODE_SEG, (uint64_t) &isr_149, 0b000, 0b10001110);
    setIDTEntry(&IDT[150], CODE_SEG, (uint64_t) &isr_150, 0b000, 0b10001110);
    setIDTEntry(&IDT[151], CODE_SEG, (uint64_t) &isr_151, 0b000, 0b10001110);
    setIDTEntry(&IDT[152], CODE_SEG, (uint64_t) &isr_152, 0b000, 0b10001110);
    setIDTEntry(&IDT[153], CODE_SEG, (uint64_t) &isr_153, 0b000, 0b10001110);
    setIDTEntry(&IDT[154], CODE_SEG, (uint64_t) &isr_154, 0b000, 0b10001110);
    setIDTEntry(&IDT[155], CODE_SEG, (uint64_t) &isr_155, 0b000, 0b10001110);
    setIDTEntry(&IDT[156], CODE_SEG, (uint64_t) &isr_156, 0b000, 0b10001110);
    setIDTEntry(&IDT[157], CODE_SEG, (uint64_t) &isr_157, 0b000, 0b10001110);
    setIDTEntry(&IDT[158], CODE_SEG, (uint64_t) &isr_158, 0b000, 0b10001110);
    setIDTEntry(&IDT[159], CODE_SEG, (uint64_t) &isr_159, 0b000, 0b10001110);
    setIDTEntry(&IDT[160], CODE_SEG, (uint64_t) &isr_160, 0b000, 0b10001110);
    setIDTEntry(&IDT[161], CODE_SEG, (uint64_t) &isr_161, 0b000, 0b10001110);
    setIDTEntry(&IDT[162], CODE_SEG, (uint64_t) &isr_162, 0b000, 0b10001110);
    setIDTEntry(&IDT[163], CODE_SEG, (uint64_t) &isr_163, 0b000, 0b10001110);
    setIDTEntry(&IDT[164], CODE_SEG, (uint64_t) &isr_164, 0b000, 0b10001110);
    setIDTEntry(&IDT[165], CODE_SEG, (uint64_t) &isr_165, 0b000, 0b10001110);
    setIDTEntry(&IDT[166], CODE_SEG, (uint64_t) &isr_166, 0b000, 0b10001110);
    setIDTEntry(&IDT[167], CODE_SEG, (uint64_t) &isr_167, 0b000, 0b10001110);
    setIDTEntry(&IDT[168], CODE_SEG, (uint64_t) &isr_168, 0b000, 0b10001110);
    setIDTEntry(&IDT[169], CODE_SEG, (uint64_t) &isr_169, 0b000, 0b10001110);
    setIDTEntry(&IDT[170], CODE_SEG, (uint64_t) &isr_170, 0b000, 0b10001110);
    setIDTEntry(&IDT[171], CODE_SEG, (uint64_t) &isr_171, 0b000, 0b10001110);
    setIDTEntry(&IDT[172], CODE_SEG, (uint64_t) &isr_172, 0b000, 0b10001110);
    setIDTEntry(&IDT[173], CODE_SEG, (uint64_t) &isr_173, 0b000, 0b10001110);
    setIDTEntry(&IDT[174], CODE_SEG, (uint64_t) &isr_174, 0b000, 0b10001110);
    setIDTEntry(&IDT[175], CODE_SEG, (uint64_t) &isr_175, 0b000, 0b10001110);
    setIDTEntry(&IDT[176], CODE_SEG, (uint64_t) &isr_176, 0b000, 0b10001110);
    setIDTEntry(&IDT[177], CODE_SEG, (uint64_t) &isr_177, 0b000, 0b10001110);
    setIDTEntry(&IDT[178], CODE_SEG, (uint64_t) &isr_178, 0b000, 0b10001110);
    setIDTEntry(&IDT[179], CODE_SEG, (uint64_t) &isr_179, 0b000, 0b10001110);
    setIDTEntry(&IDT[180], CODE_SEG, (uint64_t) &isr_180, 0b000, 0b10001110);
    setIDTEntry(&IDT[181], CODE_SEG, (uint64_t) &isr_181, 0b000, 0b10001110);
    setIDTEntry(&IDT[182], CODE_SEG, (uint64_t) &isr_182, 0b000, 0b10001110);
    setIDTEntry(&IDT[183], CODE_SEG, (uint64_t) &isr_183, 0b000, 0b10001110);
    setIDTEntry(&IDT[184], CODE_SEG, (uint64_t) &isr_184, 0b000, 0b10001110);
    setIDTEntry(&IDT[185], CODE_SEG, (uint64_t) &isr_185, 0b000, 0b10001110);
    setIDTEntry(&IDT[186], CODE_SEG, (uint64_t) &isr_186, 0b000, 0b10001110);
    setIDTEntry(&IDT[187], CODE_SEG, (uint64_t) &isr_187, 0b000, 0b10001110);
    setIDTEntry(&IDT[188], CODE_SEG, (uint64_t) &isr_188, 0b000, 0b10001110);
    setIDTEntry(&IDT[189], CODE_SEG, (uint64_t) &isr_189, 0b000, 0b10001110);
    setIDTEntry(&IDT[190], CODE_SEG, (uint64_t) &isr_190, 0b000, 0b10001110);
    setIDTEntry(&IDT[191], CODE_SEG, (uint64_t) &isr_191, 0b000, 0b10001110);
    setIDTEntry(&IDT[192], CODE_SEG, (uint64_t) &isr_192, 0b000, 0b10001110);
    setIDTEntry(&IDT[193], CODE_SEG, (uint64_t) &isr_193, 0b000, 0b10001110);
    setIDTEntry(&IDT[194], CODE_SEG, (uint64_t) &isr_194, 0b000, 0b10001110);
    setIDTEntry(&IDT[195], CODE_SEG, (uint64_t) &isr_195, 0b000, 0b10001110);
    setIDTEntry(&IDT[196], CODE_SEG, (uint64_t) &isr_196, 0b000, 0b10001110);
    setIDTEntry(&IDT[197], CODE_SEG, (uint64_t) &isr_197, 0b000, 0b10001110);
    setIDTEntry(&IDT[198], CODE_SEG, (uint64_t) &isr_198, 0b000, 0b10001110);
    setIDTEntry(&IDT[199], CODE_SEG, (uint64_t) &isr_199, 0b000, 0b10001110);
    setIDTEntry(&IDT[200], CODE_SEG, (uint64_t) &isr_200, 0b000, 0b10001110);
    setIDTEntry(&IDT[201], CODE_SEG, (uint64_t) &isr_201, 0b000, 0b10001110);
    setIDTEntry(&IDT[202], CODE_SEG, (uint64_t) &isr_202, 0b000, 0b10001110);
    setIDTEntry(&IDT[203], CODE_SEG, (uint64_t) &isr_203, 0b000, 0b10001110);
    setIDTEntry(&IDT[204], CODE_SEG, (uint64_t) &isr_204, 0b000, 0b10001110);
    setIDTEntry(&IDT[205], CODE_SEG, (uint64_t) &isr_205, 0b000, 0b10001110);
    setIDTEntry(&IDT[206], CODE_SEG, (uint64_t) &isr_206, 0b000, 0b10001110);
    setIDTEntry(&IDT[207], CODE_SEG, (uint64_t) &isr_207, 0b000, 0b10001110);
    setIDTEntry(&IDT[208], CODE_SEG, (uint64_t) &isr_208, 0b000, 0b10001110);
    setIDTEntry(&IDT[209], CODE_SEG, (uint64_t) &isr_209, 0b000, 0b10001110);
    setIDTEntry(&IDT[210], CODE_SEG, (uint64_t) &isr_210, 0b000, 0b10001110);
    setIDTEntry(&IDT[211], CODE_SEG, (uint64_t) &isr_211, 0b000, 0b10001110);
    setIDTEntry(&IDT[212], CODE_SEG, (uint64_t) &isr_212, 0b000, 0b10001110);
    setIDTEntry(&IDT[213], CODE_SEG, (uint64_t) &isr_213, 0b000, 0b10001110);
    setIDTEntry(&IDT[214], CODE_SEG, (uint64_t) &isr_214, 0b000, 0b10001110);
    setIDTEntry(&IDT[215], CODE_SEG, (uint64_t) &isr_215, 0b000, 0b10001110);
    setIDTEntry(&IDT[216], CODE_SEG, (uint64_t) &isr_216, 0b000, 0b10001110);
    setIDTEntry(&IDT[217], CODE_SEG, (uint64_t) &isr_217, 0b000, 0b10001110);
    setIDTEntry(&IDT[218], CODE_SEG, (uint64_t) &isr_218, 0b000, 0b10001110);
    setIDTEntry(&IDT[219], CODE_SEG, (uint64_t) &isr_219, 0b000, 0b10001110);
    setIDTEntry(&IDT[220], CODE_SEG, (uint64_t) &isr_220, 0b000, 0b10001110);
    setIDTEntry(&IDT[221], CODE_SEG, (uint64_t) &isr_221, 0b000, 0b10001110);
    setIDTEntry(&IDT[222], CODE_SEG, (uint64_t) &isr_222, 0b000, 0b10001110);
    setIDTEntry(&IDT[223], CODE_SEG, (uint64_t) &isr_223, 0b000, 0b10001110);
    setIDTEntry(&IDT[224], CODE_SEG, (uint64_t) &isr_224, 0b000, 0b10001110);
    setIDTEntry(&IDT[225], CODE_SEG, (uint64_t) &isr_225, 0b000, 0b10001110);
    setIDTEntry(&IDT[226], CODE_SEG, (uint64_t) &isr_226, 0b000, 0b10001110);
    setIDTEntry(&IDT[227], CODE_SEG, (uint64_t) &isr_227, 0b000, 0b10001110);
    setIDTEntry(&IDT[228], CODE_SEG, (uint64_t) &isr_228, 0b000, 0b10001110);
    setIDTEntry(&IDT[229], CODE_SEG, (uint64_t) &isr_229, 0b000, 0b10001110);
    setIDTEntry(&IDT[230], CODE_SEG, (uint64_t) &isr_230, 0b000, 0b10001110);
    setIDTEntry(&IDT[231], CODE_SEG, (uint64_t) &isr_231, 0b000, 0b10001110);
    setIDTEntry(&IDT[232], CODE_SEG, (uint64_t) &isr_232, 0b000, 0b10001110);
    setIDTEntry(&IDT[233], CODE_SEG, (uint64_t) &isr_233, 0b000, 0b10001110);
    setIDTEntry(&IDT[234], CODE_SEG, (uint64_t) &isr_234, 0b000, 0b10001110);
    setIDTEntry(&IDT[235], CODE_SEG, (uint64_t) &isr_235, 0b000, 0b10001110);
    setIDTEntry(&IDT[236], CODE_SEG, (uint64_t) &isr_236, 0b000, 0b10001110);
    setIDTEntry(&IDT[237], CODE_SEG, (uint64_t) &isr_237, 0b000, 0b10001110);
    setIDTEntry(&IDT[238], CODE_SEG, (uint64_t) &isr_238, 0b000, 0b10001110);
    setIDTEntry(&IDT[239], CODE_SEG, (uint64_t) &isr_239, 0b000, 0b10001110);
    setIDTEntry(&IDT[240], CODE_SEG, (uint64_t) &isr_240, 0b000, 0b10001110);
    setIDTEntry(&IDT[241], CODE_SEG, (uint64_t) &isr_241, 0b000, 0b10001110);
    setIDTEntry(&IDT[242], CODE_SEG, (uint64_t) &isr_242, 0b000, 0b10001110);
    setIDTEntry(&IDT[243], CODE_SEG, (uint64_t) &isr_243, 0b000, 0b10001110);
    setIDTEntry(&IDT[244], CODE_SEG, (uint64_t) &isr_244, 0b000, 0b10001110);
    setIDTEntry(&IDT[245], CODE_SEG, (uint64_t) &isr_245, 0b000, 0b10001110);
    setIDTEntry(&IDT[246], CODE_SEG, (uint64_t) &isr_246, 0b000, 0b10001110);
    setIDTEntry(&IDT[247], CODE_SEG, (uint64_t) &isr_247, 0b000, 0b10001110);
    setIDTEntry(&IDT[248], CODE_SEG, (uint64_t) &isr_248, 0b000, 0b10001110);
    setIDTEntry(&IDT[249], CODE_SEG, (uint64_t) &isr_249, 0b000, 0b10001110);
    setIDTEntry(&IDT[250], CODE_SEG, (uint64_t) &isr_250, 0b000, 0b10001110);
    setIDTEntry(&IDT[251], CODE_SEG, (uint64_t) &isr_251, 0b000, 0b10001110);
    setIDTEntry(&IDT[252], CODE_SEG, (uint64_t) &isr_252, 0b000, 0b10001110);
    setIDTEntry(&IDT[253], CODE_SEG, (uint64_t) &isr_253, 0b000, 0b10001110);
    setIDTEntry(&IDT[254], CODE_SEG, (uint64_t) &isr_254, 0b000, 0b10001110);
    setIDTEntry(&IDT[255], CODE_SEG, (uint64_t) &isr_255, 0b000, 0b10001110);
    }

    //load the IDT
    asm volatile(
        ".intel_syntax noprefix\n"
        "lidt [%[idt]]\n"
        ".att_syntax\n"
        :
        : [idt] "r"(&IDTPtr)
        : "memory"
    );

    uint32_t versionMajor = 1;
    uint32_t versionMinor = 2;
    //font
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

    //get CPU vendor
    uint32_t CPUVendor_r[4];
    cpuid_string(0, CPUVendor_r);
    uint8_t CPUVendor[13];
    ((uint32_t*)CPUVendor)[0] = CPUVendor_r[1];
    ((uint32_t*)CPUVendor)[1] = CPUVendor_r[3];
    ((uint32_t*)CPUVendor)[2] = CPUVendor_r[2];
    CPUVendor[12] = 0;

    //Display
    //swap the buffers so the GOP framebuffer is actually the backbuffer
    {
        EFI_PHYSICAL_ADDRESS backbuf = ctx->fb;
        ctx->fb = ctx->GOP->FrameBufferBase; //set fb to vram
        ctx->GOP->FrameBufferBase = backbuf; //set the GOP address to backbuffer
    }

    bool fill = true;
    uint32_t screenX = ctx->GOP->Info->HorizontalResolution - 1;
    uint32_t screenYFraction = ctx->GOP->Info->VerticalResolution / 5;
    KERNEL_TEXT_OUTPUT title = {VGAfont, 8, 16, 2, 2, 0, 0, 20, 20, hex(0xFF10F0), hex(0x000000), true};
    KERNEL_TEXT_OUTPUT ConOut = {VGAfont, 8, 16, 1, 1, 0, 8, 0, 0, hex(0xFF10F0), hex(0x000000), false};
    KERNEL_TEXT_OUTPUT HeapOut = {VGAfont, 8, 16, 1, 1, 0, 0, 0, 0, hex(0xFF10F0), hex(0x000000), false};


    //parse video headers
    uint8_t* video_addr = (uint8_t*) ctx->videoFile;
    uint32_t video_format = *(uint32_t*)(video_addr);
    uint32_t video_width = *(uint32_t*)(video_addr+4);
    uint32_t video_height = *(uint32_t*)(video_addr+8);
    uint32_t video_frameCount = *(uint32_t*)(video_addr+12);
    video_addr += 16;

    uint32_t video_frameCounter = 0;
    uint32_t e = 0;

    //parse image header
    uint8_t* image_addr = (uint8_t*) ctx->imageFile;
    uint32_t image_format = *(uint32_t*)(image_addr);
    uint32_t image_width = *(uint32_t*)(image_addr+4);
    uint32_t image_height = *(uint32_t*)(image_addr+8);
    uint32_t image_frameCount = *(uint32_t*)(image_addr+12);
    image_addr += 16;



    //MEM ALLOC STUFFFFFF IDK
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

    while(true){
        title = (KERNEL_TEXT_OUTPUT){VGAfont, 8, 16, 2, 2, 0, 0, 20, 20, hex(0xFF10F0), hex(0x000000), true};
        ConOut = (KERNEL_TEXT_OUTPUT){VGAfont, 8, 16, 1, 1, 0, 8, 0, 0, hex(0xFF10F0), hex(0x000000), false};
        HeapOut =(KERNEL_TEXT_OUTPUT){VGAfont, 8, 16, 1, 1, 0, 0, 0, 0, hex(0xFF10F0), hex(0x000000), false};
        //clear screen
        GOPDrawRect(ctx->GOP, 0, 0, ctx->GOP->Info->HorizontalResolution-1, ctx->GOP->Info->VerticalResolution-1, rgba(0, 0, 0, 0), true);
        /*
        GOPDrawRect(ctx->GOP, 0, 0, screenX, screenYFraction - 1, hex(0x55CDFC), fill);
        GOPDrawRect(ctx->GOP, 0, screenYFraction, screenX, 2*screenYFraction - 1, hex(0xF7A8B8), fill);
        GOPDrawRect(ctx->GOP, 0, 2*screenYFraction, screenX, 3*screenYFraction - 1, hex(0xFFFFFF), fill);
        GOPDrawRect(ctx->GOP, 0, 3*screenYFraction, screenX, 4*screenYFraction - 1, hex(0xF7A8B8), fill);
        GOPDrawRect(ctx->GOP, 0, 4*screenYFraction, screenX, 5*screenYFraction - 1, hex(0x55CDFC), fill);
        */
        GOPDrawRect(ctx->GOP, 0, 0, ctx->GOP->Info->HorizontalResolution-1, ctx->GOP->Info->VerticalResolution-1, hex(0x00807F), true);





        //play video
        if(e > 100){
            playVideo(ctx->GOP, ctx->GOP->Info->HorizontalResolution - video_width, 0, video_format, video_addr, video_width, video_height, video_frameCount, &video_frameCounter, true, 4);
        }
        else{
            e++;
        }

        //logo
        GOPDrawImage(ctx->GOP, ctx->GOP->Info->HorizontalResolution - image_width - 10, ctx->GOP->Info->VerticalResolution - image_height - 10, image_width, image_height, image_addr, image_format);
        


        printf(ctx->GOP, &ConOut, "operating system of the future\r\n");
        printf(ctx->GOP, &ConOut, "Display pixel format: %d\r\n", ctx->GOP->Info->PixelFormat);
        printf(ctx->GOP, &ConOut, "Code segment: %x\r\nData segment: %x\r\nCPU Vendor: %s\r\n", CODE_SEG, DATA_SEG, CPUVendor);
        printf(ctx->GOP, &ConOut, "Video resolution: %dx%d\r\nformat %d\r\nframe %d/%d\r\n", video_width, video_height, video_format, video_frameCounter+1, video_frameCount);

        printf(ctx->GOP, &title, "Welcome to \r\n");
        title.frontColor = 0xE50000;title.backColor = 0x000000;
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

        printf(ctx->GOP, &title, " Version %u.%u!\r\n", versionMajor, versionMinor);


        heap_display(ctx->heap, ctx->GOP, &HeapOut);
        printf(ctx->GOP, &ConOut, "\r\nheap page allocator: \r\n%lx, +32768 pages, %ld MB\r\n", ctx->heap->heap, 32768*4*1024/1024/1024);
        printf(ctx->GOP, &ConOut, "\r\n\nsubpage allocator: \r\n%lx to %lx\r\n", alloc.freeListStart, alloc.freeListEnd);
        printf(ctx->GOP, &ConOut, "1st subpage: %lx\r\n", subPtr);
        printf(ctx->GOP, &ConOut, "2nd subpage: %lx\r\n", subPtr2);
        
        printf(ctx->GOP, &ConOut, "\r\n\n                            (Logo designed by Serafim Kulukundis)\r\n");
        //copy framebuffer
        memcpy((void*)ctx->GOP->FrameBufferBase, (void*)ctx->fb, ctx->GOP->FrameBufferSize);
    }
    while(true);
}























//PIC definitions
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC_EOI		0x20		/* End-of-interrupt command code */
//PIC functions
void PIC_sendEOI(uint8_t irq){
    if(irq >= 8){ //slave PIC
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI); //master PIC
}

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */
/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(uint8_t offset1, uint8_t offset2){
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();
	// Unmask both PICs.
	outb(PIC1_DATA, 0);
	outb(PIC2_DATA, 0);
}
void PIC_disable(){
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}
void IRQ_set_mask(uint8_t IRQline){
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);        
}
void IRQ_clear_mask(uint8_t IRQline){
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);        
}
#define PIC_READ_IRR                0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */
/* Helper func */
static uint16_t __pic_get_irq_reg(int ocw3){
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}
/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(){
    return __pic_get_irq_reg(PIC_READ_IRR);
}
/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(){
    return __pic_get_irq_reg(PIC_READ_ISR);
}






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

//GDT/IDT functions, general functions
void setIDTEntry(IDT_Entry* entry, uint16_t segment, uint64_t offset, uint8_t ISTOffset, uint8_t attributes){
    entry->offset_low = (uint16_t)(offset & 0xFFFF);
    entry->segment = segment;
    entry->ist = (uint8_t)(ISTOffset & 0b111); //only last 3 bits are the ist, 5 high bits set to 0 because reserved
    entry->attributes = attributes;
    entry->offset_mid = (uint16_t)((offset >> 16) & 0xFFFF);
    entry->offset_high = (uint32_t)((offset >> 32) & 0xFFFFFFFF);
    entry->reserved = 0;
}
void setGDTEntry(GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
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
void playVideo(EFI_GOP* GOP, uint32_t x, uint32_t y, uint32_t format, uint8_t* addr, uint32_t frameWidth, uint32_t frameHeight, uint32_t frameCount, uint32_t* frameCounter, bool loop, uint8_t skips){
    if(*frameCounter >= frameCount){
        return;
    }
    uint8_t* frameAddr = (uint8_t*)addr;
    switch(format){
        case 0: { //black and white, packed
            frameAddr += (*frameCounter) * (frameWidth * frameHeight / 8);
            GOPDrawImage(GOP, x, y, frameWidth, frameHeight, frameAddr, format);
            break;
        }
    }
    (*frameCounter)+=skips;
    if(*frameCounter >= frameCount && loop){
        *frameCounter = 0;
    }
}
void GOPDrawImage(EFI_GOP* GOP, uint32_t x, uint32_t y, uint32_t imgwidth, uint32_t imgheight, uint8_t* imgaddr, uint32_t format){
    switch(format){
        case 0: { //black and white, packed, imgwidth is number of pixels, imgheight is number of pixels
            uint32_t bpr = imgwidth/8; //bytes per row
            for(uint32_t row = 0;row < imgheight;row++){
                for(uint32_t byte = 0;byte < bpr;byte++){
                    //calculate byte
                    uint8_t b = imgaddr[row * bpr + byte];
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
            uint32_t bpr = imgwidth * 3; //bytes per row
            for(uint32_t row = 0;row < imgheight;row++){
                for(uint32_t col = 0;col < imgwidth;col++){
                    //row, col is pixel position
                    //get pixel byte position
                    uint32_t color_byte = *(uint32_t*)(imgaddr + row * bpr + col * 3);
                    uint32_t draw_x = x + col;
                    uint32_t draw_y = y + row;
                    GOPPutPixel(GOP, draw_x, draw_y, hex(color_byte)); //color is ARGB;
                }
            }
        }
    }
}
void printf(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t* str, ...){
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
                printString(GOP, ConOut, (uint8_t*)va_arg(args, uint8_t*));
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

    uint8_t* charmap = "0123456789ABCDEF"; //character map

    while(num != 0){
        index--;
        buff[index] = charmap[num % base]; //push digit to buffer
        num /= base;
    }
    printString(GOP, ConOut, &buff[index]);
}
void printString(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t* string){
    while(*string){ //while it's not null
        printChar(GOP, ConOut, *string++);
    }
}
void printChar(EFI_GOP* GOP, KERNEL_TEXT_OUTPUT* ConOut, uint8_t ascii_char){
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
static inline uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    return ((uint32_t)b) | ((uint32_t)g << 8) | ((uint32_t)r << 16) | ((uint32_t)a << 24);
}
static inline uint32_t hex(uint32_t hex){
    return hex | 0xFF000000;
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
static inline void outb(uint16_t port, uint8_t value){
    asm volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}
static inline void outw(uint16_t port, uint16_t value){
    asm volatile (
        "outw %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}
static inline void outl(uint16_t port, uint32_t value){
    asm volatile (
        "outl %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}
static inline uint8_t inb(uint16_t port){
    uint8_t ret;
    asm volatile (
        "inb %1, %0"
        : "=a"(ret)
        : "Nd"(port)
    );
    return ret;
}
static inline uint16_t inw(uint16_t port){
    uint16_t ret;
    asm volatile (
        "inw %1, %0"
        : "=a"(ret)
        : "Nd"(port)
    );
    return ret;
}
static inline uint32_t inl(uint16_t port){
    uint32_t ret;
    asm volatile (
        "inl %1, %0"
        : "=a"(ret)
        : "Nd"(port)
    );
    return ret;
}
static inline void io_wait(){
    outb(0x80, 0);
}
static inline void cpuid(int code, uint32_t* a, uint32_t* d){
    asm volatile("cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx");
}
static inline int cpuid_string(int code, uint32_t where[4]){
  asm volatile("cpuid":"=a"(*where),"=b"(*(where+1)),
               "=c"(*(where+2)),"=d"(*(where+3)):"a"(code));
  return (int)where[0];
}
uint64_t rdtsc(){
    uint32_t low, high;
    asm volatile("rdtsc":"=a"(low),"=d"(high));
    return ((uint64_t)high << 32) | low;
}

void isr_0(){
    asm volatile("iretq");
}
void isr_1(){
    asm volatile("iretq");
}
void isr_2(){
    asm volatile("iretq");
}
void isr_3(){
    asm volatile("iretq");
}
void isr_4(){
    asm volatile("iretq");
}
void isr_5(){
    asm volatile("iretq");
}
void isr_6(){
    asm volatile("iretq");
}
void isr_7(){
    asm volatile("iretq");
}
void isr_8(){
    asm volatile("iretq");
}
void isr_9(){
    asm volatile("iretq");
}
void isr_10(){
    asm volatile("iretq");
}
void isr_11(){
    asm volatile("iretq");
}
void isr_12(){
    asm volatile("iretq");
}
void isr_13(){
    asm volatile("iretq");
}
void isr_14(){
    asm volatile("iretq");
}
void isr_15(){
    asm volatile("iretq");
}
void isr_16(){
    asm volatile("iretq");
}
void isr_17(){
    asm volatile("iretq");
}
void isr_18(){
    asm volatile("iretq");
}
void isr_19(){
    asm volatile("iretq");
}
void isr_20(){
    asm volatile("iretq");
}
void isr_21(){
    asm volatile("iretq");
}
void isr_22(){
    asm volatile("iretq");
}
void isr_23(){
    asm volatile("iretq");
}
void isr_24(){
    asm volatile("iretq");
}
void isr_25(){
    asm volatile("iretq");
}
void isr_26(){
    asm volatile("iretq");
}
void isr_27(){
    asm volatile("iretq");
}
void isr_28(){
    asm volatile("iretq");
}
void isr_29(){
    asm volatile("iretq");
}
void isr_30(){
    asm volatile("iretq");
}
void isr_31(){
    asm volatile("iretq");
}


void isr_32(){
    asm volatile("iretq");
}
void isr_33(){

    asm volatile("iretq");
}
void isr_34(){
    asm volatile("iretq");
}
void isr_35(){
    asm volatile("iretq");
}
void isr_36(){
    asm volatile("iretq");
}
void isr_37(){
    asm volatile("iretq");
}
void isr_38(){
    asm volatile("iretq");
}
void isr_39(){
    asm volatile("iretq");
}
void isr_40(){
    asm volatile("iretq");
}
void isr_41(){
    asm volatile("iretq");
}
void isr_42(){
    asm volatile("iretq");
}
void isr_43(){
    asm volatile("iretq");
}
void isr_44(){
    asm volatile("iretq");
}
void isr_45(){
    asm volatile("iretq");
}
void isr_46(){
    asm volatile("iretq");
}
void isr_47(){
    asm volatile("iretq");
}
void isr_48(){
    asm volatile("iretq");
}
void isr_49(){
    asm volatile("iretq");
}
void isr_50(){
    asm volatile("iretq");
}
void isr_51(){
    asm volatile("iretq");
}
void isr_52(){
    asm volatile("iretq");
}
void isr_53(){
    asm volatile("iretq");
}
void isr_54(){
    asm volatile("iretq");
}
void isr_55(){
    asm volatile("iretq");
}
void isr_56(){
    asm volatile("iretq");
}
void isr_57(){
    asm volatile("iretq");
}
void isr_58(){
    asm volatile("iretq");
}
void isr_59(){
    asm volatile("iretq");
}
void isr_60(){
    asm volatile("iretq");
}
void isr_61(){
    asm volatile("iretq");
}
void isr_62(){
    asm volatile("iretq");
}
void isr_63(){
    asm volatile("iretq");
}
void isr_64(){
    asm volatile("iretq");
}
void isr_65(){
    asm volatile("iretq");
}
void isr_66(){
    asm volatile("iretq");
}
void isr_67(){
    asm volatile("iretq");
}
void isr_68(){
    asm volatile("iretq");
}
void isr_69(){
    asm volatile("iretq");
}
void isr_70(){
    asm volatile("iretq");
}
void isr_71(){
    asm volatile("iretq");
}
void isr_72(){
    asm volatile("iretq");
}
void isr_73(){
    asm volatile("iretq");
}
void isr_74(){
    asm volatile("iretq");
}
void isr_75(){
    asm volatile("iretq");
}
void isr_76(){
    asm volatile("iretq");
}
void isr_77(){
    asm volatile("iretq");
}
void isr_78(){
    asm volatile("iretq");
}
void isr_79(){
    asm volatile("iretq");
}
void isr_80(){
    asm volatile("iretq");
}
void isr_81(){
    asm volatile("iretq");
}
void isr_82(){
    asm volatile("iretq");
}
void isr_83(){
    asm volatile("iretq");
}
void isr_84(){
    asm volatile("iretq");
}
void isr_85(){
    asm volatile("iretq");
}
void isr_86(){
    asm volatile("iretq");
}
void isr_87(){
    asm volatile("iretq");
}
void isr_88(){
    asm volatile("iretq");
}
void isr_89(){
    asm volatile("iretq");
}
void isr_90(){
    asm volatile("iretq");
}
void isr_91(){
    asm volatile("iretq");
}
void isr_92(){
    asm volatile("iretq");
}
void isr_93(){
    asm volatile("iretq");
}
void isr_94(){
    asm volatile("iretq");
}
void isr_95(){
    asm volatile("iretq");
}
void isr_96(){
    asm volatile("iretq");
}
void isr_97(){
    asm volatile("iretq");
}
void isr_98(){
    asm volatile("iretq");
}
void isr_99(){
    asm volatile("iretq");
}
void isr_100(){
    asm volatile("iretq");
}
void isr_101(){
    asm volatile("iretq");
}
void isr_102(){
    asm volatile("iretq");
}
void isr_103(){
    asm volatile("iretq");
}
void isr_104(){
    asm volatile("iretq");
}
void isr_105(){
    asm volatile("iretq");
}
void isr_106(){
    asm volatile("iretq");
}
void isr_107(){
    asm volatile("iretq");
}
void isr_108(){
    asm volatile("iretq");
}
void isr_109(){
    asm volatile("iretq");
}
void isr_110(){
    asm volatile("iretq");
}
void isr_111(){
    asm volatile("iretq");
}
void isr_112(){
    asm volatile("iretq");
}
void isr_113(){
    asm volatile("iretq");
}
void isr_114(){
    asm volatile("iretq");
}
void isr_115(){
    asm volatile("iretq");
}
void isr_116(){
    asm volatile("iretq");
}
void isr_117(){
    asm volatile("iretq");
}
void isr_118(){
    asm volatile("iretq");
}
void isr_119(){
    asm volatile("iretq");
}
void isr_120(){
    asm volatile("iretq");
}
void isr_121(){
    asm volatile("iretq");
}
void isr_122(){
    asm volatile("iretq");
}
void isr_123(){
    asm volatile("iretq");
}
void isr_124(){
    asm volatile("iretq");
}
void isr_125(){
    asm volatile("iretq");
}
void isr_126(){
    asm volatile("iretq");
}
void isr_127(){
    asm volatile("iretq");
}
void isr_128(){
    asm volatile("iretq");
}
void isr_129(){
    asm volatile("iretq");
}
void isr_130(){
    asm volatile("iretq");
}
void isr_131(){
    asm volatile("iretq");
}
void isr_132(){
    asm volatile("iretq");
}
void isr_133(){
    asm volatile("iretq");
}
void isr_134(){
    asm volatile("iretq");
}
void isr_135(){
    asm volatile("iretq");
}
void isr_136(){
    asm volatile("iretq");
}
void isr_137(){
    asm volatile("iretq");
}
void isr_138(){
    asm volatile("iretq");
}
void isr_139(){
    asm volatile("iretq");
}
void isr_140(){
    asm volatile("iretq");
}
void isr_141(){
    asm volatile("iretq");
}
void isr_142(){
    asm volatile("iretq");
}
void isr_143(){
    asm volatile("iretq");
}
void isr_144(){
    asm volatile("iretq");
}
void isr_145(){
    asm volatile("iretq");
}
void isr_146(){
    asm volatile("iretq");
}
void isr_147(){
    asm volatile("iretq");
}
void isr_148(){
    asm volatile("iretq");
}
void isr_149(){
    asm volatile("iretq");
}
void isr_150(){
    asm volatile("iretq");
}
void isr_151(){
    asm volatile("iretq");
}
void isr_152(){
    asm volatile("iretq");
}
void isr_153(){
    asm volatile("iretq");
}
void isr_154(){
    asm volatile("iretq");
}
void isr_155(){
    asm volatile("iretq");
}
void isr_156(){
    asm volatile("iretq");
}
void isr_157(){
    asm volatile("iretq");
}
void isr_158(){
    asm volatile("iretq");
}
void isr_159(){
    asm volatile("iretq");
}
void isr_160(){
    asm volatile("iretq");
}
void isr_161(){
    asm volatile("iretq");
}
void isr_162(){
    asm volatile("iretq");
}
void isr_163(){
    asm volatile("iretq");
}
void isr_164(){
    asm volatile("iretq");
}
void isr_165(){
    asm volatile("iretq");
}
void isr_166(){
    asm volatile("iretq");
}
void isr_167(){
    asm volatile("iretq");
}
void isr_168(){
    asm volatile("iretq");
}
void isr_169(){
    asm volatile("iretq");
}
void isr_170(){
    asm volatile("iretq");
}
void isr_171(){
    asm volatile("iretq");
}
void isr_172(){
    asm volatile("iretq");
}
void isr_173(){
    asm volatile("iretq");
}
void isr_174(){
    asm volatile("iretq");
}
void isr_175(){
    asm volatile("iretq");
}
void isr_176(){
    asm volatile("iretq");
}
void isr_177(){
    asm volatile("iretq");
}
void isr_178(){
    asm volatile("iretq");
}
void isr_179(){
    asm volatile("iretq");
}
void isr_180(){
    asm volatile("iretq");
}
void isr_181(){
    asm volatile("iretq");
}
void isr_182(){
    asm volatile("iretq");
}
void isr_183(){
    asm volatile("iretq");
}
void isr_184(){
    asm volatile("iretq");
}
void isr_185(){
    asm volatile("iretq");
}
void isr_186(){
    asm volatile("iretq");
}
void isr_187(){
    asm volatile("iretq");
}
void isr_188(){
    asm volatile("iretq");
}
void isr_189(){
    asm volatile("iretq");
}
void isr_190(){
    asm volatile("iretq");
}
void isr_191(){
    asm volatile("iretq");
}
void isr_192(){
    asm volatile("iretq");
}
void isr_193(){
    asm volatile("iretq");
}
void isr_194(){
    asm volatile("iretq");
}
void isr_195(){
    asm volatile("iretq");
}
void isr_196(){
    asm volatile("iretq");
}
void isr_197(){
    asm volatile("iretq");
}
void isr_198(){
    asm volatile("iretq");
}
void isr_199(){
    asm volatile("iretq");
}
void isr_200(){
    asm volatile("iretq");
}
void isr_201(){
    asm volatile("iretq");
}
void isr_202(){
    asm volatile("iretq");
}
void isr_203(){
    asm volatile("iretq");
}
void isr_204(){
    asm volatile("iretq");
}
void isr_205(){
    asm volatile("iretq");
}
void isr_206(){
    asm volatile("iretq");
}
void isr_207(){
    asm volatile("iretq");
}
void isr_208(){
    asm volatile("iretq");
}
void isr_209(){
    asm volatile("iretq");
}
void isr_210(){
    asm volatile("iretq");
}
void isr_211(){
    asm volatile("iretq");
}
void isr_212(){
    asm volatile("iretq");
}
void isr_213(){
    asm volatile("iretq");
}
void isr_214(){
    asm volatile("iretq");
}
void isr_215(){
    asm volatile("iretq");
}
void isr_216(){
    asm volatile("iretq");
}
void isr_217(){
    asm volatile("iretq");
}
void isr_218(){
    asm volatile("iretq");
}
void isr_219(){
    asm volatile("iretq");
}
void isr_220(){
    asm volatile("iretq");
}
void isr_221(){
    asm volatile("iretq");
}
void isr_222(){
    asm volatile("iretq");
}
void isr_223(){
    asm volatile("iretq");
}
void isr_224(){
    asm volatile("iretq");
}
void isr_225(){
    asm volatile("iretq");
}
void isr_226(){
    asm volatile("iretq");
}
void isr_227(){
    asm volatile("iretq");
}
void isr_228(){
    asm volatile("iretq");
}
void isr_229(){
    asm volatile("iretq");
}
void isr_230(){
    asm volatile("iretq");
}
void isr_231(){
    asm volatile("iretq");
}
void isr_232(){
    asm volatile("iretq");
}
void isr_233(){
    asm volatile("iretq");
}
void isr_234(){
    asm volatile("iretq");
}
void isr_235(){
    asm volatile("iretq");
}
void isr_236(){
    asm volatile("iretq");
}
void isr_237(){
    asm volatile("iretq");
}
void isr_238(){
    asm volatile("iretq");
}
void isr_239(){
    asm volatile("iretq");
}
void isr_240(){
    asm volatile("iretq");
}
void isr_241(){
    asm volatile("iretq");
}
void isr_242(){
    asm volatile("iretq");
}
void isr_243(){
    asm volatile("iretq");
}
void isr_244(){
    asm volatile("iretq");
}
void isr_245(){
    asm volatile("iretq");
}
void isr_246(){
    asm volatile("iretq");
}
void isr_247(){
    asm volatile("iretq");
}
void isr_248(){
    asm volatile("iretq");
}
void isr_249(){
    asm volatile("iretq");
}
void isr_250(){
    asm volatile("iretq");
}
void isr_251(){
    asm volatile("iretq");
}
void isr_252(){
    asm volatile("iretq");
}
void isr_253(){
    asm volatile("iretq");
}
void isr_254(){
    asm volatile("iretq");
}
void isr_255(){
    asm volatile("iretq");
}








