#ifndef COMPOSITION_COMPOSITION_H
#define COMPOSITION_COMPOSITION_H

#include "image/image.h"
#include "material/material.h"

void composition_ctx_set_arena(arena_t *arena);

void composition_composite(matroot_t *matroot, bool_t *anims_skipped, uint8_t palette_id, uint8_t model_id, uint8_t moption_id, uint8_t *variant_ids, uint8_t *option_ids, pixels_rgba_t *out);
void composition_composite_single_at_model(palette_t *palette, model_t *model, part_variant_t *part_variant, uint8_t model_option_id, uint8_t frame_id, pixels_rgba_t *out);
void composition_composite_single_at_variant(palette_t *palette, model_t *model, part_variant_t *part_variant, uint8_t variant_option_id, uint8_t frame_id, pixels_rgba_t *out);

#endif /* COMPOSITION_COMPOSITION_H */ 