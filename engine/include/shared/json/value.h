#ifndef JSON_VALUE_H
#define JSON_VALUE_H

#include "utils/arena.h"
#include "utils/string.h"

typedef struct json_object json_object_t;
typedef struct json_array json_array_t;

typedef enum json_vtype {
    JSON_VTYPE_NULL,
    JSON_VTYPE_BOOL,
    JSON_VTYPE_INT64,
    JSON_VTYPE_DOUBLE,
    JSON_VTYPE_STRING,
    JSON_VTYPE_OBJECT,
    JSON_VTYPE_ARRAY
} json_vtype_t;

typedef struct json_value {
    union {
        bool_t bool;
        int64_t int64;
        double vdouble;
        string_view_t *string;
        json_object_t *jobj;
        json_array_t *jarr;
    };
    json_vtype_t type;
} json_value_t;

void json_value_init(json_value_t *jval);
void json_value_deinit(json_value_t *jval);

void json_value_from(json_value_t *jval, const json_value_t *jvalv);
void json_value_from_bool(json_value_t *jval, bool_t bool);
void json_value_from_int64(json_value_t *jval, int64_t int64);
void json_value_from_double(json_value_t *jval, double vdouble);
void json_value_from_cstr(json_value_t *jval, const char *cstr);
void json_value_from_string(json_value_t *jval, const string_view_t *string);
json_object_t *json_value_from_empty_object(json_value_t *jval);
json_array_t *json_value_from_empty_array(json_value_t *jval);
void json_value_from_object(json_value_t *jval, const json_object_t *jobj);
void json_value_from_array(json_value_t *jval, const json_array_t *jarr);

void json_value_set(json_value_t *jval, const json_value_t *jvalv);
void json_value_set_null(json_value_t *jval);
void json_value_set_bool(json_value_t *jval, bool_t bool);
void json_value_set_int64(json_value_t *jval, int64_t int64);
void json_value_set_double(json_value_t *jval, double vdouble);
void json_value_set_cstr(json_value_t *jval, const char *cstr);
void json_value_set_string(json_value_t *jval, const string_view_t *string);
json_object_t *json_value_set_empty_object(json_value_t *jval);
json_array_t *json_value_set_empty_array(json_value_t *jval);
void json_value_set_object(json_value_t *jval, const json_object_t *jobj);
void json_value_set_array(json_value_t *jval, const json_array_t *jarr);

FORCE_INLINE int64_t json_value_as_int64(json_value_t *jval) {
#ifdef RC_DEBUG
    if (jval->type != JSON_VTYPE_INT64) {
        ASSERT(EXIT_FAILURE, "Json value mismatch as int64.");
    }
#endif
    return jval->int64;
}

FORCE_INLINE double json_value_as_double(json_value_t *jval) {
#ifdef RC_DEBUG
    if (jval->type != JSON_VTYPE_DOUBLE) {
        ASSERT(EXIT_FAILURE, "Json value mismatch as double.");
    }
#endif
    return jval->vdouble;
}

FORCE_INLINE string_view_t *json_value_as_string(json_value_t *jval) {
#ifdef RC_DEBUG
    if (jval->type != JSON_VTYPE_STRING) {
        ASSERT(EXIT_FAILURE, "Json value mismatch as string.");
    }
#endif
    return jval->string;
}

FORCE_INLINE json_object_t *json_value_as_object(json_value_t *jval) {
#ifdef RC_DEBUG
    if (jval->type != JSON_VTYPE_OBJECT) {
        ASSERT(EXIT_FAILURE, "Json value mismatch as object.");
    }
#endif
    return jval->jobj;
}

FORCE_INLINE json_array_t *json_value_as_array(json_value_t *jval) {
#ifdef RC_DEBUG
    if (jval->type != JSON_VTYPE_ARRAY) {
        ASSERT(EXIT_FAILURE, "Json value mismatch as array.");
    }
#endif
    return jval->jarr;
}

#endif /* JSON_VALUE_H */ 
