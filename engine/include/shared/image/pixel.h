#ifndef IMAGE_PIXEL_H
#define IMAGE_PIXEL_H

#include "utils/macro.h"
#include "utils/memory.h"

typedef struct pixel_rgba {
    union {
        uint8_t red;
        uint8_t r;
        uint8_t hue;
        uint8_t h;
    };
    union {
        uint8_t green;
        uint8_t g;
        uint8_t saturation;
        uint8_t s;
    };
    union {
        uint8_t blue;
        uint8_t b;
        uint8_t lightness;
        uint8_t l;
    };
    union {
        uint8_t alpha;
        uint8_t a;
    };
} pixel_rgba_t;

typedef struct pixels_rgba {
    pixel_rgba_t *values;
    size_t width;
    size_t height;
} pixels_rgba_t;

FORCE_INLINE void pixels_rgba_sub(pixels_rgba_t *dest, const pixels_rgba_t *src, size_t x, size_t y, size_t width, size_t height);
FORCE_INLINE void pixels_rgba_deinit(pixels_rgba_t *pixels);

FORCE_INLINE void pixels_rgba_sub(pixels_rgba_t *dest, const pixels_rgba_t *src, size_t x, size_t y, size_t width, size_t height) {
    size_t col_width = width * sizeof(pixel_rgba_t);

    dest->values = memory_alloc(col_width * height);
    dest->width = width;
    dest->height = height;

    size_t src_offset = y * src->width + x;
    size_t dest_offset = 0;

    for (size_t max_y = y + height; y < max_y; y++) {
        memcpy(dest->values + dest_offset, src->values + src_offset, col_width);

        src_offset += src->width;
        dest_offset += width;
    }
}

FORCE_INLINE void pixels_rgba_deinit(pixels_rgba_t *pixels) {
    memory_free(pixels->values);
}

#endif /* IMAGE_PIXEL_H */ 