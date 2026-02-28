#include "json/object.h"

#define JSON_OBJECT_DEFCAP      64
#define JSON_OBJECT_ELEMSIZE    sizeof(json_object_entry_t)

extern arena_t *json_arena;

/*
    ==================================================================
    PRIVATE METHOD
    json_object_entry_t
    ==================================================================
*/

FORCE_INLINE void json_object_entry_init(json_object_entry_t *entry, string_hash_t const *key, json_value_t *jval) {
    entry->key.cstr = arena_alloc(json_arena, key->length);
    entry->key.length = key->length;
    entry->key.hash = key->hash;
    entry->state = JSON_OBJECT_ENTST_USED;
    
    json_value_from(&entry->value, jval);
    memcpy(entry->key.cstr, key->cstr, key->length);
}

FORCE_INLINE void json_object_entry_init_tombstone(json_object_entry_t *entry, string_hash_t const *key) {
    entry->key.cstr = arena_alloc(json_arena, key->length);
    entry->key.length = key->length;
    entry->key.hash = key->hash;
    entry->state = JSON_OBJECT_ENTST_TOMBSTONE;
    
    memcpy(entry->key.cstr, key->cstr, key->length);
}

FORCE_INLINE void json_object_entry_nvrinit(json_object_entry_t *entry, char const *cstr, size_t length, uint64_t hash) {
    entry->key.cstr = arena_alloc(json_arena, length);
    entry->key.length = length;
    entry->key.hash = hash;
    entry->state = JSON_OBJECT_ENTST_USED;
    
    memcpy(entry->key.cstr, cstr, length);
}

FORCE_INLINE void json_object_entry_ncvrinit(json_object_entry_t *entry, char const *cstr, size_t length, uint64_t hash) {
    entry->key.cstr = (char *) cstr;
    entry->key.length = length;
    entry->key.hash = hash;
    entry->state = JSON_OBJECT_ENTST_USED;
}

/*
    ==================================================================
    PRIVATE STATIC METHOD
    json_object_t
    ==================================================================
*/

FORCE_INLINE json_object_entry_t *json_object_get_unused_entry(json_object_entry_t *entries, uint64_t hash, size_t max_index) {
    size_t index = hash & max_index;
    size_t probe = 1;

    json_object_entry_t *entry = entries + index;

    while (entry->state == JSON_OBJECT_ENTST_USED) {
        index = (index + probe * probe) & max_index;
        entry = entries + index;
        probe++;
    }

    return entry;
}

/*
    ==================================================================
    PRIVATE METHOD
    json_object_t
    ==================================================================
*/

FORCE_INLINE void json_object_calc_threshold(json_object_t *jobj) {
    jobj->threshold = (size_t) ((double) jobj->capacity - (double) jobj->capacity * 0.3);
}

FORCE_INLINE json_object_entry_t *json_object_eget(json_object_t *jobj, const char *cstr, size_t length, uint64_t hash) {
    size_t max_index = jobj->capacity - 1;
    size_t index = hash & max_index;
    size_t probe = 1;

    json_object_entry_t *entry = jobj->entries + index;

    while (entry->state != JSON_OBJECT_ENTST_EMPTY) {
        if (entry->key.hash == hash && entry->key.length == length && memcmp(entry->key.cstr, cstr, length) == 0) {
            return entry;
        }

        index = (index + probe * probe) & max_index;
        entry = jobj->entries + index;
        probe++;
    }

    return NULL;
}

FORCE_INLINE bool_t json_object_set_get(json_object_t *jobj, json_object_entry_t **out, const char *cstr, size_t length, uint64_t hash) {
    size_t max_index = jobj->capacity - 1;
    size_t index = hash & max_index;
    size_t probe = 1;

    json_object_entry_t *entry = jobj->entries + index;

    while (entry->state == JSON_OBJECT_ENTST_USED) {
        if (entry->key.hash == hash && entry->key.length == length && memcmp(entry->key.cstr, cstr, length) == 0) {
            *out = entry;
            return FALSE;
        }

        index = (index + probe * probe) & max_index;
        entry = jobj->entries + index;
        probe++;
    }

    json_object_entry_nvrinit(entry, cstr, length, hash);
    *out = entry;

    return TRUE;
}

