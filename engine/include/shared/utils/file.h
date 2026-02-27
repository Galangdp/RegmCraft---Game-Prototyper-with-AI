#ifndef UTILS_FILE_H
#define UTILS_FILE_H

#include <stdio.h>
#include "utils/string.h"

#define FILE_OMODE_RD           "rb"
#define FILE_OMODE_WR           "wb"
#define FILE_OMODE_RDWR         "wb+"

#define FILE_ERR_READ           -1
#define FILE_ERR_WRIT           -2
#define FILE_ERR_SEEK           -3
#define FILE_ERR_OPEN           -4
#define FILE_ERR_TELL           -5

typedef struct file {
    FILE *fp;
    const char *path;
    size_t length;
} file_t;

status_t file_open(file_t *file, const char *path, const char *mode);
status_t file_read_byte(file_t *file, uint8_t *buffer);
status_t file_read_string(file_t *file, string_t *buffer);
status_t file_write_byte(file_t *file, const uint8_t *buffer, size_t length);
status_t file_write_string(file_t *file, const string_t *buffer);
void file_close(file_t *file);

#endif /* UTILS_FILE_H */ 