#ifndef JSONMAT_HEXSTRING_H
#define JSONMAT_HEXSTRING_H

#include "material/color.h"

status_t hexstring_to_color(const string_view_t *string, color_t *color);
void hexstring_from_color(string_view_t *string, const color_t *color);

#endif /* JSONMAT_HEXSTRING_H */ 