#include "json/const.h"
#include "json/json.h"

arena_t *json_arena = NULL;
json_idmap_t *json_idmap = NULL;

void json_ctx_set_arena(arena_t *arena) {
    json_arena = arena;
}

void json_ctx_set_idmap(json_idmap_t *idmap) {
    json_idmap = idmap;
}

void json_init_object(json_t *json) {
    json->jobj = arena_alloc(json_arena, sizeof(json_object_t));
    json->is_array = FALSE;

    json_object_init(json->jobj);
}

void json_init_array(json_t *json) {
    json->jarr = arena_alloc(json_arena, sizeof(json_array_t));
    json->is_array = TRUE;

    json_array_init(json->jarr);
}

void json_deinit(json_t *json) {
    if (json->is_array) {
        json_array_deinit(json->jarr);
    } else {
        json_object_deinit(json->jobj);
    }
}

#ifdef RC_DEBUG
#define JSON_DUMP_SPACELEN      4

LOCAL void json_object_dump(json_object_t *jobj, string_t *out, size_t space_count);
LOCAL void json_array_dump(json_array_t *jarr, string_t *out, size_t space_count);

LOCAL void json_value_dump(json_value_t *jval, string_t *out, size_t space_count) {
    switch (jval->type) {
        case JSON_VTYPE_NULL:
            string_push_cstr(out, JSON_CSTR_NULL);
            break;
        
        case JSON_VTYPE_BOOL:
            if (jval->bool) {
                string_push_cstrl(out, JSON_CSTR_TRUE, sizeof(JSON_CSTR_TRUE));
            } else {
                string_push_cstrl(out, JSON_CSTR_FALSE, sizeof(JSON_CSTR_FALSE));
            }
            break;

        case JSON_VTYPE_INT64:
            string_push_int64(out, jval->int64);
            break;

        case JSON_VTYPE_DOUBLE:
            string_push_double(out, jval->vdouble);
            break;
            
        case JSON_VTYPE_STRING:
            string_push_cstr(out, "[ADDR: ");
            string_push_pointer(out, jval->string->cstr);
            string_push_cstr(out, "]: \"");
            string_push_cstrl(out, jval->string->cstr, jval->string->length);
            string_push_char(out, '"');
            break;

        case JSON_VTYPE_OBJECT:
            json_object_dump(jval->jobj, out, space_count);
            break;
        
        case JSON_VTYPE_ARRAY:
            json_array_dump(jval->jarr, out, space_count);
            break;
    }
}

LOCAL void json_object_dump(json_object_t *jobj, string_t *out, size_t space_count) {
    size_t next_space_count = space_count + JSON_DUMP_SPACELEN;

    string_push_cstr(out, "{\n");
    string_push_chseq(out, ' ', next_space_count);
    string_push_cstr(out, "[ADDR: ");
    string_push_pointer(out, jobj);
    string_push_cstr(out, " THRES: ");
    string_push_uint64(out, jobj->threshold);
    string_push_cstr(out, " LENGTH: ");
    string_push_uint64(out, jobj->length);
    string_push_cstr(out, " CAPS: ");
    string_push_uint64(out, jobj->capacity);
    string_push_cstr(out, "],");
    
    for (size_t i = 0, j = 0; i < jobj->capacity; i++) {
        json_object_entry_t *entry = jobj->entries + i;

        if (entry->state == JSON_OBJECT_ENTST_EMPTY) {
            continue;
        }

        string_push_char(out, '\n');
        string_push_chseq(out, ' ', next_space_count);
        string_push_cstr(out, "[IDX: ");
        string_push_uint64(out, i);
        string_push_cstr(out, " ADDR: ");
        string_push_pointer(out, entry->key.cstr);
        string_push_cstr(out, " HASH: ");
        string_push_uint64(out, entry->key.hash);
        string_push_cstr(out, "]: \"");
        string_push_cstrl(out, entry->key.cstr, entry->key.length);
        string_push_cstr(out, "\": ");
        json_value_dump(&entry->value, out, next_space_count);

        if (++j >= jobj->length) {
            break;
        }

        string_push_char(out, ',');
    }

    string_push_char(out, '\n');

    if (space_count > 0) {
        string_push_chseq(out, ' ', space_count);
    }

    string_push_char(out, '}');
}

LOCAL void json_array_dump(json_array_t *jarr, string_t *out, size_t space_count) {
    size_t next_space_count = space_count + JSON_DUMP_SPACELEN;

    string_push_cstr(out, "[\n");
    string_push_chseq(out, ' ', next_space_count);
    string_push_cstr(out, "[ADDR: ");
    string_push_pointer(out, jarr);
    string_push_cstr(out, " LENGTH: ");
    string_push_uint64(out, jarr->length);
    string_push_cstr(out, " CAPS: ");
    string_push_uint64(out, jarr->capacity);
    string_push_cstr(out, "],");
    
    for (size_t i = jarr->start, i_max = jarr->start + jarr->length; i < i_max; i++) {
        json_value_t *jval = jarr->values + i;
        
        string_push_char(out, '\n');
        string_push_chseq(out, ' ', next_space_count);
        json_value_dump(jval, out, next_space_count);

        if (i + 1 < i_max) {
            string_push_char(out, ',');
        }
    }

    string_push_char(out, '\n');

    if (space_count > 0) {
        string_push_chseq(out, ' ', space_count);
    }

    string_push_char(out, ']');
}

void json_dump(json_t *json, string_t *out) {
    string_init_capacity(out, 512);
    if (json->is_array) {
        json_array_dump(json->jarr, out, 0);
    } else {
        json_object_dump(json->jobj, out, 0);
    }
}
#endif