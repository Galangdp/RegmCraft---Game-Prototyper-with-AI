#include "json/parser.h"

void json_parse_search_contmark(json_pctx_t *pctx) {
    if (!pctx->is_strict) {
        while (TRUE) {
            char current_char = json_pctx_peek(pctx);

            if (current_char == '\0') {
                json_pctx_set_success(pctx);
                return;
            }

            if (current_char == '{' || current_char == '[') {
                json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                return;
            }

            json_pctx_consume(pctx);
        }
    } else {
        while (TRUE) {
            char current_char = json_pctx_peek(pctx);

            switch (current_char) {
                case '\0':
                    json_pctx_set_success(pctx);
                    return;

                case ' ':
                case '\n':
                    json_pctx_consume(pctx);
                    break;
                
                case '{':
                case '[':
                    json_pctx_switch_mode(pctx, JSON_PMODE_VALUE);
                    return;

                case '}':
                    json_pctx_set_error(pctx, JSON_PERR_BCTM);
                    return;

                case ']':
                    json_pctx_set_error(pctx, JSON_PERR_BTTM);
                    return;

                case '"':
                case '+':
                case '-':
                    json_pctx_set_error(pctx, JSON_PERR_VMPS);
                    return;

                default:
                    json_pctx_set_error(pctx, isalnum(current_char) ? JSON_PERR_VMPS : JSON_PERR_ISYM);
                    return;
            }
        }
    }
}