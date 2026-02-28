#include "json/parser.h"

void json_parse_recovery(json_pctx_t *pctx) {
    size_t bracket_count = 1;

    while (TRUE) {
        char current_char = json_pctx_peek(pctx);

        if (current_char == '\0') {
            json_pctx_set_success(pctx);
            return;
        }

        if (current_char == '{' || current_char == '[') {
            json_pctx_consume(pctx);
            bracket_count++;
            continue;
        }
        
        if (current_char == '}' || current_char == ']') {
            json_pctx_consume(pctx);
            bracket_count--;
            continue;
        }

        if (bracket_count > 0) {
            json_pctx_consume(pctx);
            continue;
        }

        if (current_char == ',' || current_char == ']' || current_char == '}') {
            break;
        }
    }

    json_psst_t *psst = json_psst_stack_last(&pctx->psst_stack);

    switch (pctx->fbval) {
        case JSON_FBVAL_NONE:
            return;
        case JSON_FBVAL_NULL:
            if (psst->is_array) {
                json_array_push_null(psst->jarr);
            } else {
                json_object_set_null(psst->jobj, &pctx->temp_key);
            }
            break;

        case JSON_FBVAL_INT64:
            if (psst->is_array) {
                json_array_push_int64(psst->jarr, 0);
            } else {
                json_object_set_int64(psst->jobj, &pctx->temp_key, 0);
            }
            break;

        case JSON_FBVAL_DOUBLE:
            if (psst->is_array) {
                json_array_push_double(psst->jarr, 0.0);
            } else {
                json_object_set_double(psst->jobj, &pctx->temp_key, 0.0);
            }
            break;
    }
}