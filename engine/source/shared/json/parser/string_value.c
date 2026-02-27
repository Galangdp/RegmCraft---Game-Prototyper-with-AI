#include "json/parser.h"

void json_parse_string_value(json_pctx_t *pctx) {
    while (TRUE) {
        char current_char = json_pctx_peek(pctx);

        if (current_char == '"') {
            json_pctx_consume(pctx);
            break;
        }

        if (current_char == '\n' || current_char == '\0') {
            if (pctx->is_strict) {
                json_pctx_set_error(pctx, JSON_PERR_MSQE);
                return;
            }
            break;
        }

        if (current_char == '\\') {
            json_pctx_consume(pctx);
            current_char = json_pctx_peek(pctx);

            if (!json_pctx_resolve_cseq(&current_char)) {
                if (pctx->is_strict) {
                    json_pctx_set_error(pctx, JSON_PERR_ISCS);
                    return;
                }
                continue;
            }
        }

        json_pctx_consume(pctx);
        json_pctx_buffer_push_char(pctx, current_char);
    }

    json_psst_t *psst = json_psst_stack_last(&pctx->psst_stack);
    string_view_t string_view = {.cstr = pctx->buffer.cstr, .length = pctx->buffer.length};

    if (psst->is_array) {
        json_array_push_string(psst->jarr, &string_view);
    } else {
        json_object_tset_string(psst->jobj, &pctx->temp_key, &string_view);
    }

    json_pctx_buffer_clear(pctx);
    json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_COMMA);
}