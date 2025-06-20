#include "convert-bad-apple.h"





// Fixed paths
static const char *REF_PATH   = "data/bad-apple-source/frames-bad-apple/out0001.jpg";
static const char *FRAME_DIR  = "data/frames";
static const char *OUTPUT_PATH= "data/video.nvideo";

// Compare function for qsort (lexicographical)
static int cmp_filenames(const void *a, const void *b) {
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb);
}

int main(void) {
    int width, height, channels;
    unsigned char *ref_img = stbi_load(REF_PATH, &width, &height, &channels, 3);
    if (!ref_img) {
        fprintf(stderr, "Failed to load reference image '%s': %s\n", REF_PATH, stbi_failure_reason());
        return 1;
    }
    stbi_image_free(ref_img);
    printf("Reference size: %dx%d\n", width, height);

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

    FILE *out = fopen(OUTPUT_PATH, "wb");
    if (!out) {
        fprintf(stderr, "Failed to open output '%s': %s\n", OUTPUT_PATH, strerror(errno));
        return 1;
    }

    uint32_t header[4] = {0, (uint32_t)width, (uint32_t)height, (uint32_t)count};
    fwrite(header, sizeof(header), 1, out);

    for (size_t i = 0; i < count; i++){
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
        stbi_image_free(img);
    }

    fclose(out);
    printf("Wrote %zu frames to '%s'\n", count, OUTPUT_PATH);

    for (size_t i = 0; i < count; i++) free(files[i]);
    free(files);

    return 0;
}