#include "json/parser.h"

void json_parse_double(json_pctx_t *pctx) {
    size_t digit_count = 0;

    while (TRUE) {
        char current_char = json_pctx_peek(pctx);

        if (isdigit(current_char)) {
            json_pctx_consume(pctx);
            json_pctx_buffer_push_char(pctx, current_char);
            digit_count++;
            continue;
        }

        if (current_char == '_' || isalnum(current_char)) {
            if (pctx->is_strict) {
                json_pctx_set_error(pctx, JSON_PERR_IDDG);
            } else {
                json_pctx_set_recover(pctx, JSON_FBVAL_DOUBLE);
            }
            return;
        }

        break;
    }

    json_psst_t *psst = json_psst_stack_last(&pctx->psst_stack);
    double value = 0.0;

    if (digit_count == 0) {
        if (pctx->is_strict) {
            json_pctx_set_error(pctx, JSON_PERR_MTDD);
            return;
        }
    } else if (pctx->buffer.length >= STRING_MAX_NUMDIG) {
        if (pctx->is_strict) {
            json_pctx_set_error(pctx, JSON_PERR_TDDG);
            return;
        }
    } else {
        value = string_to_double(&pctx->buffer);
    }

    if (psst->is_array) {
        json_array_push_double(psst->jarr, value);
    } else {
        json_object_tset_double(psst->jobj, &pctx->temp_key, value);
    }

    json_pctx_buffer_clear(pctx);
    json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_COMMA);
}