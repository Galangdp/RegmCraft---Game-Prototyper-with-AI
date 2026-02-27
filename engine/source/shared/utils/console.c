#include "utils/console.h"

status_t console_read(string_t *out) {
    uint8_t buffer[sizeof(uint32_t)] = {};
    size_t counter = 0;

    while (counter < sizeof(uint32_t)) {
        size_t res = fread(buffer + counter, 1, sizeof(uint32_t) - counter, stdin);

        counter += res;

        if (res == 0) {
            break;
        }
    }

    if (counter != sizeof(uint32_t)) {
        return EXIT_FAILURE;
    }

    uint32_t message_length = read_le32(buffer, 0);

    out->cstr = memory_alloc(message_length);
    out->length = message_length;
    out->capacity = message_length;

    counter = 0;

    while (counter < message_length) {
        size_t res = fread(out->cstr + counter, 1, message_length - counter, stdin);

        counter += res;

        if (res == 0) {
            break;
        }
    }

    if (counter != message_length) {
        memory_free(out->cstr);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

status_t console_write(const string_t *in) {
    uint8_t buffer[sizeof(uint32_t)] = {};
    size_t counter = 0;

    write_le32(buffer, 0, in->length);

    while (counter < sizeof(uint32_t)) {
        size_t res = fwrite(buffer + counter, 1, sizeof(uint32_t) - counter, stdout);

        counter += res;

        if (res == 0) {
            break;
        }
    }

    if (counter != sizeof(uint32_t)) {
        return EXIT_FAILURE;
    }

    counter = 0;

    while (counter < in->length) {
        size_t res = fwrite(in->cstr + counter, 1, in->length - counter, stdout);

        counter += res;

        if (res == 0) {
            break;
        }
    }

    fflush(stdout);
    return counter != in->length ? EXIT_FAILURE : EXIT_SUCCESS;
}

status_t console_write_cstr(const char *cstr) {
    uint8_t buffer[sizeof(uint32_t)] = {};
    size_t counter = 0;
    size_t length = strlen(cstr);

    write_le32(buffer, 0, length);

    while (counter < sizeof(uint32_t)) {
        size_t res = fwrite(buffer + counter, 1, sizeof(uint32_t) - counter, stdout);

        counter += res;

        if (res == 0) {
            break;
        }
    }

    if (counter != sizeof(uint32_t)) {
        return EXIT_FAILURE;
    }

    counter = 0;

    while (counter < length) {
        size_t res = fwrite(cstr + counter, 1, length - counter, stdout);

        counter += res;

        if (res == 0) {
            break;
        }
    }

    fflush(stdout);
    return counter != length ? EXIT_FAILURE : EXIT_SUCCESS;
}