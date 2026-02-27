#ifndef JSON_JSON_H
#define JSON_JSON_H

#include "utils/arena.h"
#include "json/array.h"
#include "json/idmap.h"
#include "json/object.h"

#define JSON_PERR_MBRT              -1      /* Missing bracket */
#define JSON_PERR_MBRC              -2      /* Missing brace */
#define JSON_PERR_BTTM              -3      /* Bracket is too much */
#define JSON_PERR_BCTM              -4      /* Brace is too much */
#define JSON_PERR_BTMS              -5      /* Bracket mismatch expect brace */
#define JSON_PERR_BCMS              -6      /* Brace mismatch expect bracket */
#define JSON_PERR_ISYM              -7      /* Invalid symbol */
#define JSON_PERR_MKEY              -8      /* Missing object key */
#define JSON_PERR_IDIN              -9      /* Invalid char for identifier */
#define JSON_PERR_IDUN              -10     /* Unknown identifier */
#define JSON_PERR_VMPS              -11     /* Primitive value in wrong place */
#define JSON_PERR_MSQE              -12     /* Missing string quot end */
#define JSON_PERR_ISCS              -13     /* Invalid string char sequence */
#define JSON_PERR_EKEY              -14     /* Key is empty string */
#define JSON_PERR_DKEY              -15     /* Key duplicate */
#define JSON_PERR_MHDD              -16     /* Missing heading digit of double */
#define JSON_PERR_MTDD              -17     /* Missing trailing digit of double */
#define JSON_PERR_IIDG              -18     /* Invalid int64 digit */
#define JSON_PERR_MIDG              -19     /* Missing int64 digit */
#define JSON_PERR_IDDG              -20     /* Invalid double digit */
#define JSON_PERR_TIDG              -21     /* Digit is too long in int64 */
#define JSON_PERR_TDDG              -22     /* Digit is too long in double */
#define JSON_PERR_CMPS              -23     /* Container e.g { and [ in wrong place */
#define JSON_PERR_EFPL              -25     /* EOF at wrong place dircetly after object key */
#define JSON_PERR_EFPM              -26     /* EOF at wrong place dircetly after value */
#define JSON_PERR_MSCL              -27     /* Missing colon after object key */
#define JSON_PERR_MSCM              -28     /* Missing comma after value */
#define JSON_PERR_MVAL              -29     /* Missing value after object key */

typedef struct json {
    union {
        json_object_t *jobj;
        json_array_t *jarr;
    };
    bool_t is_array;
} json_t;

void json_ctx_set_arena(arena_t *arena);
void json_ctx_set_idmap(json_idmap_t *idmap);

void json_init_object(json_t *json);
void json_init_array(json_t *json);
void json_deinit(json_t *json);

status_t json_parse(json_t *json, string_t *in, bool_t is_strict);
void json_stringify(json_t *json, string_t *out);

#ifdef RC_DEBUG
void json_dump(json_t *json, string_t *out);
#endif

#endif /* JSON_JSON_H */ 