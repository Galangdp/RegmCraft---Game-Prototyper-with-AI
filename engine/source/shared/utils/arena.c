#include "utils/arena.h"

#define ARENA_DEFCAP        8
#define ARENA_ELEMSIZE      sizeof(arena_block_t)

FORCE_INLINE void arena_block_init(arena_block_t *block, size_t block_capacity) {
    block->ptr = memory_alloc(block_capacity);
    block->length = 0;
}

FORCE_INLINE void arena_block_deinit(arena_block_t *block) {
    memory_free(block->ptr);
}

FORCE_INLINE void arena_resize(arena_t *arena) {
    if (arena->length >= arena->capacity) {
        arena->capacity <<= 1;
        arena->blocks = memory_realloc(arena->blocks, arena->capacity * ARENA_ELEMSIZE);
    }
}

void arena_init(arena_t *arena, size_t block_capacity) {
    arena->blocks = memory_alloc(ARENA_DEFCAP * ARENA_ELEMSIZE);
    arena->length = 1;
    arena->capacity = ARENA_DEFCAP;
    arena->block_capacity = block_capacity;

    arena_block_init(arena->blocks, block_capacity);
}


void arena_deinit(arena_t *arena) {
    for (size_t i = 0; i < arena->length; i++) {
        arena_block_deinit(arena->blocks + i);
    }
    memory_free(arena->blocks);
}

void arena_reset(arena_t *arena) {
    for (size_t i = 1; i < arena->length; i++) {
        arena_block_deinit(arena->blocks + i);
    }
    arena->blocks[0].length = 0;
}

void *arena_alloc(arena_t *arena, size_t size) {
    arena_block_t *block = arena->blocks + arena->length - 1;

    if (block->length + size >= arena->block_capacity) {
        block = NULL;
    
        for (size_t i = 0, length = arena->length - 1; i < length; i++) {
            arena_block_t *block_check = arena->blocks + i;
            if (block_check->length + size < arena->block_capacity) {
                block = block_check;
                break;
            }
        }
    
        if (block == NULL) {
            arena_resize(arena);
            arena_block_init(arena->blocks + arena->length, arena->block_capacity);
            block = arena->blocks + arena->length++;
        }
    }

    void *ptr = block->ptr + block->length;
    block->length += size;
    return ptr;
}