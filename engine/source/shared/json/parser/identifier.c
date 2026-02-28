#include "json/parser.h"

void json_parse_identifier(json_pctx_t *pctx) {
    uint64_t hash;

    fnv1amix_stream_start(&hash);

    while (TRUE) {
        char current_char = json_pctx_peek(pctx);

        if (isalpha(current_char)) {
            json_pctx_consume(pctx);
            json_pctx_buffer_push_char(pctx, current_char);
            fnv1amix_stream_consume(&hash, current_char);
            continue;
        }

        if (current_char == '_' || isdigit(current_char)) {
            if (pctx->is_strict) {
                json_pctx_set_error(pctx, JSON_PERR_IDIN);
            } else {
                json_pctx_set_recover(pctx, JSON_FBVAL_NULL);
            }
            json_pctx_buffer_clear(pctx);
            return;
        }

        break;
    }

    fnv1amix_stream_end(&hash);

    json_id_t id = json_idmap_search(json_idmap, pctx->buffer.cstr, pctx->buffer.length, hash);

    if (id == JSON_ID_UNKNOWN) {
        if (pctx->is_strict) {
            json_pctx_set_error(pctx, JSON_PERR_IDUN);
            json_pctx_buffer_clear(pctx);
            return;
        }
        id = JSON_ID_NULL;
    }

    json_psst_t *psst = json_psst_stack_last(&pctx->psst_stack);

    if (psst->is_array) {
        if (id == JSON_ID_NULL) {
            json_array_push_null(psst->jarr);
        } else {
            json_array_push_bool(psst->jarr, id == JSON_ID_TRUE);
        }
    } else {
        if (id == JSON_ID_NULL) {
            json_object_tset_null(psst->jobj, &pctx->temp_key);
        } else {
            json_object_tset_bool(psst->jobj, &pctx->temp_key, id == JSON_ID_TRUE);
        }
    }
    
    json_pctx_buffer_clear(pctx);
    json_pctx_switch_mode(pctx, JSON_PMODE_SEARCH_COMMA);
}