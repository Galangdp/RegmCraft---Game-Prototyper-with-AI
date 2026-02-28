#ifndef JSONMAT_COLORRP_H
#define JSONMAT_COLORRP_H

#include "material/color.h"

typedef struct colorrp_entry {
    color_t from;
    color_t to;
} colorrp_entry_t;

typedef struct colorrp {
    colorrp_entry_t *entries;
    size_t length;
} colorrp_t;

void colorrp_init(colorrp_t *colorrp, size_t total_color);
void colorrp_push(colorrp_t *colorrp, const color_t* from, const color_t *to);
color_t *colorrp_search(colorrp_t *colorrp, uint8_t red, uint8_t green, uint8_t blue);
void colorrp_deinit(colorrp_t *colorrp);

#endif /* JSONMAT_COLORRP_H */ 