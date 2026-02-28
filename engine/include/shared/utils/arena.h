#ifndef UTILS_ARENA_H
#define UTILS_ARENA_H

#include "utils/memory.h"

#define ARENA_SMALL_CAP         0x4000
#define ARENA_BIG_CAP           0x400000

typedef struct arena_block {
    void *ptr;
    size_t length;
} arena_block_t;

typedef struct arena {
    arena_block_t *blocks;
    size_t length;
    size_t capacity;
    size_t block_capacity;
} arena_t;

void arena_init(arena_t *arena, size_t block_capacity);
void arena_deinit(arena_t *arena);
void arena_reset(arena_t *arena);
void *arena_alloc(arena_t *arena, size_t size);

#endif /* UTILS_ARENA_H */ 