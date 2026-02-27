#include "json/parser.h"

void json_parse_value(json_pctx_t *pctx) {
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
                } else {
                    json_pctx_set_success(pctx);
                }
                return;
            
            case '"':
                json_pctx_consume(pctx);
                json_pctx_switch_mode(pctx, JSON_PMODE_STRING_VALUE);
                return;
            
            case '{': {
                json_object_t *jobj = NULL;

                if (psst == NULL) {
                    jobj = arena_alloc(json_arena, sizeof(json_object_t));
                    json_object_init(jobj);
                } else {
                    jobj = psst->is_array ?
                        json_array_push_empty_object(psst->jarr) :
                        json_object_tset_empty_object(psst->jobj, &pctx->temp_key);
                }
    
                json_psst_stack_push_object(&pctx->psst_stack, jobj);
                json_pctx_consume(pctx);
                json_pctx_switch_mode(pctx, JSON_PMODE_KEY);
                return;
            }
            
            case '[': {
                json_array_t *jarr = NULL;

                if (psst == NULL) {
                    jarr = arena_alloc(json_arena, sizeof(json_array_t));
                    json_array_init(jarr);
                } else {
                    jarr = psst->is_array ?
                        json_array_push_empty_array(psst->jarr) :
                        json_object_tset_empty_array(psst->jobj, &pctx->temp_key);
                }
                
                json_psst_stack_push_array(&pctx->psst_stack, jarr);
                json_pctx_consume(pctx);

                psst = json_psst_stack_last(&pctx->psst_stack);
                break;
            }
    
            case '}': {
                if (pctx->is_strict && psst->is_array) {
                    json_pctx_set_error(pctx, JSON_PERR_BCMS);
                    return;
                }
    
                json_pctx_consume(pctx);
    
                if (pctx->psst_stack.length == 1) {
                    json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_EOF);
                    return;
                }

                json_psst_stack_pop(&pctx->psst_stack);
                json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_COMMA);
                return;
            }
    
            case ']': {
                if (pctx->is_strict && !psst->is_array) {
                    json_pctx_set_error(pctx, JSON_PERR_BTMS);
                    return;
                }
    
                json_pctx_consume(pctx);
                
                if (pctx->psst_stack.length == 1) {
                    json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_EOF);
                    return;
                }
                
                json_psst_stack_pop(&pctx->psst_stack);
                json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_COMMA);
                return;
            }

            case '+':
            case '-':
                json_pctx_buffer_push_char(pctx, current_char);
                json_pctx_consume(pctx);
                json_pctx_switch_mode(pctx, JSON_PMODE_INT64);
                return;
    
            default:
                if (isalpha(current_char)) {
                    json_pctx_switch_mode(pctx, JSON_PMODE_IDENTIFIER);
                    return;
                }
    
                if (isdigit(current_char)) {
                    json_pctx_switch_mode(pctx, JSON_PMODE_INT64);
                    return;
                }
    
                if (pctx->is_strict) {
                    printf("%c\n", current_char);
                    json_pctx_set_error(pctx, JSON_PERR_ISYM);
                    return;
                }
    
                json_pctx_consume(pctx);
                break;
        }
    }
}