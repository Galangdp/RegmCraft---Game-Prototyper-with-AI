#ifndef JSON_OBJECT_H
#define JSON_OBJECT_H

#include "json/value.h"

typedef uint64_t json_object_entid_t;

typedef enum json_object_entst {
    JSON_OBJECT_ENTST_EMPTY,
    JSON_OBJECT_ENTST_TOMBSTONE,
    JSON_OBJECT_ENTST_USED,
} json_object_entst_t;

typedef struct json_object_entry {
    string_hash_t key;
    json_value_t value;
    json_object_entst_t state;
} json_object_entry_t;

struct json_object {
    json_object_entry_t *entries;
    size_t length;
    size_t capacity;
    size_t threshold;
};

#define JSON_OBJECT_ENTID_INV   ((json_object_entid_t) UINT64_MAX)

void json_object_init(json_object_t *jobj);
FORCE_INLINE void json_object_deinit(json_object_t *jobj);
void json_object_shrink(json_object_t *jobj);
void json_object_from(json_object_t *jobj, const json_object_t *jobjv);

bool_t json_object_has(json_object_t *jobj, const string_hash_t *key);
bool_t json_object_chas(json_object_t *jobj, const char *cstr);

json_value_t *json_object_get(json_object_t *jobj, const string_hash_t *key);
json_value_t *json_object_cget(json_object_t *jobj, const char *cstr);
json_value_t *json_object_sget(json_object_t *jobj, const char *cstr, size_t length);
FORCE_INLINE json_value_t *json_object_iget(json_object_t *jobj, json_object_entid_t id);
json_object_entid_t json_object_get_id(json_object_t *jobj, const string_hash_t *key);
json_object_entid_t json_object_cget_id(json_object_t *jobj, const char *cstr);

void json_object_delete(json_object_t *jobj, const string_hash_t *key);
void json_object_cdelete(json_object_t *jobj, const char *cstr);
FORCE_INLINE void json_object_idelete(json_object_t *jobj, json_object_entid_t id);

void json_object_set(json_object_t *jobj, const string_hash_t *key, json_value_t *jval);
void json_object_set_null(json_object_t *jobj, const string_hash_t *key);
void json_object_set_bool(json_object_t *jobj, const string_hash_t *key, bool_t bool);
void json_object_set_int64(json_object_t *jobj, const string_hash_t *key, int64_t int64);
void json_object_set_double(json_object_t *jobj, const string_hash_t *key, double vdouble);
void json_object_set_string(json_object_t *jobj, const string_hash_t *key, const string_view_t *string);
void json_object_set_cstr(json_object_t *jobj, const string_hash_t *key, const char *cstr);
void json_object_set_object(json_object_t *jobj, const string_hash_t *key, const json_object_t *jobjv);
void json_object_set_array(json_object_t *jobj, const string_hash_t *key, const json_array_t *jarr);
json_object_t *json_object_set_empty_object(json_object_t *jobj, const string_hash_t *key);
json_array_t *json_object_set_empty_array(json_object_t *jobj, const string_hash_t *key);

void json_object_vset(json_object_t *jobj, const string_view_t *key, json_value_t *jval);
void json_object_vset_null(json_object_t *jobj, const string_view_t *key);
void json_object_vset_bool(json_object_t *jobj, const string_view_t *key, bool_t bool);
void json_object_vset_int64(json_object_t *jobj, const string_view_t *key, int64_t int64);
void json_object_vset_double(json_object_t *jobj, const string_view_t *key, double vdouble);
void json_object_vset_string(json_object_t *jobj, const string_view_t *key, const string_view_t *string);
void json_object_vset_cstr(json_object_t *jobj, const string_view_t *key, const char *cstr);
void json_object_vset_object(json_object_t *jobj, const string_view_t *key, const json_object_t *jobjv);
void json_object_vset_array(json_object_t *jobj, const string_view_t *key, const json_array_t *jarr);
json_object_t *json_object_vset_empty_object(json_object_t *jobj, const string_view_t *key);
json_array_t *json_object_vset_empty_array(json_object_t *jobj, const string_view_t *key);

