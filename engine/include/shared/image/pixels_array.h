#ifndef IMAGE_PIXELS_ARRAY_H
#define IMAGE_PIXELS_ARRAY_H

#include "image/pixel.h"
#include "utils/memory.h"

typedef struct pixels_array {
  pixels_rgba_t *values;
  size_t length;
  size_t capacity;
} pixels_array_t;

FORCE_INLINE void pixels_array_init(pixels_array_t *array) {
  array->values = NULL;
  array->length = 0;
  array->capacity = 0;
}

FORCE_INLINE pixels_rgba_t *pixels_array_alloc(pixels_array_t *array) {
  if (array->length >= array->capacity) {
    array->capacity = array->capacity == 0 ? 8 : array->capacity * 2;
    array->values =
        memory_realloc(array->values, array->capacity * sizeof(pixels_rgba_t));
  }
  return array->values + array->length++;
}

FORCE_INLINE void pixels_array_deinit(pixels_array_t *array) {
  memory_free(array->values);
  array->values = NULL;
  array->length = 0;
  array->capacity = 0;
}

#endif /* IMAGE_PIXELS_ARRAY_H */
