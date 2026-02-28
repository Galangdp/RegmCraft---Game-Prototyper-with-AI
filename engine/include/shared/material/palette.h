#ifndef MATERIAL_PALETTE_H
#define MATERIAL_PALETTE_H

#include "material/color.h"
#include "jsonmat/hesxstring.h"

typedef struct palette {
    string_view_t name;
    struct {
        color_t *entries;
        size_t length;
    } colors;
    colorfam_t colorfams[COLOR_SEM_TOTAL];
} palette_t;

void palette_init(palette_t *palette, size_t total_colors);
void palette_set_name(palette_t *palette, const string_view_t *name);
FORCE_INLINE status_t palette_push_color(palette_t *palette, const string_view_t *hexstring);
FORCE_INLINE void palette_set_color_fam(palette_t *palette, uint8_t index, uint8_t lighten, uint8_t normal, uint8_t darken);
FORCE_INLINE void palette_unset_color_fam(palette_t *palette, uint8_t index);

FORCE_INLINE status_t palette_push_color(palette_t *palette, const string_view_t *hexstring) {
    color_t *color = palette->colors.entries + palette->colors.length++;
    return hexstring_to_color(hexstring, color);
}

FORCE_INLINE void palette_set_color_fam(palette_t *palette, uint8_t index, uint8_t lighten, uint8_t normal, uint8_t darken) {
    palette->colorfams[index] = (colorfam_t) {
        .lighten = lighten,
        .normal = normal,
        .darken = darken,
        .flag = 1
    };
}

FORCE_INLINE void palette_unset_color_fam(palette_t *palette, uint8_t index) {
    palette->colorfams[index] = (colorfam_t) {
        .lighten = 0,
        .normal = 0,
        .darken = 0,
        .flag = 0
    };
}

#endif /* MATERIAL_PALETTE_H */ 