for i in range(256):
    print(f"setIDTEntry(&IDT[{i}], CODE_SEG, &isr_{i}, 0b000, 0b10001110);")
