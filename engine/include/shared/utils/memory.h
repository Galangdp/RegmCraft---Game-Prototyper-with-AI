#ifndef UTILS_MEMORY_H
#define UTILS_MEMORY_H

#include <string.h>
#include "utils/types.h"
#include "utils/macro.h"

FORCE_INLINE void *memory_alloc(size_t size);
FORCE_INLINE void *memory_ialloc(size_t size, uint8_t value);
FORCE_INLINE void *memory_zalloc(size_t size);
FORCE_INLINE void *memory_realloc(void *ptr, size_t size);
FORCE_INLINE void memory_free(void *ptr);

FORCE_INLINE void *memory_alloc(size_t size) {
#ifdef RC_DEBUG
    void *ptr = malloc(size);
    if (ptr == NULL) {
        ASSERT(EXIT_FAILURE, "Can't allocating memory.");
    }
    return ptr;
#else
    return malloc(size);
#endif
}

FORCE_INLINE void *memory_ialloc(size_t size, uint8_t value) {
    void *ptr = malloc(size);
#ifdef RC_DEBUG
    if (ptr == NULL) {
        ASSERT(EXIT_FAILURE, "Can't allocating memory.");
    }
#endif
    memset(ptr, value, size);
    return ptr;
}

FORCE_INLINE void *memory_zalloc(size_t size) {
    void *ptr = malloc(size);
#ifdef RC_DEBUG
    if (ptr == NULL) {
        ASSERT(EXIT_FAILURE, "Can't allocating memory.");
    }
#endif
    memset(ptr, 0, size);
    return ptr;
}

FORCE_INLINE void *memory_realloc(void *ptr, size_t size) {
#ifdef RC_DEBUG
    if (ptr == NULL) {
        ASSERT(EXIT_FAILURE, "Try to reallocating NULL pointer.");
    }
    if (size == 0) {
        ASSERT(EXIT_FAILURE, "Try to reallocating 0 size.");
    }
    void *new_ptr = realloc(ptr, size);
    if (new_ptr == NULL) {
        ASSERT(EXIT_FAILURE, "Can't reallocating memory.");
    }
    return new_ptr;
#else
    return realloc(ptr, size);
#endif
}

FORCE_INLINE void memory_free(void *ptr) {
    free(ptr);
}

#endif /* UTILS_MEMORY_H */ 