void json_object_cset(json_object_t *jobj, const char *cstr, json_value_t *jval);
void json_object_cset_null(json_object_t *jobj, const char *cstr);
void json_object_cset_bool(json_object_t *jobj, const char *cstr, bool_t bool);
void json_object_cset_int64(json_object_t *jobj, const char *cstr, int64_t int64);
void json_object_cset_double(json_object_t *jobj, const char *cstr, double vdouble);
void json_object_cset_string(json_object_t *jobj, const char *cstr, const string_view_t *string);
void json_object_cset_cstr(json_object_t *jobj, const char *cstr, const char *cstrv);
void json_object_cset_object(json_object_t *jobj, const char *cstr, const json_object_t *jobjv);
void json_object_cset_array(json_object_t *jobj, const char *cstr, const json_array_t *jarr);
json_object_t *json_object_cset_empty_object(json_object_t *jobj, const char *cstr);
json_array_t *json_object_cset_empty_array(json_object_t *jobj, const char *cstr);

void json_object_tset(json_object_t *jobj, const string_hash_t *key, json_value_t *jval);
void json_object_tset_null(json_object_t *jobj, const string_hash_t *key);
void json_object_tset_bool(json_object_t *jobj, const string_hash_t *key, bool_t bool);
void json_object_tset_int64(json_object_t *jobj, const string_hash_t *key, int64_t int64);
void json_object_tset_double(json_object_t *jobj, const string_hash_t *key, double vdouble);
void json_object_tset_string(json_object_t *jobj, const string_hash_t *key, const string_view_t *string);
void json_object_tset_cstr(json_object_t *jobj, const string_hash_t *key, const char *cstr);
void json_object_tset_object(json_object_t *jobj, const string_hash_t *key, const json_object_t *jobjv);
void json_object_tset_array(json_object_t *jobj, const string_hash_t *key, const json_array_t *jarr);
json_object_t *json_object_tset_empty_object(json_object_t *jobj, const string_hash_t *key);
json_array_t *json_object_tset_empty_array(json_object_t *jobj, const string_hash_t *key);

FORCE_INLINE void json_object_deinit(json_object_t *jobj) {
    for (size_t i = 0, j = 0; i < jobj->capacity; i++) {
        json_object_entry_t *entry = jobj->entries + i;
        switch (entry->state) {
            case JSON_OBJECT_ENTST_EMPTY:
                continue;
            case JSON_OBJECT_ENTST_TOMBSTONE:
                break;
            case JSON_OBJECT_ENTST_USED:
                json_value_deinit(&entry->value);
                break;
        }
        if (++j >= jobj->length) {
            break;
        }
    }
    memory_free(jobj->entries);
}

FORCE_INLINE json_value_t *json_object_iget(json_object_t *jobj, json_object_entid_t id) {
#ifdef RC_DEBUG
    if (id >= jobj->capacity) {
        ASSERT(EXIT_FAILURE, "Json object entry id exceed the capacity.");
    }
#endif
    json_object_entry_t *entry = jobj->entries + id;
    return entry->state == JSON_OBJECT_ENTST_USED ? &entry->value : NULL;
}

FORCE_INLINE void json_object_idelete(json_object_t *jobj, json_object_entid_t id) {
#ifdef RC_DEBUG
    if (id >= jobj->capacity) {
        ASSERT(EXIT_FAILURE, "Json object entry id exceed the capacity.");
    }
#endif
    json_object_entry_t *entry = jobj->entries + id;
    
    if (entry->state == JSON_OBJECT_ENTST_USED) {
        json_value_deinit(&entry->value);
        entry->state = JSON_OBJECT_ENTST_TOMBSTONE;
    }
}

#endif /* JSON_OBJECT_H */ 