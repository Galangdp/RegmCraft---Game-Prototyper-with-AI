#ifndef IMAGE_PIXELS_ARRAY_H
#define IMAGE_PIXELS_ARRAY_H

#include "image/pixel.h"
#include "utils/memory.h"

typedef struct pixels_array {
    pixels_rgba_t *values;
    size_t length;
    size_t capacity;
} pixels_array_t;

void pixels_array_init(pixels_array_t *array);
pixels_rgba_t *pixels_array_alloc(pixels_array_t *array);
void pixels_array_deinit(pixels_array_t *array);

#endif /* IMAGE_PIXELS_ARRAY_H */
