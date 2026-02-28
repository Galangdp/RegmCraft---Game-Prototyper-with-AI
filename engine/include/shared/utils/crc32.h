#ifndef UTILS_CRC32_H
#define UTILS_CRC32_H

#include "utils/types.h"
#include "utils/macro.h"

#ifndef CRC_OREF
void crc32_init();
uint32_t crc32(const uint8_t *buffer, size_t length);
#endif

#ifndef CRC_NOREF
void crc32_init_reflect();
uint32_t crc32_reflect(const uint8_t *buffer, size_t length);
#endif

#endif /* UTILS_CRC32_H */ 