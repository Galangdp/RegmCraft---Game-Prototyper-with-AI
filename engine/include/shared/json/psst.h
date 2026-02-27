#ifndef JSON_TMPCNT_H
#define JSON_TMPCNT_H

#include "json/json.h"

typedef struct json_psst {
    union {
        json_object_t *jobj;
        json_array_t *jarr;
    };
    size_t index;
    size_t optional_data;
    bool_t is_array;
} json_psst_t;

typedef struct json_psst_stack {
    json_psst_t *values;
    size_t length;
    size_t capacity;
} json_psst_stack_t;

void json_psst_stack_init(json_psst_stack_t *psst_stack);
FORCE_INLINE void json_psst_stack_deinit(json_psst_stack_t *psst_stack);
FORCE_INLINE json_psst_t *json_psst_stack_last(json_psst_stack_t *psst_stack);
FORCE_INLINE bool_t json_psst_stack_is_empty(json_psst_stack_t *psst_stack);
void json_psst_stack_push_object(json_psst_stack_t *psst_stack, json_object_t *jobj);
void json_psst_stack_push_array(json_psst_stack_t *psst_stack, json_array_t *jarr);
FORCE_INLINE void json_psst_stack_pop(json_psst_stack_t *psst_stack);

FORCE_INLINE void json_psst_stack_deinit(json_psst_stack_t *psst_stack) {
    memory_free(psst_stack->values);
}

FORCE_INLINE json_psst_t *json_psst_stack_last(json_psst_stack_t *psst_stack) {
    return psst_stack->length == 0 ? NULL : psst_stack->values + psst_stack->length - 1;
}

FORCE_INLINE json_psst_t *json_psst_stack_first(json_psst_stack_t *psst_stack) {
    return psst_stack->length == 0 ? NULL : psst_stack->values;
}

FORCE_INLINE bool_t json_psst_stack_is_empty(json_psst_stack_t *psst_stack) {
    return psst_stack->length == 0;
}

FORCE_INLINE void json_psst_stack_pop(json_psst_stack_t *psst_stack) {
    psst_stack->length--;
}

#endif /* JSON_TMPCNT_H */ 