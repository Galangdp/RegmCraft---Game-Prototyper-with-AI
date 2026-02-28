#include "json/stringifier.h"

void json_stringify_value(json_sctx_t *sctx) {
    json_psst_t *psst = json_psst_stack_last(&sctx->psst_stack);
    json_value_t *jval = sctx->temp_jval;

    switch (jval->type) {
        case JSON_VTYPE_NULL:
            string_push_cstrl(sctx->out, JSON_CSTR_NULL, sizeof(JSON_CSTR_NULL));
            break;

        case JSON_VTYPE_BOOL:
            if (jval->bool) {
                string_push_cstrl(sctx->out, JSON_CSTR_TRUE, sizeof(JSON_CSTR_TRUE));
            } else {
                string_push_cstrl(sctx->out, JSON_CSTR_FALSE, sizeof(JSON_CSTR_FALSE));
            }
            break;

        case JSON_VTYPE_INT64:
            string_push_int64(sctx->out, jval->int64);
            break;

        case JSON_VTYPE_DOUBLE:
            string_push_double(sctx->out, jval->vdouble);
            break;

        case JSON_VTYPE_STRING:
            string_push_char(sctx->out, '"');
            if (jval->string->length > 0) {
                string_push_cstrl(sctx->out, jval->string->cstr, jval->string->length);
            }
            string_push_char(sctx->out, '"');
            break;
            
        case JSON_VTYPE_OBJECT:
            json_psst_stack_push_object(&sctx->psst_stack, jval->jobj);
            json_sctx_switch_mode(sctx, JSON_SMODE_OBJECT);
            return;
        
        case JSON_VTYPE_ARRAY:
            json_psst_stack_push_array(&sctx->psst_stack, jval->jarr);
            json_sctx_switch_mode(sctx, JSON_SMODE_ARRAY);
            return;
    }

    json_sctx_switch_mode(sctx, psst->is_array ? JSON_SMODE_ARRAY : JSON_SMODE_OBJECT);
}