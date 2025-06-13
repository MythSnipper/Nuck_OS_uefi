#include "convert-bad-apple.h"

int ends_with(const char *str, const char *suffix) {
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    return lenstr >= lensuffix && strcmp(str + lenstr - lensuffix, suffix) == 0;
}

int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void write_uint32_le(FILE *f, uint32_t val) {
    uint8_t b[4] = { val & 0xFF, (val >> 8) & 0xFF, (val >> 16) & 0xFF, (val >> 24) & 0xFF };
    fwrite(b, 1, 4, f);
}

uint8_t grayscale_threshold(uint8_t r, uint8_t g, uint8_t b) {
    return ((r + g + b) / 3) >= 128 ? 1 : 0;
}

int main() {
    int ref_width, ref_height, ref_channels;
    unsigned char *ref_img = stbi_load(JPG, &ref_width, &ref_height, &ref_channels, 3);
    if (!ref_img) {
        fprintf(stderr, "Failed to load reference JPG: %s\n", JPG);
        return 1;
    }
    stbi_image_free(ref_img);
    printf("Using reference resolution: %dx%d\n", ref_width, ref_height);

    // Collect BMP filenames
    DIR *dir = opendir(FRAME_DIR);
    if (!dir) {
        perror("Could not open frame directory");
        return 1;
    }

    char *files[10000];
    int file_count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (ends_with(entry->d_name, ".bmp")) {
            files[file_count++] = strdup(entry->d_name);
        }
    }
    closedir(dir);
    qsort(files, file_count, sizeof(char *), compare);

    FILE *out = fopen(OUTPUT_FILE, "wb");
    if (!out) {
        perror("Could not open output file");
        return 1;
    }

    write_uint32_le(out, ref_width);
    write_uint32_le(out, ref_height);

    for (int i = 0; i < file_count; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", FRAME_DIR, files[i]);

        int w, h, c;
        unsigned char *img = stbi_load(path, &w, &h, &c, 3);
        if (!img) {
            fprintf(stderr, "Failed to load: %s\n", path);
            continue;
        }

        // Resize if needed (not supported by stb_image — just warn)
        if (w != ref_width || h != ref_height) {
            fprintf(stderr, "Skipping %s due to mismatched resolution (%dx%d)\n", path, w, h);
            stbi_image_free(img);
            continue;
        }

        uint8_t byte = 0;
        int bit_count = 0;
        for (int i = 0; i < w * h; i++) {
            uint8_t r = img[i * 3 + 0];
            uint8_t g = img[i * 3 + 1];
            uint8_t b = img[i * 3 + 2];

            byte = (byte << 1) | grayscale_threshold(r, g, b);
            bit_count++;
            if (bit_count == 8) {
                fputc(byte, out);
                bit_count = 0;
                byte = 0;
            }
        }
        if (bit_count > 0) {
            byte <<= (8 - bit_count);
            fputc(byte, out);
        }

        stbi_image_free(img);
        free(files[i]);
    }

    fclose(out);
    printf("Done. Output written to %s\n", OUTPUT_FILE);
    return 0;
}
