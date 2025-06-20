from PIL import Image
import os
import struct

# Configuration
JPG = "data/bad-apple-source/frames-bad-apple/out0001.jpg"  # Path to reference JPG file for width and height
FRAME_DIR = "data/frames"
OUTPUT = "data/video.nvideo"

def rgb_to_bw_bits(img):
    img = img.convert("L")  # grayscale
    img = img.point(lambda x: 1 if x >= 128 else 0)  # threshold
    bits = img.getdata()

    bitstream = bytearray()
    byte = 0
    count = 0

    for bit in bits:
        byte = (byte << 1) | bit
        count += 1
        if count == 8:
            bitstream.append(byte)
            byte = 0
            count = 0

    if count > 0:
        byte <<= (8 - count)
        bitstream.append(byte)

    return bitstream

# Get WIDTH and HEIGHT from JPG
with Image.open(JPG) as ref_img:
    WIDTH, HEIGHT = ref_img.size
print(f"{WIDTH}x{HEIGHT}")
frame_files = sorted(f for f in os.listdir(FRAME_DIR) if f.endswith(".bmp"))

with open(OUTPUT, "wb") as out:
    # Write header: width and height only
    out.write(struct.pack("<IIII", 0, WIDTH, HEIGHT, len(frame_files)))

    for fname in frame_files:
        path = os.path.join(FRAME_DIR, fname)
        img = Image.open(path)

        if img.size != (WIDTH, HEIGHT):
            img = img.resize((WIDTH, HEIGHT), Image.LANCZOS)

        bw_data = rgb_to_bw_bits(img)
        out.write(bw_data)
