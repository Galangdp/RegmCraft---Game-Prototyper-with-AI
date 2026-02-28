#ifndef UTILS_CONSOLE_H
#define UTILS_CONSOLE_H

#include "utils/string.h"
#include "utils/utils.h"

status_t console_read(string_t *out);
status_t console_write(const string_t *in);
status_t console_write_cstr(const char *cstr);

#endif /* UTILS_CONSOLE_H */ 