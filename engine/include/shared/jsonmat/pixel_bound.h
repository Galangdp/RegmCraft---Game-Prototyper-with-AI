#ifndef JSONMAT_PIXEL_BOUND_H
#define JSONMAT_PIXEL_BOUND_H

#include "image/pixel.h"
#include "utils/hash.h"

typedef struct pixel_bound {
    pixel_rgba_t *data;
    uint64_t hash;
    uint8_t width;
    uint8_t height;
    uint8_t x;
    uint8_t y;
    bool_t is_own;
    uint8_t owner_id;
} pixel_bound_t;

FORCE_INLINE void pixel_bound_init(pixel_bound_t *bound, const pixels_rgba_t *pixels);
FORCE_INLINE void pixel_bound_deinit(pixel_bound_t *bound);

FORCE_INLINE void pixel_bound_init(pixel_bound_t *bound, const pixels_rgba_t *pixels) {
    bound->x = pixels->width;
    bound->y = pixels->height;
    bound->width = 0;
    bound->height = 0;

    size_t i = 0;

    for (uint8_t y = 0; y < pixels->height; y++) {
        for (uint8_t x = 0; x < pixels->width; x++) {
            pixel_rgba_t *pixel = pixels->values + i++;
            
            if (pixel->alpha == 0) {
                continue;
            }

            bound->x = bound->x > x ? x : bound->x;
            bound->y = bound->y > y ? y : bound->y;
            bound->width = bound->width < x ? x : bound->width;
            bound->height = bound->height < y ? y : bound->height;
        }
    }

    bound->width -= bound->x - 1;
    bound->height -= bound->y - 1;
    bound->is_own = TRUE;
    bound->owner_id = 0;

    size_t col_width = bound->width * sizeof(pixel_rgba_t);
    uint8_t width_pad = pixels->width - bound->width;

    bound->data = memory_alloc(col_width * bound->height);

    pixel_rgba_t *pixel_src = pixels->values + (bound->y * pixels->width + bound->x);
    pixel_rgba_t *pixel_dest = bound->data;

    fnv1amix_stream_start(&bound->hash);

    for (uint8_t y = bound->y, max_y = bound->y + bound->height; y < max_y; y++) {
        for (uint8_t x = bound->x, max_x = bound->x + bound->width; x < max_x; x++) {
            fnv1amix_stream_consume(&bound->hash, pixel_src->red);
            fnv1amix_stream_consume(&bound->hash, pixel_src->green);
            fnv1amix_stream_consume(&bound->hash, pixel_src->blue);
            
            pixel_dest->red = pixel_src->red;
            pixel_dest->green = pixel_src->green;
            pixel_dest->blue = pixel_src->blue;
            pixel_dest->alpha = pixel_src->alpha;

            pixel_src++;
            pixel_dest++;
        }
        pixel_src += width_pad;
    }

    fnv1amix_stream_end(&bound->hash);
}

FORCE_INLINE void pixel_bound_deinit(pixel_bound_t *bound) {
    memory_free(bound->data);
}

#endif /* JSONMAT_PIXEL_BOUND_H */ 