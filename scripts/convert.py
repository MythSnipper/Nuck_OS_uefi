from PIL import Image
import os
import struct

# Configuration
WIDTH = 640
HEIGHT = 360
FPS = 3
SCALE = 2
FRAME_DIR = "data/frames"
OUTPUT = "data/video.nvideo"

frame_files = sorted(f for f in os.listdir(FRAME_DIR) if f.endswith(".bmp"))
frame_count = len(frame_files)

with open(OUTPUT, "wb") as out:
    # Write header
    out.write(struct.pack("<IIIII", WIDTH, HEIGHT, frame_count, FPS, SCALE))

    for fname in frame_files:
        path = os.path.join(FRAME_DIR, fname)

        img = Image.open(path).convert("RGB")
        if img.size != (WIDTH, HEIGHT):
            img = img.resize((WIDTH, HEIGHT), Image.LANCZOS)

        # Efficiently convert RGB to BGR using split and join
        r, g, b = img.split()
        bgr_img = Image.merge("RGB", (b, g, r))

        out.write(bgr_img.tobytes())


