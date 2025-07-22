
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../include/external/stb_image.h"

// Dynamic array of strings
typedef struct {
    char **items;
    size_t count, capacity;
} StringArray;

static void sa_init(StringArray *sa) {
    sa->items = NULL;
    sa->count = sa->capacity = 0;
}

static void sa_free(StringArray *sa) {
    for (size_t i = 0; i < sa->count; i++)
        free(sa->items[i]);
    free(sa->items);
    sa->items = NULL;
    sa->count = sa->capacity = 0;
}

static int sa_append(StringArray *sa, const char *s) {
    if (sa->count == sa->capacity) {
        size_t newcap = sa->capacity ? sa->capacity * 2 : 16;
        char **tmp = realloc(sa->items, newcap * sizeof(*tmp));
        if (!tmp) return -1;
        sa->items = tmp;
        sa->capacity = newcap;
    }
    sa->items[sa->count] = strdup(s);
    if (!sa->items[sa->count]) return -1;
    sa->count++;
    return 0;
}

// Compare for qsort
static int cmp_str(const void *a, const void *b) {
    return strcmp(*(char * const *)a, *(char * const *)b);
}

// Check filename extension
static int has_ext(const char *filename, const char *ext) {
    size_t n = strlen(filename), m = strlen(ext);
    return n >= m && strcmp(filename + n - m, ext) == 0;
}

// Process a single frame: load, convert, write to out (mirroring original logic)
static int process_file(const char *path, FILE *out,
                        int format, int width, int height) {
    int reqc = (format == 2) ? 4 : 3;
    int w, h, c;
    unsigned char *img = stbi_load(path, &w, &h, &c, reqc);
    if (!img) {
        fprintf(stderr, "Failed to load frame '%s': %s\n", path,
                stbi_failure_reason());
        return -1;
    }
    if (w != width || h != height) {
        fprintf(stderr, "Frame '%s' has size %dx%d, expected %dx%d\n",
                path, w, h, width, height);
        stbi_image_free(img);
        return -1;
    }

    size_t npix = (size_t)w * h;
    if (format == 0) {
        // 1-bit grayscale
        uint8_t byte = 0;
        int bit_count = 0;
        for (size_t p = 0; p < npix; p++) {
            unsigned char r = img[3*p + 0];
            unsigned char g = img[3*p + 1];
            unsigned char b = img[3*p + 2];
            unsigned char gray = (r + g + b) / 3;
            int bit = (gray >= 128);
            byte = (uint8_t)((byte << 1) | bit);
            bit_count++;
            if (bit_count == 8) {
                if (fputc(byte, out) == EOF) {
                    perror("fputc");
                    stbi_image_free(img);
                    return -1;
                }
                byte = 0;
                bit_count = 0;
            }
        }
        if (bit_count > 0) {
            byte <<= (8 - bit_count);
            if (fputc(byte, out) == EOF) {
                perror("fputc");
                stbi_image_free(img);
                return -1;
            }
        }
    }
    else if (format == 1) {
        // Raw RGB
        if (fwrite(img, 3, npix, out) != npix) {
            perror("fwrite");
            stbi_image_free(img);
            return -1;
        }
    }
    else {
        // ARGB from RGBA
        for (size_t p = 0; p < npix; p++) {
            unsigned char r = img[4*p + 0];
            unsigned char g = img[4*p + 1];
            unsigned char b = img[4*p + 2];
            unsigned char a = img[4*p + 3];
            if (fputc(a, out) == EOF ||
                fputc(r, out) == EOF ||
                fputc(g, out) == EOF ||
                fputc(b, out) == EOF) {
                perror("fputc");
                stbi_image_free(img);
                return -1;
            }
        }
    }

    stbi_image_free(img);
    return 0;
}

// Usage and exit
static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s <dir> <out.nvideo> <format> <ext>\n"
        "  format: 0=1-bit gray,1=RGB24,2=ARGB32 (PNG)\n"
        "  ext   : file extension, e.g. .bmp or .png\n",
        prog);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 5) usage(argv[0]);
    const char *dir = argv[1];
    const char *outp = argv[2];
    int format = atoi(argv[3]);
    const char *ext = argv[4];
    if (format < 0 || format > 2) usage(argv[0]);

    // Collect frame filenames
    StringArray sa;
    sa_init(&sa);
    DIR *d = opendir(dir);
    if (!d) { perror("opendir"); return EXIT_FAILURE; }
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        if (!has_ext(ent->d_name, ext))
            continue;
        size_t n = strlen(dir) + 1 + strlen(ent->d_name) + 1;
        char *path = malloc(n);
        snprintf(path, n, "%s/%s", dir, ent->d_name);
        struct stat st;
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
            sa_append(&sa, path);
        }
        free(path);
    }
    closedir(d);
    if (sa.count == 0) {
        fprintf(stderr, "No %s files in %s\n", ext, dir);
        return EXIT_FAILURE;
    }
    qsort(sa.items, sa.count, sizeof(char*), cmp_str);

    // Probe first frame
    int w, h, c;
    unsigned char *p0 = stbi_load(sa.items[0], &w, &h, &c, (format==2?4:3));
    if (!p0) {
        fprintf(stderr, "Failed to load '%s': %s\n",
                sa.items[0], stbi_failure_reason());
        return EXIT_FAILURE;
    }
    stbi_image_free(p0);

    // Open output and write header
    FILE *out = fopen(outp, "wb");
    if (!out) { perror("fopen"); return EXIT_FAILURE; }
    uint32_t hdr[4] = {(uint32_t)format, (uint32_t)w, (uint32_t)h, (uint32_t)sa.count};
    if (fwrite(hdr, sizeof(hdr), 1, out) != 1) {
        perror("fwrite header"); fclose(out); return EXIT_FAILURE;
    }

    // Process frames
    for (size_t i = 0; i < sa.count; i++) {
        if (process_file(sa.items[i], out, format, w, h) < 0) {
            fclose(out);
            sa_free(&sa);
            return EXIT_FAILURE;
        }
    }

    printf("Wrote %zu frames → %s\n", sa.count, outp);
    fclose(out);
    sa_free(&sa);
    return EXIT_SUCCESS;
}
