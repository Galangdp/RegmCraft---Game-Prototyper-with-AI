#include "json/const.h"
#include "json/idmap.h"

#define JSON_IDMAP_ELEMSIZE     sizeof(json_idmap_entry_t)

FORCE_INLINE void json_idmap_insert(json_idmap_t *idmap, const char *cstr, size_t length, uint64_t hash, json_id_t id) {
    size_t max_index = idmap->capacity - 1;
    size_t index = hash & max_index;
    size_t probe = 1;

    json_idmap_entry_t *entry = idmap->entries + index;

    while (entry->key.hash != 0) {
        index = (index + probe * probe) & max_index;
        entry = idmap->entries + index;
        probe++;
    }
    
    entry->key.cstr = (char *) cstr;
    entry->key.length = length;
    entry->key.hash = hash;
    entry->id = id;
}

void json_idmap_init(json_idmap_t *idmap) {
    idmap->capacity = ceil_pow2_32(JSON_ID_UNKNOWN - JSON_ID_NULL + 2);
    idmap->entries = memory_zalloc(idmap->capacity * JSON_IDMAP_ELEMSIZE);

#define _DISPATCHER(NAME, STR, STRLEN) json_idmap_insert(idmap, STR, STRLEN, fnv1amix((uint8_t const *) STR, STRLEN), NAME);
    JSON_ID_LIST
#undef _DISPATCHER
}

json_id_t json_idmap_search(json_idmap_t *idmap, const char *cstr, size_t length, uint64_t hash) {
    size_t max_index = idmap->capacity - 1;
    size_t index = hash & max_index;
    size_t probe = 1;

    json_idmap_entry_t *entry = idmap->entries + index;

    while (entry->key.hash != 0) {
        if (hash == entry->key.hash && length == entry->key.length && memcmp(cstr, entry->key.cstr, length) == 0) {
            return entry->id;
        }

        index = (index + probe * probe) & max_index;
        entry = idmap->entries + index;
        probe++;
    }

    return JSON_ID_UNKNOWN;
}