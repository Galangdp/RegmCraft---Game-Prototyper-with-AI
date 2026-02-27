#ifndef UTILS_STRING_MAP_H
#define UTILS_STRING_MAP_H

#include "utils/arena.h"
#include "utils/utils.h"
#include "utils/string.h"

typedef struct string_map_entry {
    string_hash_t key;
    bool_t is_used;
    int32_t value;
} string_map_entry_t;

typedef struct string_map {
    string_map_entry_t *entries;
    size_t capacity;
} string_map_t;

#define STRING_MAP_INVVAL       -1

void string_map_ctx_set_arena(arena_t *arena);

void string_map_init(string_map_t *map, size_t capacity);
void string_map_insert(string_map_t *map, const char *cstr, size_t length, uint16_t value);
int32_t string_map_get(string_map_t *map, const char *cstr, size_t length);

#endif /* UTILS_STRING_MAP_H */ 