#include "json/parser.h"

void json_parse_string_key(json_pctx_t *pctx) {
    uint64_t hash;

    fnv1amix_stream_start(&hash);

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
        fnv1amix_stream_consume(&hash, current_char);
    }

    if (pctx->buffer.length == 0) {
        if (pctx->is_strict) {
            json_pctx_set_error(pctx, JSON_PERR_EKEY);
            return;
        }
        json_pctx_buffer_clear(pctx);
        json_pctx_set_recover(pctx, JSON_FBVAL_NONE);
        return;
    }

    fnv1amix_stream_end(&hash);
    json_pctx_temp_key_update(pctx, hash);
    
    if (pctx->is_strict && json_object_has(json_psst_stack_last(&pctx->psst_stack)->jobj, &pctx->temp_key)) {
        json_pctx_set_error(pctx, JSON_PERR_DKEY);
        return;
    }

    json_pctx_buffer_clear(pctx);
    json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_COLON);
}