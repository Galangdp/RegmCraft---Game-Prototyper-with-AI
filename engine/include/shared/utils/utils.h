#ifndef UTILS_UTILS_H
#define UTILS_UTILS_H

#include <string.h>
#include "utils/macro.h"
#include "utils/types.h"

#define BIT_BASE_MASK       0x01

FORCE_INLINE uint32_t ceil_pow2_32(uint32_t value);
FORCE_INLINE uint64_t ceil_pow2_64(uint64_t value);

FORCE_INLINE bool_t bit_cmpmask(uint8_t value, uint8_t mask);
FORCE_INLINE bool_t bit_mask(uint8_t value, uint8_t mask);
FORCE_INLINE bool_t bit_cmp(uint8_t value1, uint8_t value2, uint8_t mask);
FORCE_INLINE bool_t bit_ncmp(uint8_t value1, uint8_t value2, uint8_t mask);
FORCE_INLINE bool_t bit_zero(uint8_t value1, uint8_t mask);
FORCE_INLINE bool_t bit_nzero(uint8_t value1, uint8_t mask);

FORCE_INLINE uint16_t read_be16(const uint8_t *buffer, size_t index);
FORCE_INLINE uint32_t read_be32(const uint8_t *buffer, size_t index);
FORCE_INLINE uint64_t read_be64(const uint8_t *buffer, size_t index);
FORCE_INLINE uint16_t read_le16(const uint8_t *buffer, size_t index);
FORCE_INLINE uint32_t read_le32(const uint8_t *buffer, size_t index);
FORCE_INLINE uint64_t read_le64(const uint8_t *buffer, size_t index);

FORCE_INLINE void write_be16(uint8_t *buffer, size_t index, uint16_t value);
FORCE_INLINE void write_be32(uint8_t *buffer, size_t index, uint32_t value);
FORCE_INLINE void write_be64(uint8_t *buffer, size_t index, uint64_t value);
FORCE_INLINE void write_le16(uint8_t *buffer, size_t index, uint16_t value);
FORCE_INLINE void write_le32(uint8_t *buffer, size_t index, uint32_t value);
FORCE_INLINE void write_le64(uint8_t *buffer, size_t index, uint64_t value);

FORCE_INLINE uint32_t ceil_pow2_32(uint32_t value) {
#if defined(__clang__) || defined(__GNUC__)
    return value <= 1 ? 1 : (uint32_t) 1 << (32 - __builtin_clzll(value - 1));
#else
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;

    return value;
#endif
}

FORCE_INLINE uint64_t ceil_pow2_64(uint64_t value) {
#if defined(__clang__) || defined(__GNUC__)
    return value <= 1 ? 1 : (uint64_t) 1 << (64 - __builtin_clzll(value - 1));
#else
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    value++;

    return value;
#endif
}

FORCE_INLINE bool_t bit_cmpmask(uint8_t value, uint8_t mask) {
    return (value & mask) == mask;
}

FORCE_INLINE bool_t bit_mask(uint8_t value, uint8_t mask) {
    return value & mask;
}

FORCE_INLINE bool_t bit_cmp(uint8_t value1, uint8_t value2, uint8_t mask) {
    return (value1 & mask) == value2;
}

FORCE_INLINE bool_t bit_ncmp(uint8_t value1, uint8_t value2, uint8_t mask) {
    return (value1 & mask) != value2;
}

FORCE_INLINE bool_t bit_zero(uint8_t value1, uint8_t mask) {
    return (value1 & mask) == 0;
}

FORCE_INLINE bool_t bit_nzero(uint8_t value1, uint8_t mask) {
    return (value1 & mask) != 0;
}

FORCE_INLINE uint16_t read_be16(const uint8_t *buffer, size_t index) {
    uint16_t value;
    memcpy(&value, buffer + index, sizeof(uint16_t));
#ifdef RC_LITTLE_ENDIAN
    return __builtin_bswap16(value);
#else
    return value;
#endif
}

FORCE_INLINE uint32_t read_be32(const uint8_t *buffer, size_t index) {
    uint32_t value;
    memcpy(&value, buffer + index, sizeof(uint32_t));
#ifdef RC_LITTLE_ENDIAN
    return __builtin_bswap32(value);
#else
    return value;
#endif
}

FORCE_INLINE uint64_t read_be64(const uint8_t *buffer, size_t index) {
    uint64_t value;
    memcpy(&value, buffer + index, sizeof(uint64_t));
#ifdef RC_LITTLE_ENDIAN
    return __builtin_bswap64(value);
#else
    return value;
#endif
}

FORCE_INLINE uint16_t read_le16(const uint8_t *buffer, size_t index) {
    uint16_t value;
    memcpy(&value, buffer + index, sizeof(uint16_t));
#ifdef RC_BIG_ENDIAN
    return __builtin_bswap16(value);
#else
    return value;
#endif
}

FORCE_INLINE uint32_t read_le32(const uint8_t *buffer, size_t index) {
    uint32_t value;
    memcpy(&value, buffer + index, sizeof(uint32_t));
#ifdef RC_BIG_ENDIAN
    return __builtin_bswap32(value);
#else
    return value;
#endif
}

FORCE_INLINE uint64_t read_le64(const uint8_t *buffer, size_t index) {
    uint64_t value;
    memcpy(&value, buffer + index, sizeof(uint64_t));
#ifdef RC_BIG_ENDIAN
    return __builtin_bswap64(value);
#else
    return value;
#endif
}

FORCE_INLINE void write_be16(uint8_t *buffer, size_t index, uint16_t value) {
#ifdef RC_LITTLE_ENDIAN
    value = __builtin_bswap16(value);
#endif
    memcpy(buffer + index, &value, sizeof(uint16_t));
}

FORCE_INLINE void write_be32(uint8_t *buffer, size_t index, uint32_t value) {
#ifdef RC_LITTLE_ENDIAN
    value = __builtin_bswap32(value);
#endif
    memcpy(buffer + index, &value, sizeof(uint32_t));
}

FORCE_INLINE void write_be64(uint8_t *buffer, size_t index, uint64_t value) {
#ifdef RC_LITTLE_ENDIAN
    value = __builtin_bswap64(value);
#endif
    memcpy(buffer + index, &value, sizeof(uint64_t));
}

FORCE_INLINE void write_le16(uint8_t *buffer, size_t index, uint16_t value) {
#ifdef RC_BIG_ENDIAN
    value = __builtin_bswap16(value);
#endif
    memcpy(buffer + index, &value, sizeof(uint16_t));
}

FORCE_INLINE void write_le32(uint8_t *buffer, size_t index, uint32_t value) {
#ifdef RC_BIG_ENDIAN
    value = __builtin_bswap32(value);
#endif
    memcpy(buffer + index, &value, sizeof(uint32_t));
}

FORCE_INLINE void write_le64(uint8_t *buffer, size_t index, uint64_t value) {
#ifdef RC_BIG_ENDIAN
    value = __builtin_bswap64(value);
#endif
    memcpy(buffer + index, &value, sizeof(uint64_t));
}

#endif /* UTILS_UTILS_H */ 