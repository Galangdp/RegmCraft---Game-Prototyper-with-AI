#include "json/array.h"
#include "json/object.h"
#include "json/value.h"

extern arena_t *json_arena;

void json_value_init(json_value_t *jval) {
    jval->type = JSON_VTYPE_NULL;
}

void json_value_deinit(json_value_t *jval) {
    switch (jval->type) {
        case JSON_VTYPE_NULL:
            return;
        
        case JSON_VTYPE_BOOL:
        case JSON_VTYPE_INT64:
        case JSON_VTYPE_DOUBLE:
        case JSON_VTYPE_STRING:
            break;

        case JSON_VTYPE_OBJECT:
            json_object_deinit(jval->jobj);
            break;

        case JSON_VTYPE_ARRAY:
            json_array_deinit(jval->jarr);
            break;
    }

    jval->type = JSON_VTYPE_NULL;
}

void json_value_from(json_value_t *jval, const json_value_t *jvalv) {
    switch (jvalv->type) {
        case JSON_VTYPE_NULL:
            break;
        
        case JSON_VTYPE_BOOL:
            jval->bool = jvalv->bool;
            break;

        case JSON_VTYPE_INT64:
            jval->int64 = jvalv->int64;
            break;

        case JSON_VTYPE_DOUBLE:
            jval->vdouble = jvalv->vdouble;
            break;

        case JSON_VTYPE_STRING:
            jval->string = arena_alloc(json_arena, sizeof(string_view_t));
            jval->string->cstr = arena_alloc(json_arena, jvalv->string->length);
            jval->string->length = jvalv->string->length;

            memcpy(jval->string->cstr, jvalv->string->cstr, jval->string->length);
            break;

        case JSON_VTYPE_OBJECT:
            jval->jobj = arena_alloc(json_arena, sizeof(json_object_t));
            json_object_from(jval->jobj, jvalv->jobj);
            break;

        case JSON_VTYPE_ARRAY:
            jval->jarr = arena_alloc(json_arena, sizeof(json_array_t));
            json_array_from(jval->jarr, jvalv->jarr);
            break;
    }

    jval->type = jvalv->type;
}

void json_value_from_bool(json_value_t *jval, bool_t bool) {
    jval->type = JSON_VTYPE_BOOL;
    jval->bool = bool;
}

void json_value_from_int64(json_value_t *jval, int64_t int64) {
    jval->type = JSON_VTYPE_INT64;
    jval->int64 = int64;
}

void json_value_from_double(json_value_t *jval, double vdouble) {
    jval->type = JSON_VTYPE_DOUBLE;
    jval->vdouble = vdouble;
}

void json_value_from_cstr(json_value_t *jval, const char *cstr) {
    size_t length = strlen(cstr);

    jval->string = arena_alloc(json_arena, sizeof(string_view_t));
    jval->string->cstr = arena_alloc(json_arena, length);
    jval->string->length = length;
    jval->type = JSON_VTYPE_STRING;

    memcpy(jval->string->cstr, cstr, length);
}

void json_value_from_string(json_value_t *jval, const string_view_t *string) {
    jval->string = arena_alloc(json_arena, sizeof(string_view_t));
    jval->string->cstr = arena_alloc(json_arena, string->length);
    jval->string->length = string->length;
    jval->type = JSON_VTYPE_STRING;

    if (string->length > 0) {
        memcpy(jval->string->cstr, string->cstr, string->length);
    }
}

json_object_t *json_value_from_empty_object(json_value_t *jval) {
    jval->jobj = arena_alloc(json_arena, sizeof(json_object_t));
    jval->type = JSON_VTYPE_OBJECT;

    json_object_init(jval->jobj);
    return jval->jobj;
}

json_array_t *json_value_from_empty_array(json_value_t *jval) {
    jval->jarr = arena_alloc(json_arena, sizeof(json_array_t));
    jval->type = JSON_VTYPE_ARRAY;

    json_array_init(jval->jarr);
    return jval->jarr;
}

void json_value_from_object(json_value_t *jval, const json_object_t *jobj) {
    jval->jobj = arena_alloc(json_arena, sizeof(json_object_t));
    jval->type = JSON_VTYPE_OBJECT;

    json_object_from(jval->jobj, jobj);
}

void json_value_from_array(json_value_t *jval, const json_array_t *jarr) {
    jval->jarr = arena_alloc(json_arena, sizeof(json_array_t));
    jval->type = JSON_VTYPE_OBJECT;

    json_array_from(jval->jarr, jarr);
}

void json_value_set(json_value_t *jval, const json_value_t *jvalv) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
    }

    json_value_from(jval, jvalv);
}

void json_value_set_null(json_value_t *jval) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
        jval->type = JSON_VTYPE_NULL;
    }
}

void json_value_set_bool(json_value_t *jval, bool_t bool) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
    }
    jval->type = JSON_VTYPE_BOOL;
    jval->bool = bool;
}

void json_value_set_int64(json_value_t *jval, int64_t int64) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
    }
    jval->type = JSON_VTYPE_INT64;
    jval->int64 = int64;
}

void json_value_set_double(json_value_t *jval, double vdouble) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
    }
    jval->type = JSON_VTYPE_DOUBLE;
    jval->vdouble = vdouble;
}

void json_value_set_cstr(json_value_t *jval, const char *cstr) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
    }

    size_t length = strlen(cstr);

    jval->string = arena_alloc(json_arena, sizeof(string_view_t));
    jval->string->cstr = arena_alloc(json_arena, length);
    jval->string->length = length;
    jval->type = JSON_VTYPE_STRING;

    memcpy(jval->string->cstr, cstr, length);
}

void json_value_set_string(json_value_t *jval, const string_view_t *string) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
    }

    jval->string = arena_alloc(json_arena, sizeof(string_view_t));
    jval->string->cstr = arena_alloc(json_arena, string->length);
    jval->string->length = string->length;
    jval->type = JSON_VTYPE_STRING;

    if (string->length > 0) {
        memcpy(jval->string->cstr, string->cstr, string->length);
    }
}

json_object_t *json_value_set_empty_object(json_value_t *jval) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
    }
    jval->jobj = arena_alloc(json_arena, sizeof(json_object_t));
    jval->type = JSON_VTYPE_OBJECT;

    json_object_init(jval->jobj);
    return jval->jobj;
}

json_array_t *json_value_set_empty_array(json_value_t *jval) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
    }
    jval->jarr = arena_alloc(json_arena, sizeof(json_array_t));
    jval->type = JSON_VTYPE_ARRAY;

    json_array_init(jval->jarr);
    return jval->jarr;
}

void json_value_set_object(json_value_t *jval, const json_object_t *jobj) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
    }
    jval->jobj = arena_alloc(json_arena, sizeof(json_object_t));
    jval->type = JSON_VTYPE_OBJECT;

    json_object_from(jval->jobj, jobj);
}

void json_value_set_array(json_value_t *jval, const json_array_t *jarr) {
    if (jval->type != JSON_VTYPE_NULL) {
        json_value_deinit(jval);
    }
    jval->jarr = arena_alloc(json_arena, sizeof(json_array_t));
    jval->type = JSON_VTYPE_ARRAY;

    json_array_from(jval->jarr, jarr);
}
