#include "json/array.h"

#define JSON_ARRAY_DEFCAP      32
#define JSON_ARRAY_ELEMSIZE    sizeof(json_value_t)

extern arena_t *json_arena;

/*
    ==================================================================
    PRIVATE METHOD
    json_array_t
    ==================================================================
*/

FORCE_INLINE json_value_t *json_array_calc_offset(json_array_t *jarr, size_t index) {
    return jarr->values + jarr->start + index;
}

FORCE_INLINE void json_array_obresize(json_array_t *jarr) {
    if (jarr->start + jarr->length >= jarr->capacity) {
        jarr->capacity <<= 1;
        jarr->values = memory_realloc(jarr->values, jarr->capacity * JSON_ARRAY_ELEMSIZE);
    }
}

FORCE_INLINE void json_array_ofresize(json_array_t *jarr) {
    if (jarr->start == 0) {
        size_t rshift_count = jarr->capacity >> 1;

        jarr->capacity <<= 1;
        jarr->values = memory_realloc(jarr->values, jarr->capacity * JSON_ARRAY_ELEMSIZE);

        memmove(jarr->values + rshift_count, jarr->values, jarr->length * JSON_ARRAY_ELEMSIZE);
        jarr->start = rshift_count;
    }
}

/*
    ==================================================================
    PUBLIC METHOD
    json_array_t
    ==================================================================
*/

void json_array_init(json_array_t *jarr) {
    jarr->values = memory_alloc(JSON_ARRAY_DEFCAP * JSON_ARRAY_ELEMSIZE);
    jarr->length = 0;
    jarr->capacity = JSON_ARRAY_DEFCAP;
    jarr->start = (JSON_ARRAY_DEFCAP >> 1) - 1;
}

void json_array_from(json_array_t *jarr, const json_array_t *jarrv) {
    jarr->values = memory_alloc(jarrv->capacity * JSON_ARRAY_ELEMSIZE);
    jarr->length = jarrv->length;
    jarr->start = jarrv->start;
    jarr->capacity = jarrv->capacity;

    for (size_t i = jarrv->start, length = jarrv->start + jarrv->length; i < length; i++) {
        json_value_from(jarr->values + i, jarrv->values + i);
    }
}

void json_array_set(json_array_t *jarr, size_t index, json_value_t *jval) {
    if (index >= jarr->length) {
        return;
    }
    json_value_set(json_array_calc_offset(jarr, index), jval);
}

void json_array_set_null(json_array_t *jarr, size_t index) {
    if (index >= jarr->length) {
        return;
    }
    json_value_set_null(json_array_calc_offset(jarr, index));
}

void json_array_set_bool(json_array_t *jarr, size_t index, bool_t bool) {
    if (index >= jarr->length) {
        return;
    }
    json_value_set_bool(json_array_calc_offset(jarr, index), bool);
}

void json_array_set_int64(json_array_t *jarr, size_t index, int64_t int64) {
    if (index >= jarr->length) {
        return;
    }
    json_value_set_int64(json_array_calc_offset(jarr, index), int64);
}

void json_array_set_double(json_array_t *jarr, size_t index, double vdouble) {
    if (index >= jarr->length) {
        return;
    }
    json_value_set_double(json_array_calc_offset(jarr, index), vdouble);
}

void json_array_set_string(json_array_t *jarr, size_t index, const string_view_t *string) {
    if (index >= jarr->length) {
        return;
    }
    json_value_set_string(json_array_calc_offset(jarr, index), string);
}

void json_array_set_cstr(json_array_t *jarr, size_t index, const char *cstr) {
    if (index >= jarr->length) {
        return;
    }
    json_value_set_cstr(json_array_calc_offset(jarr, index), cstr);
}

void json_array_set_object(json_array_t *jarr, size_t index, const json_object_t *jobj) {
    if (index >= jarr->length) {
        return;
    }
    json_value_set_object(json_array_calc_offset(jarr, index), jobj);
}

void json_array_set_array(json_array_t *jarr, size_t index, const json_array_t *jarrv) {
    if (index >= jarr->length) {
        return;
    }
    json_value_set_array(json_array_calc_offset(jarr, index), jarrv);
}

json_object_t *json_array_set_empty_object(json_array_t *jarr, size_t index) {
    if (index >= jarr->length) {
        return NULL;
    }
    return json_value_set_empty_object(json_array_calc_offset(jarr, index));
}

json_array_t *json_array_set_empty_array(json_array_t *jarr, size_t index) {
    if (index >= jarr->length) {
        return NULL;
    }
    return json_value_set_empty_array(json_array_calc_offset(jarr, index));
}

void json_array_setuns(json_array_t *jarr, size_t index, json_value_t *jval) {
    json_value_set(json_array_calc_offset(jarr, index), jval);
}

void json_array_setuns_null(json_array_t *jarr, size_t index) {
    json_value_set_null(json_array_calc_offset(jarr, index));
}

void json_array_setuns_bool(json_array_t *jarr, size_t index, bool_t bool) {
    json_value_set_bool(json_array_calc_offset(jarr, index), bool);
}

void json_array_setuns_int64(json_array_t *jarr, size_t index, int64_t int64) {
    json_value_set_int64(json_array_calc_offset(jarr, index), int64);
}

void json_array_setuns_double(json_array_t *jarr, size_t index, double vdouble) {
    json_value_set_double(json_array_calc_offset(jarr, index), vdouble);
}

