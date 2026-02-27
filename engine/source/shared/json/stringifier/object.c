#include "json/stringifier.h"

void json_stringify_object(json_sctx_t *sctx) {
    json_psst_t *psst = json_psst_stack_last(&sctx->psst_stack);
    json_object_t *jobj = psst->jobj;

    if (psst->optional_data >= jobj->length || psst->index >= jobj->capacity) {
        if (jobj->length == 0) {
            string_push_char(sctx->out, '{');
        }

        string_push_char(sctx->out, '}');
        json_psst_stack_pop(&sctx->psst_stack);

        psst = json_psst_stack_last(&sctx->psst_stack);

        if (psst == NULL) {
            json_sctx_set_success(sctx);
        } else {
            json_sctx_switch_mode(sctx, psst->is_array ? JSON_SMODE_ARRAY : JSON_SMODE_OBJECT);
        }

        return;
    }

    string_push_char(sctx->out, psst->index == 0 ? '{' : ',');

    for (; psst->index < jobj->capacity;) {
        json_object_entry_t *entry = jobj->entries + psst->index++;
        
        if (entry->state == JSON_OBJECT_ENTST_EMPTY) {
            continue;
        }

        if (entry->state == JSON_OBJECT_ENTST_TOMBSTONE) {
            psst->optional_data++;
            continue;
        }

        string_push_char(sctx->out, '"');
        string_push_cstrl(sctx->out, entry->key.cstr, entry->key.length);
        string_push_cstr(sctx->out, "\":");
        json_sctx_set_value(sctx, &entry->value);

        psst->optional_data++;
        break;
    }
}