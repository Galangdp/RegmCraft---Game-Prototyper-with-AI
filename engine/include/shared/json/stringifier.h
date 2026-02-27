#ifndef JSON_STRINGIFIER_H
#define JSON_STRINGIFIER_H

#include <ctype.h>
#include "json/const.h"
#include "json/json.h"
#include "json/psst.h"

typedef enum json_smode {
    JSON_SMODE_SUCCESS,
    JSON_SMODE_OBJECT,
    JSON_SMODE_ARRAY,
    JSON_SMODE_VALUE
} json_smode_t;

typedef struct json_sctx {
    json_psst_stack_t psst_stack;
    string_t *out;
    json_value_t *temp_jval;
    json_smode_t mode;
} json_sctx_t;

FORCE_INLINE void json_sctx_set_success(json_sctx_t *sctx) {
    sctx->mode = JSON_SMODE_SUCCESS;
}

FORCE_INLINE void json_sctx_set_value(json_sctx_t *sctx, json_value_t *jval) {
    sctx->mode = JSON_SMODE_VALUE;
    sctx->temp_jval = jval;
}

FORCE_INLINE void json_sctx_switch_mode(json_sctx_t *sctx, json_smode_t mode) {
    sctx->mode = mode;
}

void json_stringify_object(json_sctx_t *sctx);
void json_stringify_array(json_sctx_t *sctx);
void json_stringify_value(json_sctx_t *sctx);

#endif /* JSON_STRINGIFIER_H */ 