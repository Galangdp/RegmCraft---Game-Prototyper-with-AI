#ifndef MATERIAL_MATROOT_H
#define MATERIAL_MATROOT_H

#include "material/palette.h"
#include "material/model.h"

typedef struct matroot {
    struct {
        palette_t *entries;
        size_t length;
    } palettes;
    struct {
        part_variant_t *entries;
        size_t length;
    } part_variants;
    struct {
        model_t *entries;
        size_t length;
    } models;
} matroot_t;

void matroot_init(matroot_t *matroot, size_t total_palettes, size_t total_part_variants, size_t total_models);
FORCE_INLINE palette_t *matroot_alloc_palette(matroot_t *matroot);
FORCE_INLINE part_variant_t *matroot_alloc_part_variant(matroot_t *matroot);
FORCE_INLINE model_t *matroot_alloc_model(matroot_t *matroot);

FORCE_INLINE palette_t *matroot_alloc_palette(matroot_t *matroot) {
    return matroot->palettes.entries + matroot->palettes.length++;
}

FORCE_INLINE part_variant_t *matroot_alloc_part_variant(matroot_t *matroot) {
    return matroot->part_variants.entries + matroot->part_variants.length++;
}

FORCE_INLINE model_t *matroot_alloc_model(matroot_t *matroot) {
    return matroot->models.entries + matroot->models.length++;
}

#endif /* MATERIAL_MATROOT_H */ 