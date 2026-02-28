#include "json/psst.h"

#define JSON_PSST_STACK_DEFCAP      32
#define JSON_PSST_STACK_ELEMSIZE    sizeof(json_psst_t)

FORCE_INLINE void json_psst_init_object(json_psst_t *psst, json_object_t *jobj) {
    psst->jobj = jobj;
    psst->is_array = FALSE;
    psst->index = 0;
    psst->optional_data = 0;
}

FORCE_INLINE void json_psst_init_array(json_psst_t *psst, json_array_t *jarr) {
    psst->jarr = jarr;
    psst->is_array = TRUE;
    psst->index = 0;
    psst->optional_data = 0;
}

FORCE_INLINE void json_psst_stack_oresize(json_psst_stack_t *psst_stack) {
    if (psst_stack->length >= psst_stack->capacity) {
        psst_stack->capacity <<= 1;
        psst_stack->values = memory_alloc(psst_stack->capacity * JSON_PSST_STACK_ELEMSIZE);
    }
}

void json_psst_stack_init(json_psst_stack_t *psst_stack) {
    psst_stack->values = memory_alloc(JSON_PSST_STACK_DEFCAP * JSON_PSST_STACK_ELEMSIZE);
    psst_stack->length = 0;
    psst_stack->capacity = JSON_PSST_STACK_DEFCAP;
}

void json_psst_stack_push_object(json_psst_stack_t *psst_stack, json_object_t *jobj) {
    json_psst_stack_oresize(psst_stack);
    json_psst_init_object(psst_stack->values + psst_stack->length++, jobj);
}

void json_psst_stack_push_array(json_psst_stack_t *psst_stack, json_array_t *jarr) {
    json_psst_stack_oresize(psst_stack);
    json_psst_init_array(psst_stack->values + psst_stack->length++, jarr);
}