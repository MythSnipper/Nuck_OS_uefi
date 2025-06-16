from PIL import Image
import struct

# Configuration
WIDTH = 200
HEIGHT = 200
INPUT_IMAGE = "data/logo/logo.bmp"
OUTPUT = "data/logo.nvideo"

with open(OUTPUT, "wb") as out:
    # Write header: format (1), width, height, frame count (1)
    out.write(struct.pack("<IIII", 1, WIDTH, HEIGHT, 1))

    # Load and process image
    img = Image.open(INPUT_IMAGE).convert("RGB")
    if img.size != (WIDTH, HEIGHT):
        img = img.resize((WIDTH, HEIGHT), Image.LANCZOS)

    # Convert RGB to BGR
    r, g, b = img.split()
    bgr_img = Image.merge("RGB", (b, g, r))

    # Write raw BGR data
    out.write(bgr_img.tobytes())
