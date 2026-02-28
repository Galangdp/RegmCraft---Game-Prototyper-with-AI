#ifndef MATERIAL_MODEL_H
#define MATERIAL_MODEL_H

#include "material/color.h"
#include "material/part_variant.h"

typedef enum model_type {
    MODEL_TYPE_CHARACTER,
    MODEL_TYPE_MONSTER,
    MODEL_TYPE_VEHICLE
} model_type_t;

typedef struct model_part_variant {
    part_variant_t *data;
    bool_t *exclude_anim_map;
} model_part_variant_t;

typedef struct model_part {
    string_view_t name;
    struct {
        model_part_variant_t *entries;
        uint8_t length;
    } variants;
    bool_t is_optional;
} model_part_t;

typedef struct anim {
    string_view_t name;
    uint16_t *frame_delays;
    int16_t require;
    uint8_t total_frames;
    bool_t is_looping;
} anim_t;

typedef struct model {
    string_view_t name;
    colorchn_t *colorchn;
    struct {
        model_part_t *entries;
        uint8_t length;
    } parts;
    struct {
        anim_t *entries;
        uint8_t length;
    } anims;
    model_type_t type;
    uint8_t base;
} model_t;

void model_part_variant_init(model_part_variant_t *partvd, part_variant_t *data, uint8_t total_anims);
FORCE_INLINE void model_part_variant_set_exclude(model_part_variant_t *partvd, uint8_t anim_id);
FORCE_INLINE bool_t model_part_variant_is_exclude(model_part_variant_t *partvd, uint8_t anim_id);

void model_part_init(model_part_t *part, uint8_t total_variants, bool_t optional);
void model_part_set_name(model_part_t *part, const string_view_t *name);
FORCE_INLINE model_part_variant_t *model_part_alloc_variant(model_part_t *part);

void anim_init(anim_t *anim, uint8_t total_frames, bool_t loop);
void anim_set_name(anim_t *anim, const string_view_t *name);
FORCE_INLINE void anim_set_require(anim_t *anim, int16_t require);
FORCE_INLINE void anim_set_delay(anim_t *anim, uint8_t index, uint16_t delay);

void model_init(model_t *model, model_type_t type, uint8_t total_anims, uint8_t total_parts, uint8_t base);
void model_set_name(model_t *model, const string_view_t *name);
colorchn_t *model_alloc_colorchn(model_t *model);
FORCE_INLINE model_part_t *model_alloc_part(model_t *model);
FORCE_INLINE anim_t *model_alloc_anim(model_t *model);

FORCE_INLINE void model_part_variant_set_exclude(model_part_variant_t *partvd, uint8_t anim_id) {
    partvd->exclude_anim_map[anim_id] = TRUE;
}

FORCE_INLINE bool_t model_part_variant_is_exclude(model_part_variant_t *partvd, uint8_t anim_id) {
    return partvd->exclude_anim_map[anim_id];
}

FORCE_INLINE model_part_variant_t *model_part_alloc_variant(model_part_t *part) {
    return part->variants.entries + part->variants.length++;
}

FORCE_INLINE void anim_set_require(anim_t *anim, int16_t require) {
    anim->require = require;
}

FORCE_INLINE void anim_set_delay(anim_t *anim, uint8_t index, uint16_t delay) {
    anim->frame_delays[index] = delay;
}

FORCE_INLINE model_part_t *model_alloc_part(model_t *model) {
    return model->parts.entries + model->parts.length++;
}

FORCE_INLINE anim_t *model_alloc_anim(model_t *model) {
    return model->anims.entries + model->anims.length++;
}

#endif /* MATERIAL_MODEL_H */ 