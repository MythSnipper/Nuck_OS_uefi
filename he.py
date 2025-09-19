c = ""
for i in range(256):
    c += f"isr_stub_{i}, "
print(c)