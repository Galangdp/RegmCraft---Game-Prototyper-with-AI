#ifndef UTILS_HASH_H
#define UTILS_HASH_H

#include "utils/types.h"

uint64_t fnv1amix(const uint8_t *buffer, size_t length);

void fnv1amix_stream_start(uint64_t *hash);
void fnv1amix_stream_consume(uint64_t *hash, uint8_t ch);
void fnv1amix_stream_end(uint64_t *hash);

#endif /* UTILS_HASH_H */ 