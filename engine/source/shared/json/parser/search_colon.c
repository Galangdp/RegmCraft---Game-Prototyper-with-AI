#include "json/parser.h"

void json_parse_search_colon(json_pctx_t *pctx) {
    json_psst_t *psst = json_psst_stack_last(&pctx->psst_stack);

    while (TRUE) {
        char current_char = json_pctx_peek(pctx);

        switch (current_char) {
            case '\0':
                if (pctx->is_strict) {
                    json_pctx_set_error(pctx, JSON_PERR_EFPL);
                } else {
                    json_object_tset_null(psst->jobj, &pctx->temp_key);
                    json_pctx_set_success(pctx);
                }
                return;

            case ' ':
            case '\n':
                json_pctx_consume(pctx);
                break;

            case '{':
            case '[':
            case '"':
            case '+':
            case '-':
                if (pctx->is_strict) {
                    json_pctx_set_error(pctx, JSON_PERR_MSCL);
                } else {
                    json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                }
                return;

            case '}':
            case ']':
                if (pctx->is_strict) {
                    json_pctx_set_error(pctx, JSON_PERR_MVAL);
                } else {
                    json_object_tset_null(psst->jobj, &pctx->temp_key);
                    json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                }
                return;

            case ':':
                json_pctx_consume(pctx);
                json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                return;

            default:
                if (isalnum(current_char)) {
                    if (pctx->is_strict) {
                        json_pctx_set_error(pctx, JSON_PERR_MSCL);
                    } else {
                        json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                    }
                    return;
                }
    
                if (pctx->is_strict) {
                    json_pctx_set_error(pctx, JSON_PERR_ISYM);
                    return;
                }
    
                json_pctx_consume(pctx);
                break;
        }
    }
}