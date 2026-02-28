#ifndef MATERIAL_PART_H
#define MATERIAL_PART_H

#include "image/pixel.h"
#include "material/color.h"

typedef struct part_variant_frame {
    color_t *data;
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
} part_variant_frame_t;

typedef struct part_variant {
    string_view_t name;
    colorchn_t *colorchn;
    struct {
        part_variant_frame_t *entries;
        uint8_t length;
    } frames;
} part_variant_t;

FORCE_INLINE void part_variant_frame_init(part_variant_frame_t *frame, uint8_t x, uint8_t y, uint8_t width, uint8_t height);
FORCE_INLINE void part_variant_frame_set_from(part_variant_frame_t *dest, part_variant_frame_t *src);
void part_variant_frame_set_new(part_variant_frame_t *frame);

void part_variant_init(part_variant_t *part_variant, uint8_t total_frames);
void part_variant_set_name(part_variant_t *part_variant, const string_view_t *name);
colorchn_t *part_variant_alloc_colorchn(part_variant_t *part_variant);
FORCE_INLINE part_variant_frame_t *part_variant_alloc_frame(part_variant_t *part_variant);

FORCE_INLINE void part_variant_frame_init(part_variant_frame_t *frame, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    frame->x = x;
    frame->y = y;
    frame->width = width;
    frame->height = height;
}

FORCE_INLINE void part_variant_frame_set_from(part_variant_frame_t *dest, part_variant_frame_t *src) {
    dest->data = src->data;
}

FORCE_INLINE part_variant_frame_t *part_variant_alloc_frame(part_variant_t *part_variant) {
    return part_variant->frames.entries + part_variant->frames.length++;
}

#endif /* MATERIAL_PART_H */ 