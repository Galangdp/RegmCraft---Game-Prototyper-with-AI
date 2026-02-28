#include "utils/file.h"

status_t file_open(file_t *file, const char *path, const char *mode) {
    FILE *fp = fopen(path, mode);

    if (fp == NULL) {
        return FILE_ERR_OPEN;
    }

    if (fseek(fp, 0, SEEK_END) != EXIT_SUCCESS) {
        fclose(fp);
        return FILE_ERR_SEEK;
    }

    if ((file->length = ftell(fp)) == -1) {
        fclose(fp);
        return FILE_ERR_TELL;
    }

    if (fseek(fp, 0, SEEK_SET) != EXIT_SUCCESS) {
        fclose(fp);
        return FILE_ERR_SEEK;
    }

    file->fp = fp;
    file->path = path;

    return EXIT_SUCCESS;
}

status_t file_read_byte(file_t *file, uint8_t *buffer) {
#ifdef RC_DEBUG
    if (file->fp == NULL) {
        ASSERT(EXIT_FAILURE, "Error read. File pointer is null, forget to open?");
    }
#endif
    size_t bytes_read = fread(buffer, 1, file->length, file->fp);
    return bytes_read < file->length ? FILE_ERR_READ : EXIT_SUCCESS;
}

status_t file_read_string(file_t *file, string_t *buffer) {
#ifdef RC_DEBUG
    if (file->fp == NULL) {
        ASSERT(EXIT_FAILURE, "Error read. File pointer is null, forget to open?");
    }
#endif
    size_t bytes_read = fread(buffer->cstr, 1, file->length, file->fp);
    buffer->length = bytes_read;
    return bytes_read < file->length ? FILE_ERR_READ : EXIT_SUCCESS;
}

status_t file_write_byte(file_t *file, const uint8_t *buffer, size_t length) {
#ifdef RC_DEBUG
    if (file->fp == NULL) {
        ASSERT(EXIT_FAILURE, "Error write. File pointer is null, forget to open?");
    }
#endif
    size_t bytes_written = fwrite(buffer, 1, length, file->fp);
    return bytes_written < length ? FILE_ERR_WRIT : EXIT_SUCCESS;
}

status_t file_write_string(file_t *file, const string_t *buffer) {
#ifdef RC_DEBUG
    if (file->fp == NULL) {
        ASSERT(EXIT_FAILURE, "Error write. File pointer is null, forget to open?");
    }
#endif
    size_t bytes_written = fwrite(buffer->cstr, 1, buffer->length, file->fp);
    return bytes_written < buffer->length ? FILE_ERR_WRIT : EXIT_SUCCESS;
}

void file_close(file_t *file) {
#if RC_DEBUG
    if (file->fp == NULL) {
        ASSERT(EXIT_FAILURE, "Error close. File pointer is null, forget to open?");
    }
#endif
    fclose(file->fp);
#if RC_DEBUG
    file->fp = NULL;
#endif
}