void json_array_setuns_string(json_array_t *jarr, size_t index, const string_view_t *string) {
    json_value_set_string(json_array_calc_offset(jarr, index), string);
}

void json_array_setuns_cstr(json_array_t *jarr, size_t index, const char *cstr) {
    json_value_set_cstr(json_array_calc_offset(jarr, index), cstr);
}

void json_array_setuns_object(json_array_t *jarr, size_t index, const json_object_t *jobj) {
    json_value_set_object(json_array_calc_offset(jarr, index), jobj);
}

void json_array_setuns_array(json_array_t *jarr, size_t index, const json_array_t *jarrv) {
    json_value_set_array(json_array_calc_offset(jarr, index), jarrv);
}

json_object_t *json_array_setuns_empty_object(json_array_t *jarr, size_t index) {
    return json_value_set_empty_object(json_array_calc_offset(jarr, index));
}

json_array_t *json_array_setuns_empty_array(json_array_t *jarr, size_t index) {
    return json_value_set_empty_array(json_array_calc_offset(jarr, index));
}

void json_array_push(json_array_t *jarr, json_value_t *jval) {
    json_array_obresize(jarr);
    json_value_from(json_array_calc_offset(jarr, jarr->length++), jval);
}

void json_array_push_null(json_array_t *jarr) {
    json_array_obresize(jarr);
    json_value_init(json_array_calc_offset(jarr, jarr->length++));
}

void json_array_push_bool(json_array_t *jarr, bool_t bool) {
    json_array_obresize(jarr);
    json_value_from_bool(json_array_calc_offset(jarr, jarr->length++), bool);
}

void json_array_push_int64(json_array_t *jarr, int64_t int64) {
    json_array_obresize(jarr);
    json_value_from_int64(json_array_calc_offset(jarr, jarr->length++), int64);
}

void json_array_push_double(json_array_t *jarr, double vdouble) {
    json_array_obresize(jarr);
    json_value_from_double(json_array_calc_offset(jarr, jarr->length++), vdouble);
}

void json_array_push_string(json_array_t *jarr, const string_view_t *string) {
    json_array_obresize(jarr);
    json_value_from_string(json_array_calc_offset(jarr, jarr->length++), string);
}

void json_array_push_cstr(json_array_t *jarr, const char *cstr) {
    json_array_obresize(jarr);
    json_value_from_cstr(json_array_calc_offset(jarr, jarr->length++), cstr);
}

void json_array_push_object(json_array_t *jarr, const json_object_t *jobj) {
    json_array_obresize(jarr);
    json_value_from_object(json_array_calc_offset(jarr, jarr->length++), jobj);
}

void json_array_push_array(json_array_t *jarr, const json_array_t *jarrv) {
    json_array_obresize(jarr);
    json_value_from_array(json_array_calc_offset(jarr, jarr->length++), jarrv);
}

json_object_t *json_array_push_empty_object(json_array_t *jarr) {
    json_array_obresize(jarr);
    return json_value_from_empty_object(json_array_calc_offset(jarr, jarr->length++));
}

json_array_t *json_array_push_empty_array(json_array_t *jarr) {
    json_array_obresize(jarr);
    return json_value_from_empty_array(json_array_calc_offset(jarr, jarr->length++));
}

void json_array_unshift(json_array_t *jarr, json_value_t *jval) {
    json_array_ofresize(jarr);
    json_value_from(jarr->values + --jarr->start, jval);
    jarr->length++;
}

void json_array_unshift_null(json_array_t *jarr) {
    json_array_ofresize(jarr);
    json_value_init(jarr->values + --jarr->start);
    jarr->length++;
}

void json_array_unshift_bool(json_array_t *jarr, bool_t bool) {
    json_array_ofresize(jarr);
    json_value_from_bool(jarr->values + --jarr->start, bool);
    jarr->length++;
}

void json_array_unshift_int64(json_array_t *jarr, int64_t int64) {
    json_array_ofresize(jarr);
    json_value_from_int64(jarr->values + --jarr->start, int64);
    jarr->length++;
}

void json_array_unshift_double(json_array_t *jarr, double vdouble) {
    json_array_ofresize(jarr);
    json_value_from_double(jarr->values + --jarr->start, vdouble);
    jarr->length++;
}

void json_array_unshift_string(json_array_t *jarr, const string_view_t *string) {
    json_array_ofresize(jarr);
    json_value_from_string(jarr->values + --jarr->start, string);
    jarr->length++;
}

void json_array_unshift_cstr(json_array_t *jarr, const char *cstr) {
    json_array_ofresize(jarr);
    json_value_from_cstr(jarr->values + --jarr->start, cstr);
    jarr->length++;
}

void json_array_unshift_object(json_array_t *jarr, const json_object_t *jobj) {
    json_array_ofresize(jarr);
    json_value_from_object(jarr->values + --jarr->start, jobj);
    jarr->length++;
}

void json_array_unshift_array(json_array_t *jarr, const json_array_t *jarrv) {
    json_array_ofresize(jarr);
    json_value_from_array(jarr->values + --jarr->start, jarrv);
    jarr->length++;
}

json_object_t *json_array_unshift_empty_object(json_array_t *jarr) {
    json_array_ofresize(jarr);
    jarr->length++;
    return json_value_from_empty_object(jarr->values + --jarr->start);
}

json_array_t *json_array_unshift_empty_array(json_array_t *jarr) {
    json_array_ofresize(jarr);
    jarr->length++;
    return json_value_from_empty_array(jarr->values + --jarr->start);
}