FORCE_INLINE bool_t json_object_ncset_get(json_object_t *jobj, json_object_entry_t **out, const char *cstr, size_t length, uint64_t hash) {
    size_t max_index = jobj->capacity - 1;
    size_t index = hash & max_index;
    size_t probe = 1;

    json_object_entry_t *entry = jobj->entries + index;

    while (entry->state == JSON_OBJECT_ENTST_USED) {
        if (entry->key.hash == hash && entry->key.length == length && memcmp(entry->key.cstr, cstr, length) == 0) {
            *out = entry;
            return FALSE;
        }

        index = (index + probe * probe) & max_index;
        entry = jobj->entries + index;
        probe++;
    }

    json_object_entry_ncvrinit(entry, cstr, length, hash);
    *out = entry;

    return TRUE;
}

FORCE_INLINE json_object_entid_t json_object_rget_id(json_object_t *jobj, const char *cstr, size_t length, uint64_t hash) {
    size_t max_index = jobj->capacity - 1;
    size_t index = hash & max_index;
    size_t probe = 1;

    json_object_entry_t *entry = jobj->entries + index;

    while (entry->state != JSON_OBJECT_ENTST_EMPTY) {
        if (entry->key.hash == hash && entry->key.length == length && memcmp(entry->key.cstr, cstr, length) == 0) {
            return entry->state == JSON_OBJECT_ENTST_TOMBSTONE ? JSON_OBJECT_ENTID_INV : index;
        }

        index = (index + probe * probe) & max_index;
        entry = jobj->entries + index;
        probe++;
    }

    return JSON_OBJECT_ENTID_INV;
}

FORCE_INLINE void json_object_oresize(json_object_t *jobj) {
    if (jobj->length < jobj->threshold) {
        return;
    }

    size_t old_capacity = jobj->capacity;

    jobj->capacity <<= 1;

    json_object_entry_t *new_entries = memory_zalloc(jobj->capacity * JSON_OBJECT_ELEMSIZE);
    size_t max_index = jobj->capacity - 1;

    for (size_t i = 0, j = 0; i < old_capacity; i++) {
        json_object_entry_t *entry = jobj->entries + i;

        if (entry->state == JSON_OBJECT_ENTST_EMPTY) {
            continue;
        }

        if (entry->state == JSON_OBJECT_ENTST_USED) {
            json_object_entry_t *new_entry = json_object_get_unused_entry(new_entries, entry->key.hash, max_index);
            json_object_entry_init(new_entry, &entry->key, &entry->value);
        }
        
        if (++j >= jobj->length) {
            break;
        }
    }

    memory_free(jobj->entries);
    json_object_calc_threshold(jobj);

    jobj->entries = new_entries;
}

/*
    ==================================================================
    PUBLIC METHOD
    json_object_t
    ==================================================================
*/

void json_object_init(json_object_t *jobj) {
    jobj->entries = memory_zalloc(JSON_OBJECT_DEFCAP * JSON_OBJECT_ELEMSIZE);
    jobj->length = 0;
    jobj->capacity = JSON_OBJECT_DEFCAP;

    json_object_calc_threshold(jobj);
}

void json_object_shrink(json_object_t *jobj) {
    json_object_entry_t *new_entries = memory_zalloc(jobj->capacity * JSON_OBJECT_ELEMSIZE);
    size_t max_index = jobj->capacity - 1;

    for (size_t i = 0, j = 0; i < jobj->capacity; i++) {
        json_object_entry_t *entry = jobj->entries + i;

        if (entry->state != JSON_OBJECT_ENTST_USED) {
            continue;
        }

        json_object_entry_t *new_entry = json_object_get_unused_entry(new_entries, entry->key.hash, max_index);
        json_object_entry_init(new_entry, &entry->key, &entry->value);
        
        if (++j >= jobj->length) {
            break;
        }
    }

    memory_free(jobj->entries);
    jobj->entries = new_entries;
}

void json_object_from(json_object_t *jobj, const json_object_t *jobjv) {
    jobj->entries = memory_zalloc(jobjv->capacity * JSON_OBJECT_ELEMSIZE);
    jobj->length = jobjv->length;
    jobj->capacity = jobjv->capacity;
    jobj->threshold = jobjv->threshold;

    for (size_t i = 0, j = 0; i < jobj->capacity; i++) {
        json_object_entry_t *entry = jobjv->entries + i;

        if (entry->state == JSON_OBJECT_ENTST_EMPTY) {
            continue;
        }

        if (entry->state == JSON_OBJECT_ENTST_TOMBSTONE) {
            json_object_entry_init_tombstone(jobj->entries + i, &entry->key);
        } else {
            json_object_entry_init(jobj->entries + i, &entry->key, &entry->value);
        }

        if (++j >= jobjv->length) {
            break;
        }
    }
}

