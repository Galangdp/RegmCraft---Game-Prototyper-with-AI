#include "json/parser.h"

status_t json_parse(json_t *json, string_t *in, bool_t is_strict) {
    json_pctx_t pctx = {
        .temp_key = {
            .cstr = NULL,
            .length = 0,
            .hash = 0
        },
        .in = in,
        .index = 0,
        .mode = JSON_PMODE_SEARCH_CONTMARK,
        .is_strict = is_strict,
        .retcode = EXIT_SUCCESS,
    };

    json_psst_stack_init(&pctx.psst_stack);
    string_init(&pctx.buffer);

    bool_t still_parsing = TRUE;

    while (still_parsing) {
        switch (pctx.mode) {
            case JSON_PMODE_ERROR: {
                json_psst_t *psst = json_psst_stack_first(&pctx.psst_stack);

                if (psst != NULL) {
                    if (psst->is_array) {
                        json_array_deinit(psst->jarr);
                    } else {
                        json_object_deinit(psst->jobj);
                    }
                    pctx.psst_stack.length = 0;
                }
            }

            case JSON_PMODE_SUCCESS:
                still_parsing = FALSE;
                break;
            
            case JSON_PMODE_VALUE: json_parse_value(&pctx); break;
            case JSON_PMODE_KEY: json_parse_key(&pctx); break;
            case JSON_PMODE_IDENTIFIER: json_parse_identifier(&pctx); break;
            case JSON_PMODE_STRING_KEY: json_parse_string_key(&pctx); break;
            case JSON_PMODE_STRING_VALUE: json_parse_string_value(&pctx); break;
            case JSON_PMODE_INT64: json_parse_int64(&pctx); break;
            case JSON_PMODE_DOUBLE: json_parse_double(&pctx); break;
            case JSON_PMODE_SEARCH_CONTMARK: json_parse_search_contmark(&pctx); break;
            case JSON_PMODE_SEARCH_COLON: json_parse_search_colon(&pctx); break;
            case JSON_PMODE_SEARCH_COMMA: json_parse_search_comma(&pctx); break;
            case JSON_PMODE_SEARCH_EOF: json_parse_search_eof(&pctx); break;
            case JSON_PMODE_RECOVERY: json_parse_recovery(&pctx); break;
        }
    }

    json_psst_t *psst = json_psst_stack_first(&pctx.psst_stack);

    if (psst == NULL) {
        json_init_object(json);
    } else {
        json->is_array = psst->is_array;
        json->jobj = psst->jobj;
    }

    string_deinit(&pctx.buffer);
    json_psst_stack_deinit(&pctx.psst_stack);

    return pctx.retcode;
}

