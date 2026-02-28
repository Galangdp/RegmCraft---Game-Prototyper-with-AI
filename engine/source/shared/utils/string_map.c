#include "utils/string_map.h"

#define STRING_MAP_ELEMSIZE         sizeof(string_map_entry_t)

arena_t *string_map_arena = NULL;

void string_map_ctx_set_arena(arena_t *arena) {
    string_map_arena = arena;
}

void string_map_init(string_map_t *map, size_t capacity) {
    map->entries = arena_alloc(string_map_arena, capacity * STRING_MAP_ELEMSIZE);
    map->capacity = capacity;

    memset(map->entries, 0, capacity * STRING_MAP_ELEMSIZE);
}

void string_map_insert(string_map_t *map, const char *cstr, size_t length, uint16_t value) {
    uint64_t hash = fnv1amix((const uint8_t *) cstr, length);
    size_t max_index = map->capacity - 1;
    size_t index = hash & max_index;
    size_t probe = 1;

    string_map_entry_t *entry = map->entries + index;

    while (entry->is_used) {
        index = (index + probe * probe) & max_index;
        entry = entry + index;
        probe++;
    }

    entry->key.cstr = (char *) cstr;
    entry->key.length = length;
    entry->key.hash = hash;
    entry->value = (int32_t) value;
    entry->is_used = TRUE;
}

int32_t string_map_get(string_map_t *map, const char *cstr, size_t length) {
    uint64_t hash = fnv1amix((const uint8_t *) cstr, length);
    size_t max_index = map->capacity - 1;
    size_t index = hash & max_index;
    size_t probe = 1;

    string_map_entry_t *entry = map->entries + index;

    while (entry->is_used) {
        if (entry->key.hash == hash && entry->key.length == length && memcmp(cstr, entry->key.cstr, length) == 0) {
            return entry->value;
        }

        index = (index + probe * probe) & max_index;
        entry = entry + index;
        probe++;
    }

    return STRING_MAP_INVVAL;
}