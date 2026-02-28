#include "json/parser.h"

void json_parse_int64(json_pctx_t *pctx) {
    size_t digit_count = 0;

    while (TRUE) {
        char current_char = json_pctx_peek(pctx);

        if (current_char == '.') {
            if (digit_count == 0 && pctx->is_strict) {
                json_pctx_set_error(pctx, JSON_PERR_MHDD);
                return;
            }
            json_pctx_consume(pctx);
            json_pctx_buffer_push_char(pctx, current_char);
            json_pctx_switch_mode(pctx, JSON_PMODE_DOUBLE);
            return;
        }

        if (isdigit(current_char)) {
            json_pctx_consume(pctx);
            json_pctx_buffer_push_char(pctx, current_char);
            digit_count++;
            continue;
        }

        if (current_char == '_' || isalnum(current_char)) {
            if (pctx->is_strict) {
                json_pctx_set_error(pctx, JSON_PERR_IIDG);
            } else {
                json_pctx_set_recover(pctx, JSON_FBVAL_INT64);
            }
            return;
        }

        break;
    }

    json_psst_t *psst = json_psst_stack_last(&pctx->psst_stack);
    int64_t value = 0;

    if (digit_count == 0) {
        if (pctx->is_strict) {
            json_pctx_set_error(pctx, JSON_PERR_MIDG);
            return;
        }
    } else if (pctx->buffer.length >= STRING_MAX_NUMDIG) {
        if (pctx->is_strict) {
            json_pctx_set_error(pctx, JSON_PERR_TIDG);
            return;
        }
    } else {
        value = string_to_int64(&pctx->buffer, 10);
    }

    if (psst->is_array) {
        json_array_push_int64(psst->jarr, value);
    } else {
        json_object_tset_int64(psst->jobj, &pctx->temp_key, value);
    }

    json_pctx_buffer_clear(pctx);
    json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_COMMA);
}