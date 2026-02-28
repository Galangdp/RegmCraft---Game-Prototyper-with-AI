#include "image/image.h"
#include "image/const.h"

arena_t *image_arena = NULL;

void image_ctx_set_arena(arena_t *arena) {
    image_arena = arena;
}

void image_ctx_reset_arena() {
    arena_reset(image_arena);
}

#ifdef RC_DEBUG
status_t image_write_ppm(file_t *file, const pixels_rgba_t *buffer) {
    size_t length = buffer->width * buffer->height;
    size_t capacity = length * (IMAGE_PNG_CHANNELS - 1);
    uint8_t *temp_buffer = memory_alloc(capacity + 50);
    size_t bytes_written = snprintf((char *) temp_buffer, 50, "P6\n%lu %lu\n255\n", buffer->width, buffer->height);
    uint8_t *new_temp_buffer = temp_buffer + bytes_written;

    for (size_t i = 0; i < length; i++) {
        memcpy(new_temp_buffer, buffer->values + i, 3);
        new_temp_buffer += 3;
    }

    return file_write_byte(file, temp_buffer, capacity + bytes_written);
}
#endif