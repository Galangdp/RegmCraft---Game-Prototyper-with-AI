#include "json/parser.h"

void json_parse_key(json_pctx_t *pctx) {
    json_psst_t *psst = json_psst_stack_last(&pctx->psst_stack);
    
    while (TRUE) {
        char current_char = json_pctx_peek(pctx);
        
        switch (current_char) {
            case ' ':
            case '\n':
                json_pctx_consume(pctx);
                break;
    
            case '\0':
                if (pctx->is_strict && psst != NULL) {
                    json_pctx_set_error(pctx, psst->is_array ? JSON_PERR_MBRT : JSON_PERR_MBRC);
                    return;
                }
                json_pctx_set_success(pctx);
                return;
            
            case '"':
                json_pctx_consume(pctx);
                json_pctx_switch_mode(pctx, JSON_PMODE_STRING_KEY);
                return;
            
            case '{':
            case '[':
            case '+':
            case '-':
                if (pctx->is_strict) {
                    json_pctx_set_error(pctx, JSON_PERR_MKEY);
                    return;
                }
                json_pctx_set_recover(pctx, JSON_FBVAL_NONE);
                return;
    
            case '}': {
                json_pctx_consume(pctx);
                
                if (pctx->psst_stack.length == 1) {
                    json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_EOF);
                    return;
                }
                
                json_psst_stack_pop(&pctx->psst_stack);
                
                if (!(psst = json_psst_stack_last(&pctx->psst_stack))->is_array) {
                    break;
                }

                json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                return;
            }
    
            case ']': {
                if (pctx->is_strict) {
                    json_pctx_set_error(pctx, JSON_PERR_BTMS);
                    return;
                }
    
                json_pctx_consume(pctx);
                
                if (pctx->psst_stack.length == 1) {
                    json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_EOF);
                    return;
                }
                
                json_psst_stack_pop(&pctx->psst_stack);
                
                if (!(psst = json_psst_stack_last(&pctx->psst_stack))->is_array) {
                    break;
                }

                json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                return;
            }
    
            default:
                if (isalnum(current_char)) {
                    if (pctx->is_strict) {
                        json_pctx_set_error(pctx, JSON_PERR_MKEY);
                        return;
                    }
    
                    json_pctx_consume(pctx);
                    json_pctx_set_recover(pctx, JSON_FBVAL_NONE);
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