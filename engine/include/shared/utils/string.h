#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include "utils/hash.h"
#include "utils/memory.h"

#define STRING_MAX_NUMDIG   64

typedef struct string {
    char *cstr;
    size_t length;
    size_t capacity;
} string_t;

typedef struct string_hash {
    char *cstr;
    size_t length;
    uint64_t hash;
} string_hash_t;

typedef struct string_view {
    char *cstr;
    size_t length;
} string_view_t;

void string_init(string_t *string);
void string_init_capacity(string_t *string, size_t capacity);
FORCE_INLINE void string_deinit(string_t *string);
void string_shrink(string_t *string);
void string_shrink_terminate(string_t *string);
void string_terminate(string_t *string);
void string_from_cstr(string_t *string, const char *cstr);
void string_from_cstrl(string_t *string, const char *cstr, size_t length);
void string_from_string(string_t *string, string_t const *src);
FORCE_INLINE char string_at(string_t *string, size_t index);
FORCE_INLINE char string_atuns(string_t *string, size_t index);
FORCE_INLINE void string_print(string_t *string);
FORCE_INLINE void string_println(string_t *string);
FORCE_INLINE void string_clear(string_t *string);
void string_push_char(string_t *string, char ch);
void string_push_chseq(string_t *string, char ch, size_t count);
void string_push_string(string_t *string, const string_t *src);
void string_push_cstr(string_t *string, const char *cstr);
void string_push_cstrl(string_t *string, const char *cstr, size_t length);
void string_push_uint64(string_t *string, uint64_t value);
void string_push_int64(string_t *string, int64_t value);
void string_push_double(string_t *string, double value);
void string_push_pointer(string_t *string, const void *const ptr);
FORCE_INLINE int64_t string_to_int64(string_t *string, uint8_t base);
FORCE_INLINE double string_to_double(string_t *string);

void string_hash_from_string(string_hash_t *string_hash, const string_t *string);
FORCE_INLINE void string_hash_print(string_hash_t *string_hash);
FORCE_INLINE void string_hash_println(string_hash_t *string_hash);

void string_view_from_string(string_view_t *string_view, const string_t *string);
FORCE_INLINE void string_view_print(string_view_t *string_view);
FORCE_INLINE void string_view_println(string_view_t *string_view);

FORCE_INLINE void string_deinit(string_t *string) {
    memory_free(string->cstr);
}

FORCE_INLINE char string_at(string_t *string, size_t index) {
    return index >= string->length ? '\0' : string->cstr[index];
}

FORCE_INLINE char string_atuns(string_t *string, size_t index) {
    return string->cstr[index];
}

FORCE_INLINE void string_print(string_t *string) {
    printf("%.*s", (int) string->length, string->cstr);
}

FORCE_INLINE void string_println(string_t *string) {
    printf("%.*s\n", (int) string->length, string->cstr);
}

FORCE_INLINE void string_clear(string_t *string) {
    string->length = 0;
}

FORCE_INLINE uint64_t string_to_uint64(string_t *string, uint8_t base) {
    return strtoull(string->cstr, NULL, base);
}

FORCE_INLINE int64_t string_to_int64(string_t *string, uint8_t base) {
    return strtoll(string->cstr, NULL, base);
}

FORCE_INLINE double string_to_double(string_t *string) {
    return strtod(string->cstr, NULL);
}

FORCE_INLINE void string_hash_print(string_hash_t *string_hash) {
    printf("%.*s", (int) string_hash->length, string_hash->cstr);
}

FORCE_INLINE void string_hash_println(string_hash_t *string_hash) {
    printf("%.*s\n", (int) string_hash->length, string_hash->cstr);
}

FORCE_INLINE void string_view_print(string_view_t *string_view) {
    printf("%.*s", (int) string_view->length, string_view->cstr);
}

FORCE_INLINE void string_view_println(string_view_t *string_view) {
    printf("%.*s\n", (int) string_view->length, string_view->cstr);
}

#endif /* UTILS_STRING_H */ 