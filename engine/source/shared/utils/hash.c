#include "utils/hash.h"

#define FNV1A_PRIME_BASE    0xCBF29CE484222325ULL
#define FNV1A_PRIME         0x100000001B3ULL
#define FNV1A_PRIME_AVA1    0xFF51AFD7ED558CCDULL
#define FNV1A_PRIME_AVA2    0xC4CEB9FE1A85EC53ULL

uint64_t fnv1amix(const uint8_t *buffer, size_t length) {
    uint64_t hash = FNV1A_PRIME_BASE;

    for (size_t i = 0; i < length; i++) {
        hash *= FNV1A_PRIME;
        hash ^= buffer[i];
    }

    // Murmurhash3 64 avalanche
    hash ^= hash >> 33;
    hash *= FNV1A_PRIME_AVA1;
    hash ^= hash >> 33;
    hash *= FNV1A_PRIME_AVA2;
    hash ^= hash >> 33;

    return hash;
}

void fnv1amix_stream_start(uint64_t *hash) {
    *hash = FNV1A_PRIME_BASE;
}

void fnv1amix_stream_consume(uint64_t *hash, uint8_t ch) {
    *hash *= FNV1A_PRIME;
    *hash ^= ch;
}

void fnv1amix_stream_end(uint64_t *hash) {
    *hash ^= *hash >> 33;
    *hash *= FNV1A_PRIME_AVA1;
    *hash ^= *hash >> 33;
    *hash *= FNV1A_PRIME_AVA2;
    *hash ^= *hash >> 33;
}