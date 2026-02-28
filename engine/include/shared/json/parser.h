#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <ctype.h>
#include "json/const.h"
#include "json/json.h"
#include "json/psst.h"

typedef enum json_pmode {
    JSON_PMODE_ERROR,
    JSON_PMODE_SUCCESS,
    JSON_PMODE_VALUE,
    JSON_PMODE_KEY,
    JSON_PMODE_IDENTIFIER,
    JSON_PMODE_STRING_KEY,
    JSON_PMODE_STRING_VALUE,
    JSON_PMODE_INT64,
    JSON_PMODE_DOUBLE,
    JSON_PMODE_SEARCH_CONTMARK,
    JSON_PMODE_SEARCH_COLON,
    JSON_PMODE_SEARCH_COMMA,
    JSON_PMODE_SEARCH_EOF,
    JSON_PMODE_RECOVERY,
} json_pmode_t;

typedef enum json_fbval {
    JSON_FBVAL_NONE,
    JSON_FBVAL_NULL,
    JSON_FBVAL_INT64,
    JSON_FBVAL_DOUBLE,
} json_fbval_t;

typedef struct json_pctx {
    json_psst_stack_t psst_stack;
    string_t buffer;
    string_hash_t temp_key;
    string_t *in;
    size_t index;
    json_pmode_t mode;
    json_fbval_t fbval;
    status_t retcode;
    bool_t is_strict;
} json_pctx_t;

extern arena_t *json_arena;
extern json_idmap_t *json_idmap;

FORCE_INLINE char json_pctx_peek(json_pctx_t *pctx) {
    return string_at(pctx->in, pctx->index);
}

FORCE_INLINE char json_pctx_peek_offset(json_pctx_t *pctx, size_t offset) {
    return string_at(pctx->in, pctx->index + offset);
}

FORCE_INLINE char json_pctx_peek_n_consume(json_pctx_t *pctx) {
    return string_at(pctx->in, pctx->index++);
}

FORCE_INLINE void json_pctx_consume(json_pctx_t *pctx) {
    pctx->index++;
}

FORCE_INLINE void json_pctx_consume_offset(json_pctx_t *pctx, size_t offset) {
    pctx->index += offset;
}

FORCE_INLINE void json_pctx_switch_mode(json_pctx_t *pctx, json_pmode_t mode) {
    pctx->mode = mode;
}

FORCE_INLINE void json_pctx_set_recover(json_pctx_t *pctx, json_fbval_t fbval) {
    pctx->mode = JSON_PMODE_RECOVERY;
    pctx->fbval = fbval;
}

FORCE_INLINE void json_pctx_set_error(json_pctx_t *pctx, status_t errcode) {
    pctx->mode = JSON_PMODE_ERROR;
    pctx->retcode = errcode;
}

FORCE_INLINE void json_pctx_set_success(json_pctx_t *pctx) {
    pctx->mode = JSON_PMODE_SUCCESS;
}

FORCE_INLINE void json_pctx_buffer_push_char(json_pctx_t *pctx, char ch) {
    string_push_char(&pctx->buffer, ch);
}

FORCE_INLINE void json_pctx_buffer_clear(json_pctx_t *pctx) {
    string_clear(&pctx->buffer);
}

FORCE_INLINE void json_pctx_temp_key_update(json_pctx_t *pctx, uint64_t hash) {
    pctx->temp_key.cstr = arena_alloc(json_arena, pctx->buffer.length);
    pctx->temp_key.length = pctx->buffer.length;
    pctx->temp_key.hash = hash;

    memcpy(pctx->temp_key.cstr, pctx->buffer.cstr, pctx->buffer.length);
}

FORCE_INLINE bool_t json_pctx_resolve_cseq(char *ch) {
    switch (*ch) {
        case '0': *ch = '\0'; break;
        case '1': *ch = '\1'; break;
        case '2': *ch = '\2'; break;
        case '3': *ch = '\3'; break;
        case '4': *ch = '\4'; break;
        case '5': *ch = '\5'; break;
        case '6': *ch = '\6'; break;
        case '7': *ch = '\7'; break;
        case 'a': *ch = '\a'; break;
        case 'b': *ch = '\b'; break;
        case 't': *ch = '\t'; break;
        case 'n': *ch = '\n'; break;
        case 'v': *ch = '\v'; break;
        case 'f': *ch = '\f'; break;
        case 'r': *ch = '\r'; break;
        case '"': *ch = '"'; break;
        default: return FALSE;
    }

    return TRUE;
}

void json_parse_value(json_pctx_t *pctx);
void json_parse_key(json_pctx_t *pctx);
void json_parse_identifier(json_pctx_t *pctx);
void json_parse_string_key(json_pctx_t *pctx);
void json_parse_string_value(json_pctx_t *pctx);
void json_parse_int64(json_pctx_t *pctx);
void json_parse_double(json_pctx_t *pctx);
void json_parse_search_contmark(json_pctx_t *pctx);
void json_parse_search_colon(json_pctx_t *pctx);
void json_parse_search_comma(json_pctx_t *pctx);
void json_parse_search_eof(json_pctx_t *pctx);
void json_parse_recovery(json_pctx_t *pctx);

#endif /* JSON_PARSER_H */ 