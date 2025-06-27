#define STB_IMAGE_IMPLEMENTATION
#include "../include/external/stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <errno.h>

// Compare function for qsort (lexicographical)
static int cmp_filenames(const void *a, const void *b) {
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <frame_dir> <output_file.nvideo> <format_number>\n", argv[0]);
        fprintf(stderr, "Format 0 = grayscale (1-bit), format 1 = RGB (24-bit)\n");
        return 1;
    }

    const char *FRAME_DIR = argv[1];
    const char *OUTPUT_PATH = argv[2];
    int format = atoi(argv[3]);

    if (format != 0 && format != 1) {
        fprintf(stderr, "Unsupported format number: %d. Use 0 or 1.\n", format);
        return 1;
    }

    DIR *dir = opendir(FRAME_DIR);
    if (!dir) {
        fprintf(stderr, "Failed to open directory '%s': %s\n", FRAME_DIR, strerror(errno));
        return 1;
    }

    char **files = NULL;
    size_t count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        size_t len = strlen(entry->d_name);
        if (len > 4 && strcmp(entry->d_name + len - 4, ".bmp") == 0) {
            files = realloc(files, (count + 1) * sizeof(char *));
            files[count] = strdup(entry->d_name);
            count++;
        }
    }
    closedir(dir);

    if (count == 0) {
        fprintf(stderr, "No .bmp files found in '%s'\n", FRAME_DIR);
        return 1;
    }

    qsort(files, count, sizeof(char *), cmp_filenames);

    // Load first image to determine dimensions
    char first_path[4096];
    snprintf(first_path, sizeof(first_path), "%s/%s", FRAME_DIR, files[0]);
    int width, height, channels;
    unsigned char *first_img = stbi_load(first_path, &width, &height, &channels, 3);
    if (!first_img) {
        fprintf(stderr, "Failed to load first image '%s': %s\n", first_path, stbi_failure_reason());
        return 1;
    }
    stbi_image_free(first_img);
    printf("Using format %d. Frame size: %dx%d\n", format, width, height);

    FILE *out = fopen(OUTPUT_PATH, "wb");
    if (!out) {
        fprintf(stderr, "Failed to open output '%s': %s\n", OUTPUT_PATH, strerror(errno));
        return 1;
    }

    uint32_t header[4] = {(uint32_t)format, (uint32_t)width, (uint32_t)height, (uint32_t)count};
    fwrite(header, sizeof(header), 1, out);

    for (size_t i = 0; i < count; i++) {
        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", FRAME_DIR, files[i]);
        int w, h, c;
        unsigned char *img = stbi_load(path, &w, &h, &c, 3);
        if (!img) {
            fprintf(stderr, "Failed to load frame '%s': %s\n", path, stbi_failure_reason());
            fclose(out);
            return 1;
        }
        if (w != width || h != height) {
            fprintf(stderr, "Frame '%s' has size %dx%d, expected %dx%d\n", path, w, h, width, height);
            stbi_image_free(img);
            fclose(out);
            return 1;
        }

        if (format == 0) {
            // Format 0: 1-bit grayscale
            uint8_t byte = 0;
            int bit_count = 0;
            for (int p = 0; p < w * h; p++) {
                unsigned char r = img[3*p + 0];
                unsigned char g = img[3*p + 1];
                unsigned char b = img[3*p + 2];
                unsigned char gray = (r + g + b) / 3;
                int bit = (gray >= 128) ? 1 : 0;
                byte = (uint8_t)((byte << 1) | bit);
                bit_count++;
                if (bit_count == 8) {
                    fputc(byte, out);
                    byte = 0;
                    bit_count = 0;
                }
            }
            if (bit_count > 0) {
                byte <<= (8 - bit_count);
                fputc(byte, out);
            }
        } else {
            // Format 1: Raw RGB (3 bytes per pixel)
            fwrite(img, 3, w * h, out);
        }

        stbi_image_free(img);
    }

    fclose(out);
    printf("Wrote %zu frames to '%s'\n", count, OUTPUT_PATH);

    for (size_t i = 0; i < count; i++) free(files[i]);
    free(files);

    return 0;
}
