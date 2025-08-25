for i in range(256):
    print(f"        setIDTEntry(&IDT[{i}], CODE_SEG, (uint64_t)&isr_stub_{i}, 0b000, 0b10001110);")
