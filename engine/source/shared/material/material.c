#include "material/material.h"

arena_t *material_arena = NULL;
string_map_t *material_color_map = NULL;
color_stats_t *material_color_stats = NULL;

void material_ctx_set_arena(arena_t *arena) {
    material_arena = arena;
}

void material_ctx_set_color_map(string_map_t *color_map) {
    material_color_map = color_map;
}

void material_ctx_set_color_stats(color_stats_t *color_stats) {
    material_color_stats = color_stats;
}

void matroot_init(matroot_t *matroot, size_t total_palettes, size_t total_part_variants, size_t total_models) {
    matroot->palettes.entries = arena_alloc(material_arena, total_palettes * sizeof(palette_t));
    matroot->palettes.length = 0;
    
    matroot->part_variants.entries = arena_alloc(material_arena, total_part_variants * sizeof(part_variant_t));
    matroot->part_variants.length = 0;

    matroot->models.entries = arena_alloc(material_arena, total_models * sizeof(model_t));
    matroot->models.length = 0;
}

void palette_init(palette_t *palette, size_t total_colors) {
    palette->colors.entries = arena_alloc(material_arena, total_colors * sizeof(color_t));
    palette->colors.length = 0;
}

void palette_set_name(palette_t *palette, const string_view_t *name) {
    palette->name.cstr = arena_alloc(material_arena, name->length);
    palette->name.length = name->length;

    memcpy(palette->name.cstr, name->cstr, name->length);
}

void colorchn_opt_init(colorchn_opt_t *colorchn_opt, uint8_t total_colorfam) {
    colorchn_opt->entries = arena_alloc(material_arena, total_colorfam * sizeof(color_sem_t));
    colorchn_opt->length = 0;
}

void colorchn_opt_set_name(colorchn_opt_t *colorchn_opt, const string_view_t *name) {
    colorchn_opt->name.cstr = arena_alloc(material_arena, name->length);
    colorchn_opt->name.length = name->length;

    memcpy(colorchn_opt->name.cstr, name->cstr, name->length);
}

void colorchn_init(colorchn_t *colorchn, uint8_t total_opts) {
    colorchn->entries = arena_alloc(material_arena, total_opts * sizeof(colorchn_opt_t));
    colorchn->length = 0;
}

void colorchn_set_name(colorchn_t *colorchn, const string_view_t *name) {
    colorchn->name.cstr = arena_alloc(material_arena, name->length);
    colorchn->name.length = name->length;

    memcpy(colorchn->name.cstr, name->cstr, name->length);
}

void part_variant_frame_set_new(part_variant_frame_t *frame) {
    frame->data = arena_alloc(material_arena, frame->width * frame->height * sizeof(color_t));
}

void part_variant_init(part_variant_t *part_variant, uint8_t total_frames) {
    part_variant->colorchn = NULL;

    part_variant->frames.entries = arena_alloc(material_arena, (size_t) total_frames * sizeof(part_variant_frame_t));
    part_variant->frames.length = total_frames;
}

colorchn_t *part_variant_alloc_colorchn(part_variant_t *part_variant) {
    colorchn_t *colorchn = arena_alloc(material_arena, sizeof(colorchn_t));
    part_variant->colorchn = colorchn;
    return colorchn;
}

void part_variant_set_name(part_variant_t *part_variant, const string_view_t *name) {
    part_variant->name.cstr = arena_alloc(material_arena, name->length);
    part_variant->name.length = name->length;

    memcpy(part_variant->name.cstr, name->cstr, name->length);
}

void model_part_variant_init(model_part_variant_t *partvd, part_variant_t *data, uint8_t total_anims) {
    partvd->data = data;
    partvd->exclude_anim_map = arena_alloc(material_arena, total_anims);

    memset(partvd->exclude_anim_map, 0, total_anims);
}

void model_part_init(model_part_t *part, uint8_t total_variants, bool_t optional) {
    part->is_optional = optional;

    part->variants.entries = arena_alloc(material_arena, total_variants * sizeof(model_part_variant_t));
    part->variants.length = 0;
}

void model_part_set_name(model_part_t *part, const string_view_t *name) {
    part->name.cstr = arena_alloc(material_arena, name->length);
    part->name.length = name->length;

    memcpy(part->name.cstr, name->cstr, name->length);
}

void anim_init(anim_t *anim, uint8_t total_frames, bool_t loop) {
    anim->is_looping = loop;
    anim->total_frames = total_frames;
    anim->frame_delays = arena_alloc(material_arena, total_frames * sizeof(uint16_t));
    anim->require = MATERIAL_INV_ANIM_ID;
}

void anim_set_name(anim_t *anim, const string_view_t *name) {
    anim->name.cstr = arena_alloc(material_arena, name->length);
    anim->name.length = name->length;

    memcpy(anim->name.cstr, name->cstr, name->length);
}

void model_init(model_t *model, model_type_t type, uint8_t total_anims, uint8_t total_parts, uint8_t base) {
    model->type = type;
    model->colorchn = NULL;
    model->base = base;

    model->parts.entries = arena_alloc(material_arena, total_parts * sizeof(model_part_t));
    model->parts.length = 0;
    
    model->anims.entries = arena_alloc(material_arena, total_anims * sizeof(anim_t));
    model->anims.length = 0;
}

void model_set_name(model_t *model, const string_view_t *name) {
    model->name.cstr = arena_alloc(material_arena, name->length);
    model->name.length = name->length;

    memcpy(model->name.cstr, name->cstr, name->length);
}

colorchn_t *model_alloc_colorchn(model_t *model) {
    colorchn_t *colorchn = arena_alloc(material_arena, sizeof(colorchn_t));
    model->colorchn = colorchn;
    return colorchn;
}