bool_t json_object_has(json_object_t *jobj, const string_hash_t *key) {
    json_object_entry_t *entry = json_object_eget(jobj, key->cstr, key->length, key->hash);
    return entry != NULL && entry->state != JSON_OBJECT_ENTST_TOMBSTONE;
}

bool_t json_object_chas(json_object_t *jobj, const char *cstr) {
    size_t length = strlen(cstr);
    uint64_t hash = fnv1amix((const uint8_t *) cstr, length);
    json_object_entry_t *entry = json_object_eget(jobj, cstr, length, hash);

    return entry != NULL && entry->state != JSON_OBJECT_ENTST_TOMBSTONE;
}

json_value_t *json_object_get(json_object_t *jobj, const string_hash_t *key) {
    json_object_entry_t *entry = json_object_eget(jobj, key->cstr, key->length, key->hash);
    return entry != NULL && entry->state != JSON_OBJECT_ENTST_TOMBSTONE ? &entry->value : NULL;
}

json_value_t *json_object_sget(json_object_t *jobj, const char *cstr, size_t length) {
    uint64_t hash = fnv1amix((const uint8_t *) cstr, length);
    json_object_entry_t *entry = json_object_eget(jobj, cstr, length, hash);

    return entry != NULL && entry->state != JSON_OBJECT_ENTST_TOMBSTONE ? &entry->value : NULL;
}

json_value_t *json_object_cget(json_object_t *jobj, const char *cstr) {
    size_t length = strlen(cstr);
    uint64_t hash = fnv1amix((const uint8_t *) cstr, length);
    json_object_entry_t *entry = json_object_eget(jobj, cstr, length, hash);

    return entry != NULL && entry->state != JSON_OBJECT_ENTST_TOMBSTONE ? &entry->value : NULL;
}

json_object_entid_t json_object_get_id(json_object_t *jobj, const string_hash_t *key) {
    return json_object_rget_id(jobj, key->cstr, key->length, key->hash);
}

json_object_entid_t json_object_cget_id(json_object_t *jobj, const char *cstr) {
    size_t length = strlen(cstr);
    return json_object_rget_id(jobj, cstr, length, fnv1amix((const uint8_t *) cstr, length));
}

void json_object_delete(json_object_t *jobj, const string_hash_t *key) {
    json_object_entry_t *entry = json_object_eget(jobj, key->cstr, key->length, key->hash);

    if (entry == NULL) {
#ifdef RC_DEBUG
        ASSERT(EXIT_FAILURE, "Deleting a key but key doesn't exist.");
#endif
        return;
    }

    if (entry->state == JSON_OBJECT_ENTST_USED) {
        json_value_deinit(&entry->value);
        entry->state = JSON_OBJECT_ENTST_TOMBSTONE;
    }
}

void json_object_cdelete(json_object_t *jobj, const char *cstr) {
    size_t length = strlen(cstr);
    json_object_entry_t *entry = json_object_eget(jobj, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (entry == NULL) {
#ifdef RC_DEBUG
        ASSERT(EXIT_FAILURE, "Deleting a key but key doesn't exist.");
#endif
        return;
    }

    if (entry->state == JSON_OBJECT_ENTST_USED) {
        json_value_deinit(&entry->value);
        entry->state = JSON_OBJECT_ENTST_TOMBSTONE;
    }
}

void json_object_set(json_object_t *jobj, const string_hash_t *key, json_value_t *jval) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from(&entry->value, jval);
    } else {
        json_value_set(&entry->value, jval);
    }
}

void json_object_set_null(json_object_t *jobj, const string_hash_t *key) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_init(&entry->value);
    } else {
        json_value_set_null(&entry->value);
    }
}

void json_object_set_bool(json_object_t *jobj, const string_hash_t *key, bool_t bool) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_bool(&entry->value, bool);
    } else {
        json_value_set_bool(&entry->value, bool);
    }
}

void json_object_set_int64(json_object_t *jobj, const string_hash_t *key, int64_t int64) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_int64(&entry->value, int64);
    } else {
        json_value_set_int64(&entry->value, int64);
    }
}

void json_object_set_double(json_object_t *jobj, const string_hash_t *key, double vdouble) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_double(&entry->value, vdouble);
    } else {
        json_value_set_double(&entry->value, vdouble);
    }
}

