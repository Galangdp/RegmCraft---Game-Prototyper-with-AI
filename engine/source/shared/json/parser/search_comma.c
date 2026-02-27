#include "json/parser.h"

void json_parse_search_comma(json_pctx_t *pctx) {
    json_psst_t *psst = json_psst_stack_last(&pctx->psst_stack);

    while (TRUE) {
        char current_char = json_pctx_peek(pctx);

        switch (current_char) {
            case '\0':
                if (pctx->is_strict) {
                    json_pctx_set_error(pctx, JSON_PERR_EFPM);
                } else {
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
                    json_pctx_set_error(pctx, JSON_PERR_MSCM);
                } else {
                    if (psst->is_array) {
                        json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                    } else {
                        json_pctx_set_recover(pctx, JSON_FBVAL_NONE);
                    }
                }
                return;

            case ',':
                json_pctx_consume(pctx);
                if (!psst->is_array) {
                    json_pctx_switch_mode(pctx, JSON_PMODE_KEY);
                    return;
                }

            case '}':
            case ']':
                json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                return;

            default:
                if (isalnum(current_char)) {
                    if (pctx->is_strict) {
                        json_pctx_set_error(pctx, JSON_PERR_MSCM);
                    } else {
                        if (psst->is_array) {
                            json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                        } else {
                            json_pctx_set_recover(pctx, JSON_FBVAL_NONE);
                        }
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