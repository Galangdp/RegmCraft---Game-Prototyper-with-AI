#ifndef IMAGE_IMAGE_H
#define IMAGE_IMAGE_H

#include "image/pixel.h"
#include "utils/arena.h"
#include "utils/crc32.h"
#include "utils/file.h"
#include "utils/utils.h"
#include "image/zlib.h"

#define IMAGE_DERR_FNPF             -1      /* File not png file */
#define IMAGE_DERR_RDPF             -2      /* Failed to read png file */
#define IMAGE_DERR_PFTS             -3      /* Failed to read png file */
#define IMAGE_DERR_PFDC             -4      /* Png file corrupt */
#define IMAGE_DERR_PFNS             -5      /* Png format not supported */

#define IMAGE_EERR_BFOV             -6      /* Buffer overflow */
#define IMAGE_EERR_IENP             -7      /* Invalid encoding png file */

void image_ctx_set_arena(arena_t *arena);
void image_ctx_reset_arena();

status_t image_decode(const uint8_t *buffer, size_t buffer_length, pixels_rgba_t *out);
status_t image_encode_bound(const pixels_rgba_t *buffer, size_t *out_length);
status_t image_encode(const pixels_rgba_t *buffer, uint8_t *out, size_t *out_length);

#ifdef RC_DEBUG
status_t image_write_ppm(file_t *file, const pixels_rgba_t *buffer);
#endif

#endif /* IMAGE_IMAGE_H */ 