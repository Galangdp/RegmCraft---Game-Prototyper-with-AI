#ifndef JSON_ARRAY_H
#define JSON_ARRAY_H

#include "json/value.h"

struct json_array {
    json_value_t *values;
    size_t start;
    size_t length;
    size_t capacity;
};

void json_array_init(json_array_t *jarr);
FORCE_INLINE void json_array_deinit(json_array_t *jarr);
FORCE_INLINE void json_array_shrink(json_array_t *jarr);
void json_array_from(json_array_t *jarr, const json_array_t *jarrv);

FORCE_INLINE json_value_t *json_array_first(json_array_t *jarr);
FORCE_INLINE json_value_t *json_array_last(json_array_t *jarr);
FORCE_INLINE json_value_t *json_array_at(json_array_t *jarr, size_t index);
FORCE_INLINE json_value_t *json_array_atuns(json_array_t *jarr, size_t index);

FORCE_INLINE void json_array_pop(json_array_t *jarr);
FORCE_INLINE void json_array_shift(json_array_t *jarr);

void json_array_set(json_array_t *jarr, size_t index, json_value_t *jval);
void json_array_set_null(json_array_t *jarr, size_t index);
void json_array_set_bool(json_array_t *jarr, size_t index, bool_t bool);
void json_array_set_int64(json_array_t *jarr, size_t index, int64_t int64);
void json_array_set_double(json_array_t *jarr, size_t index, double vdouble);
void json_array_set_string(json_array_t *jarr, size_t index, const string_view_t *string);
void json_array_set_cstr(json_array_t *jarr, size_t index, const char *cstr);
void json_array_set_object(json_array_t *jarr, size_t index, const json_object_t *jobj);
void json_array_set_array(json_array_t *jarr, size_t index, const json_array_t *jarrv);
json_object_t *json_array_set_empty_object(json_array_t *jarr, size_t index);
json_array_t *json_array_set_empty_array(json_array_t *jarr, size_t index);

void json_array_setuns(json_array_t *jarr, size_t index, json_value_t *jval);
void json_array_setuns_null(json_array_t *jarr, size_t index);
void json_array_setuns_bool(json_array_t *jarr, size_t index, bool_t bool);
void json_array_setuns_int64(json_array_t *jarr, size_t index, int64_t int64);
void json_array_setuns_double(json_array_t *jarr, size_t index, double vdouble);
void json_array_setuns_string(json_array_t *jarr, size_t index, const string_view_t *string);
void json_array_setuns_cstr(json_array_t *jarr, size_t index, const char *cstr);
void json_array_setuns_object(json_array_t *jarr, size_t index, const json_object_t *jobj);
void json_array_setuns_array(json_array_t *jarr, size_t index, const json_array_t *jarrv);
json_object_t *json_array_setuns_empty_object(json_array_t *jarr, size_t index);
json_array_t *json_array_setuns_empty_array(json_array_t *jarr, size_t index);

void json_array_push(json_array_t *jarr, json_value_t *jval);
void json_array_push_null(json_array_t *jarr);
void json_array_push_bool(json_array_t *jarr, bool_t bool);
void json_array_push_int64(json_array_t *jarr, int64_t int64);
void json_array_push_double(json_array_t *jarr, double vdouble);
void json_array_push_string(json_array_t *jarr, const string_view_t *string);
void json_array_push_cstr(json_array_t *jarr, const char *cstr);
void json_array_push_object(json_array_t *jarr, const json_object_t *jobj);
void json_array_push_array(json_array_t *jarr, const json_array_t *jarrv);
json_object_t *json_array_push_empty_object(json_array_t *jarr);
json_array_t *json_array_push_empty_array(json_array_t *jarr);

void json_array_unshift(json_array_t *jarr, json_value_t *jval);
void json_array_unshift_null(json_array_t *jarr);
void json_array_unshift_bool(json_array_t *jarr, bool_t bool);
void json_array_unshift_int64(json_array_t *jarr, int64_t int64);
void json_array_unshift_double(json_array_t *jarr, double vdouble);
void json_array_unshift_string(json_array_t *jarr, const string_view_t *string);
void json_array_unshift_cstr(json_array_t *jarr, const char *cstr);
void json_array_unshift_object(json_array_t *jarr, const json_object_t *jobj);
void json_array_unshift_array(json_array_t *jarr, const json_array_t *jarrv);
json_object_t *json_array_unshift_empty_object(json_array_t *jarr);
json_array_t *json_array_unshift_empty_array(json_array_t *jarr);

FORCE_INLINE void json_array_deinit(json_array_t *jarr) {
    for (size_t i = jarr->start, i_max = jarr->start + jarr->length; i < i_max; i++) {
        json_value_deinit(jarr->values + i);
    }
    memory_free(jarr->values);
}

FORCE_INLINE void json_array_shrink(json_array_t *jarr) {
    if (jarr->start > 0) {
        memmove(jarr->values, jarr->values + jarr->start, jarr->length * sizeof(json_value_t));
        jarr->start = 0;
    }

    if (jarr->capacity > jarr->length) {
        jarr->values = memory_realloc(jarr->values, jarr->length * sizeof(json_value_t));
        jarr->capacity = jarr->length;
    }
}

FORCE_INLINE json_value_t *json_array_first(json_array_t *jarr) {
#ifdef RC_DEBUG
    if (jarr->length == 0) {
        ASSERT(EXIT_FAILURE, "Trying to get first item of json array but length is 0.");
    }
#endif
    return jarr->values + jarr->start;
}

FORCE_INLINE json_value_t *json_array_last(json_array_t *jarr) {
#ifdef RC_DEBUG
    if (jarr->length == 0) {
        ASSERT(EXIT_FAILURE, "Trying to get last item of json array but length is 0.");
    }
#endif
    return jarr->values + jarr->start + jarr->length - 1;
}

FORCE_INLINE json_value_t *json_array_at(json_array_t *jarr, size_t index) {
    return index >= jarr->length ? NULL : jarr->values + jarr->start + index;
}

FORCE_INLINE json_value_t *json_array_atuns(json_array_t *jarr, size_t index) {
    return jarr->values + jarr->start + index;
}

FORCE_INLINE void json_array_pop(json_array_t *jarr) {
#ifdef RC_DEBUG
    if (jarr->length == 0) {
        ASSERT(EXIT_FAILURE, "Trying to pop empty json array.");
    }
#endif
    json_value_deinit(jarr->values + jarr->start + --jarr->length);
}

FORCE_INLINE void json_array_shift(json_array_t *jarr) {
#ifdef RC_DEBUG
    if (jarr->length == 0) {
        ASSERT(EXIT_FAILURE, "Trying to shift empty json array.");
    }
#endif
    json_value_deinit(jarr->values + jarr->start++);
}

#endif /* JSON_ARRAY_H */ 