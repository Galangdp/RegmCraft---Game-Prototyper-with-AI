#include "json/stringifier.h"

void json_stringify_array(json_sctx_t *sctx) {
    json_psst_t *psst = json_psst_stack_last(&sctx->psst_stack);
    json_array_t *jarr = psst->jarr;

    if (psst->index >= jarr->length) {
        if (jarr->length == 0) {
            string_push_char(sctx->out, '[');
        }

        string_push_char(sctx->out, ']');
        json_psst_stack_pop(&sctx->psst_stack);

        psst = json_psst_stack_last(&sctx->psst_stack);

        if (psst == NULL) {
            json_sctx_set_success(sctx);
        } else {
            json_sctx_switch_mode(sctx, psst->is_array ? JSON_SMODE_ARRAY : JSON_SMODE_OBJECT);
        }

        return;
    }
    
    string_push_char(sctx->out, psst->index == 0 ? '[' : ',');
    json_sctx_set_value(sctx, jarr->values + jarr->start + psst->index++);
}