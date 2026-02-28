#ifndef MATERIAL_COLOR_STAT_H
#define MATERIAL_COLOR_STAT_H

#include "material/color.h"
#include "utils/hash.h"

typedef struct color_stat {
    string_hash_t string;
    color_semt_t semt;
    color_semp_t semp;
    bool_t is_required;
    bool_t is_family;
} color_stat_t;

typedef color_stat_t *color_stats_t;

void color_stats_init(color_stats_t *stat);

#endif /* MATERIAL_COLOR_STAT_H */ 