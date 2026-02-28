#ifndef JSON_IDMAP_H
#define JSON_IDMAP_H

#include "utils/string.h"
#include "utils/utils.h"

#define JSON_ID_LIST \
    _DISPATCHER(JSON_ID_NULL, JSON_CSTR_NULL, sizeof(JSON_CSTR_NULL)) \
    _DISPATCHER(JSON_ID_TRUE, JSON_CSTR_TRUE, sizeof(JSON_CSTR_TRUE)) \
    _DISPATCHER(JSON_ID_FALSE, JSON_CSTR_FALSE, sizeof(JSON_CSTR_FALSE)) \
\

#define _DISPATCHER(NAME, _STR, _STRLEN) NAME,
typedef enum json_id {
    JSON_ID_LIST
    JSON_ID_UNKNOWN
} json_id_t;
#undef _DISPATCHER

typedef struct json_idmap_entry {
    string_hash_t key;
    json_id_t id;
} json_idmap_entry_t;

typedef struct json_idmap {
    json_idmap_entry_t *entries;
    size_t capacity;
} json_idmap_t;

void json_idmap_init(json_idmap_t *idmap);
FORCE_INLINE void json_idmap_deinit(json_idmap_t *idmap);
json_id_t json_idmap_search(json_idmap_t *idmap, const char *cstr, size_t length, uint64_t hash);

FORCE_INLINE void json_idmap_deinit(json_idmap_t *idmap) {
    memory_free(idmap->entries);
}

#endif /* JSON_IDMAP_H */ 