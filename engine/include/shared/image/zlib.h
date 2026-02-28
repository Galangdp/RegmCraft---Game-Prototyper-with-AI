#ifndef IMAGE_ZLIB_H
#define IMAGE_ZLIB_H

#include "utils/types.h"
#include "utils/arena.h"

status_t zlib_inflate(const uint8_t *idat, uint8_t *unfrgba, size_t idat_length, size_t unfrgba_length);

status_t zlib_deflate_bound(size_t unfrgba_length, size_t *idat_length);
status_t zlib_deflate(const uint8_t *unfrgba, uint8_t *idat, size_t unfrgba_length, size_t *idat_length);

#endif /* IMAGE_ZLIB_H */ 
