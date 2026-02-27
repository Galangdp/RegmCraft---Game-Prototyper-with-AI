#include "json/stringifier.h"

void json_stringify(json_t *json, string_t *out) {
    json_sctx_t sctx = {
        .out = out
    };

    json_psst_stack_init(&sctx.psst_stack);

    if (json->is_array) {
        json_psst_stack_push_array(&sctx.psst_stack, json->jarr);
        sctx.mode = JSON_SMODE_ARRAY;
    } else {
        json_psst_stack_push_object(&sctx.psst_stack, json->jobj);
        sctx.mode = JSON_SMODE_OBJECT;
    }

    bool_t still_parsing = TRUE;

    while (still_parsing) {
        switch (sctx.mode) {
            case JSON_SMODE_SUCCESS:
                still_parsing = FALSE;
                break;

            case JSON_SMODE_OBJECT: json_stringify_object(&sctx); break;
            case JSON_SMODE_ARRAY: json_stringify_array(&sctx); break;
            case JSON_SMODE_VALUE: json_stringify_value(&sctx); break;
                break;
        }
    }
    
    json_psst_stack_deinit(&sctx.psst_stack);
}