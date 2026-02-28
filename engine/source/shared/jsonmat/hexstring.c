#include "jsonmat/hesxstring.h"

FORCE_INLINE status_t hexstring_chars_to_uint8(char ch0, char ch1, uint8_t *result) {
    *result = 0;

    if (ch0 >= '0' && ch0 <= '9') {
        *result |= (ch0 - '0') << 4;
    } else if (ch0 >= 'a' && ch0 <= 'f') {
        *result |= (ch0 - 'a' + 10) << 4;
    } else if (ch0 >= 'A' && ch0 <= 'F') {
        *result |= (ch0 - 'A' + 10) << 4;
    } else {
        return EXIT_FAILURE;
    }

    if (ch1 >= '0' && ch1 <= '9') {
        *result |= ch1 - '0';
    } else if (ch1 >= 'a' && ch1 <= 'f') {
        *result |= ch1 - 'a' + 10;
    } else if (ch1 >= 'A' && ch1 <= 'F') {
        *result |= ch1 - 'A' + 10;
    } else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

status_t hexstring_to_color(const string_view_t *string, color_t *color) {
    if (string->length != 7 || string->cstr[0] != '#') {
        return EXIT_FAILURE;
    }

    const char *chp = string->cstr + 1;

    if (hexstring_chars_to_uint8(chp[0], chp[1], &color->red) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    chp += 2;

    if (hexstring_chars_to_uint8(chp[0], chp[1], &color->green) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    chp += 2;

    if (hexstring_chars_to_uint8(chp[0], chp[1], &color->blue) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void hexstring_from_color(string_view_t *string, const color_t *color);