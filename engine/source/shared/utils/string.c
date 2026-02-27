#include "utils/string.h"

#define STRING_DEFCAP       256
#define STRING_PTRDIG       24

FORCE_INLINE void string_oresize(string_t *string, size_t length_inc) {
    if (string->length + length_inc >= string->capacity) {
        string->capacity <<= 1;
        string->cstr = memory_realloc(string->cstr, string->capacity);
    }
}

void string_init(string_t *string) {
    string->cstr = memory_alloc(STRING_DEFCAP);
    string->length = 0;
    string->capacity = STRING_DEFCAP;
}

void string_init_capacity(string_t *string, size_t capacity) {
    string->cstr = memory_alloc(capacity);
    string->length = 0;
    string->capacity = capacity;
}

void string_shrink(string_t *string) {
    if (string->capacity > string->length) {
        string->cstr = memory_realloc(string->cstr, string->length);
        string->capacity = string->length;
    }
}

void string_shrink_terminate(string_t *string) {
    size_t length = string->length + 1;

    string->cstr = memory_realloc(string->cstr, length);
    string->capacity = length;
    string->cstr[string->length] = '\0';
}

void string_terminate(string_t *string) {
    string_oresize(string, 1);
    string->cstr[string->length] = '\0';
}

void string_from_cstr(string_t *string, const char *cstr) {
    string->capacity = strlen(cstr);
    string->length = string->capacity;
    string->cstr = memory_alloc(string->capacity);

    memcpy(string->cstr, cstr, string->capacity);
}

void string_from_cstrl(string_t *string, const char *cstr, size_t length) {
    string->capacity = length;
    string->length = string->capacity;
    string->cstr = memory_alloc(string->capacity);

    memcpy(string->cstr, cstr, string->capacity);
}

void string_from_string(string_t *string, string_t const *src) {
    string->cstr = memory_alloc(src->capacity);
    string->length = src->length;
    string->capacity = string->capacity;
    
    memcpy(string->cstr, src->cstr, src->length);
}

void string_push_char(string_t *string, char ch) {
    string_oresize(string, 1);
    string->cstr[string->length++] = ch;
}

void string_push_chseq(string_t *string, char ch, size_t count) {
    string_oresize(string, count);
    memset(string->cstr + string->length, ch, count);
    
    string->length += count;
}

void string_push_string(string_t *string, const string_t *src) {
    string_oresize(string, src->length);
    memcpy(string->cstr + string->length, src->cstr, src->length);
    
    string->length += src->length;
}

void string_push_cstr(string_t *string, const char *cstr) {
    size_t length = strlen(cstr);

    string_oresize(string, length);
    memcpy(string->cstr + string->length, cstr, length);

    string->length += length;
}

void string_push_cstrl(string_t *string, const char *cstr, size_t length) {
    string_oresize(string, length);
    memcpy(string->cstr + string->length, cstr, length);

    string->length += length;
}

void string_push_uint64(string_t *string, uint64_t value) {
    string_oresize(string, STRING_MAX_NUMDIG);
    int result = snprintf(string->cstr + string->length, STRING_MAX_NUMDIG, "%lu", value);
    string->length += (size_t) result;
}

void string_push_int64(string_t *string, int64_t value) {
    string_oresize(string, STRING_MAX_NUMDIG);
    int result = snprintf(string->cstr + string->length, STRING_MAX_NUMDIG, "%ld", value);
    string->length += (size_t) result;
}

void string_push_double(string_t *string, double value) {
    string_oresize(string, STRING_MAX_NUMDIG);
    int result = snprintf(string->cstr + string->length, STRING_MAX_NUMDIG, "%f", value);
    string->length += (size_t) result;
}

void string_push_pointer(string_t *string, const void *const ptr) {
    string_oresize(string, STRING_PTRDIG);
    int result = snprintf(string->cstr + string->length, STRING_PTRDIG, "%p", ptr);
    string->length += (size_t) result;
}

void string_hash_from_string(string_hash_t *string_hash, const string_t *string) {
    string_hash->cstr = memory_alloc(string->length);
    string_hash->length = string->length;
    string_hash->hash = fnv1amix((const uint8_t *) string->cstr, string->length);

    memcpy(string_hash->cstr, string->cstr, string->length);
}

void string_view_from_string(string_view_t *string_view, const string_t *string) {
    string_view->cstr = memory_alloc(string->length);
    string_view->length = string->length;
    
    memcpy(string_view->cstr, string->cstr, string->length);
}
