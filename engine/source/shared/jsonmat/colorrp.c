#include "jsonmat/colorrp.h"

#define COLORRP_ELEMSIZE    sizeof(colorrp_entry_t)

void colorrp_init(colorrp_t *colorrp, size_t total_color) {
    colorrp->entries = memory_alloc(total_color * COLORRP_ELEMSIZE);
    colorrp->length = 0;
}

void colorrp_push(colorrp_t *colorrp, const color_t* from, const color_t *to) {
    colorrp_entry_t *entry = colorrp->entries + colorrp->length++;

    memcpy(&entry->from, from, sizeof(color_t));
    memcpy(&entry->to, to, sizeof(color_t));
}

color_t *colorrp_search(colorrp_t *colorrp, uint8_t red, uint8_t green, uint8_t blue) {
    for (size_t i = 0; i < colorrp->length; i++) {
        colorrp_entry_t *entry = colorrp->entries + i;
        color_t *from = &entry->from;

        if (from->red == red && from->green == green && from->blue == blue) {
            return &entry->to;
        }
    }

    return NULL;
}

void colorrp_deinit(colorrp_t *colorrp) {
    memory_free(colorrp->entries);
}