void json_object_set_string(json_object_t *jobj, const string_hash_t *key, const string_view_t *string) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_string(&entry->value, string);
    } else {
        json_value_set_string(&entry->value, string);
    }
}

void json_object_set_cstr(json_object_t *jobj, const string_hash_t *key, const char *cstr) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_cstr(&entry->value, cstr);
    } else {
        json_value_set_cstr(&entry->value, cstr);
    }
}

void json_object_set_object(json_object_t *jobj, const string_hash_t *key, const json_object_t *jobjv) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_object(&entry->value, jobjv);
    } else {
        json_value_set_object(&entry->value, jobjv);
    }
}

void json_object_set_array(json_object_t *jobj, const string_hash_t *key, const json_array_t *jarr) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_array(&entry->value, jarr);
    } else {
        json_value_set_array(&entry->value, jarr);
    }
}

json_object_t *json_object_set_empty_object(json_object_t *jobj, const string_hash_t *key) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        return json_value_from_empty_object(&entry->value);
    }
    
    return json_value_set_empty_object(&entry->value);
}

json_array_t *json_object_set_empty_array(json_object_t *jobj, const string_hash_t *key) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        return json_value_from_empty_array(&entry->value);
    }
    
    return json_value_set_empty_array(&entry->value);
}

void json_object_vset(json_object_t *jobj, const string_view_t *key, json_value_t *jval) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from(&entry->value, jval);
    } else {
        json_value_set(&entry->value, jval);
    }
}

void json_object_vset_null(json_object_t *jobj, const string_view_t *key) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        json_value_init(&entry->value);
    } else {
        json_value_set_null(&entry->value);
    }
}

void json_object_vset_bool(json_object_t *jobj, const string_view_t *key, bool_t bool) {
    json_object_oresize(jobj);
    
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_bool(&entry->value, bool);
    } else {
        json_value_set_bool(&entry->value, bool);
    }
}

void json_object_vset_int64(json_object_t *jobj, const string_view_t *key, int64_t int64) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_int64(&entry->value, int64);
    } else {
        json_value_set_int64(&entry->value, int64);
    }
}

void json_object_vset_double(json_object_t *jobj, const string_view_t *key, double vdouble) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_double(&entry->value, vdouble);
    } else {
        json_value_set_double(&entry->value, vdouble);
    }
}

void json_object_vset_string(json_object_t *jobj, const string_view_t *key, const string_view_t *string) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_string(&entry->value, string);
    } else {
        json_value_set_string(&entry->value, string);
    }
}

void json_object_vset_cstr(json_object_t *jobj, const string_view_t *key, const char *cstr) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_cstr(&entry->value, cstr);
    } else {
        json_value_set_cstr(&entry->value, cstr);
    }
}

void json_object_vset_object(json_object_t *jobj, const string_view_t *key, const json_object_t *jobjv) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_object(&entry->value, jobjv);
    } else {
        json_value_set_object(&entry->value, jobjv);
    }
}

void json_object_vset_array(json_object_t *jobj, const string_view_t *key, const json_array_t *jarr) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_array(&entry->value, jarr);
    } else {
        json_value_set_array(&entry->value, jarr);
    }
}

json_object_t *json_object_vset_empty_object(json_object_t *jobj, const string_view_t *key) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        return json_value_from_empty_object(&entry->value);
    }
    
    return json_value_set_empty_object(&entry->value);
}

json_array_t *json_object_vset_empty_array(json_object_t *jobj, const string_view_t *key) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, key->cstr, key->length, fnv1amix((const uint8_t *) key->cstr, key->length));

    if (is_new_entry) {
        jobj->length++;
        return json_value_from_empty_array(&entry->value);
    }
    
    return json_value_set_empty_array(&entry->value);
}


void json_object_cset(json_object_t *jobj, const char *cstr, json_value_t *jval) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from(&entry->value, jval);
    } else {
        json_value_set(&entry->value, jval);
    }
}

void json_object_cset_null(json_object_t *jobj, const char *cstr) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        json_value_init(&entry->value);
    } else {
        json_value_set_null(&entry->value);
    }
}

void json_object_cset_bool(json_object_t *jobj, const char *cstr, bool_t bool) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_bool(&entry->value, bool);
    } else {
        json_value_set_bool(&entry->value, bool);
    }
}

