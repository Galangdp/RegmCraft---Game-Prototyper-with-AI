#include "image/pixels_array.h"

#define PIXELS_ARRAY_DEFCAP         64
#define PIXELS_ARRAY_ELEMSIZE       sizeof(pixels_rgba_t)

void pixels_array_init(pixels_array_t *array) {
    array->values = memory_alloc(PIXELS_ARRAY_DEFCAP * PIXELS_ARRAY_ELEMSIZE);
    array->length = 0;
    array->capacity = PIXELS_ARRAY_DEFCAP;
}

pixels_rgba_t *pixels_array_alloc(pixels_array_t *array) {
    if (array->length >= array->capacity) {
        array->capacity <<= 1;
        array->values = memory_realloc(array->values, array->capacity * PIXELS_ARRAY_ELEMSIZE);
    }

    return array->values + array->length++;
}

void pixels_array_deinit(pixels_array_t *array) {
    memory_free(array->values);
}
