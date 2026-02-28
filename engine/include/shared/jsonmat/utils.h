#ifndef JSONMAT_UTILS_H
#define JSONMAT_UTILS_H

#include "jsonmat/const.h"
#include "jsonmat/hesxstring.h"
#include "jsonmat/jsonmat.h"
#include "jsonmat/pixel_bound.h"
#include "jsonmat/colorrp.h"
#include "material/utils.h"

extern string_map_t *jsonmatk_map;

status_t jsonmat_open_json(const char *path, json_t *json);

#endif /* JSONMAT_UTILS_H */ 