void json_object_cset_int64(json_object_t *jobj, const char *cstr, int64_t int64) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_int64(&entry->value, int64);
    } else {
        json_value_set_int64(&entry->value, int64);
    }
}

void json_object_cset_double(json_object_t *jobj, const char *cstr, double vdouble) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_double(&entry->value, vdouble);
    } else {
        json_value_set_double(&entry->value, vdouble);
    }
}

void json_object_cset_string(json_object_t *jobj, const char *cstr, const string_view_t *string) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_string(&entry->value, string);
    } else {
        json_value_set_string(&entry->value, string);
    }
}

void json_object_cset_cstr(json_object_t *jobj, const char *cstr, const char *cstrv) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_cstr(&entry->value, cstrv);
    } else {
        json_value_set_cstr(&entry->value, cstrv);
    }
}

void json_object_cset_object(json_object_t *jobj, const char *cstr, const json_object_t *jobjv) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_object(&entry->value, jobjv);
    } else {
        json_value_set_object(&entry->value, jobjv);
    }
}

void json_object_cset_array(json_object_t *jobj, const char *cstr, const json_array_t *jarr) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        json_value_from_array(&entry->value, jarr);
    } else {
        json_value_set_array(&entry->value, jarr);
    }
}

json_object_t *json_object_cset_empty_object(json_object_t *jobj, const char *cstr) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        return json_value_from_empty_object(&entry->value);
    }
    
    return json_value_set_empty_object(&entry->value);
}

json_array_t *json_object_cset_empty_array(json_object_t *jobj, const char *cstr) {
    json_object_oresize(jobj);

    size_t length = strlen(cstr);
    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_set_get(jobj, &entry, cstr, length, fnv1amix((const uint8_t *) cstr, length));

    if (is_new_entry) {
        jobj->length++;
        return json_value_from_empty_array(&entry->value);
    }
    
    return json_value_set_empty_array(&entry->value);
}

void json_object_tset(json_object_t *jobj, const string_hash_t *key, json_value_t *jval) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from(&entry->value, jval);
    } else {
        json_value_set(&entry->value, jval);
    }
}

void json_object_tset_null(json_object_t *jobj, const string_hash_t *key) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_init(&entry->value);
    } else {
        json_value_set_null(&entry->value);
    }
}

void json_object_tset_bool(json_object_t *jobj, const string_hash_t *key, bool_t bool) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_bool(&entry->value, bool);
    } else {
        json_value_set_bool(&entry->value, bool);
    }
}

void json_object_tset_int64(json_object_t *jobj, const string_hash_t *key, int64_t int64) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_int64(&entry->value, int64);
    } else {
        json_value_set_int64(&entry->value, int64);
    }
}

void json_object_tset_double(json_object_t *jobj, const string_hash_t *key, double vdouble) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_double(&entry->value, vdouble);
    } else {
        json_value_set_double(&entry->value, vdouble);
    }
}

void json_object_tset_string(json_object_t *jobj, const string_hash_t *key, const string_view_t *string) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_string(&entry->value, string);
    } else {
        json_value_set_string(&entry->value, string);
    }
}

void json_object_tset_cstr(json_object_t *jobj, const string_hash_t *key, const char *cstr) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_cstr(&entry->value, cstr);
    } else {
        json_value_set_cstr(&entry->value, cstr);
    }
}

void json_object_tset_object(json_object_t *jobj, const string_hash_t *key, const json_object_t *jobjv) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_object(&entry->value, jobjv);
    } else {
        json_value_set_object(&entry->value, jobjv);
    }
}

void json_object_tset_array(json_object_t *jobj, const string_hash_t *key, const json_array_t *jarr) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        json_value_from_array(&entry->value, jarr);
    } else {
        json_value_set_array(&entry->value, jarr);
    }
}

json_object_t *json_object_tset_empty_object(json_object_t *jobj, const string_hash_t *key) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        return json_value_from_empty_object(&entry->value);
    }
    
    return json_value_set_empty_object(&entry->value);
}

json_array_t *json_object_tset_empty_array(json_object_t *jobj, const string_hash_t *key) {
    json_object_oresize(jobj);

    json_object_entry_t *entry = NULL;
    bool_t is_new_entry = json_object_ncset_get(jobj, &entry, key->cstr, key->length, key->hash);

    if (is_new_entry) {
        jobj->length++;
        return json_value_from_empty_array(&entry->value);
    }
    
    return json_value_set_empty_array(&entry->value);
}