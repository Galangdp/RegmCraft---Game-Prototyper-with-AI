#include "composition/composition.h"

arena_t *composition_arena = NULL;

void composition_ctx_set_arena(arena_t *arena) {
    composition_arena = arena;
}

void composition_composite(matroot_t *matroot, bool_t *anims_skipped, uint8_t palette_id, uint8_t model_id, uint8_t moption_id, uint8_t *variant_ids, uint8_t *option_ids, pixels_rgba_t *out) {
    palette_t *palette = matroot->palettes.entries + palette_id;
    model_t *model = matroot->models.entries + model_id;
    size_t total_frames = 0;

    for (uint8_t anim_id = 0; anim_id < model->anims.length; anim_id++) {
        anim_t *anim = model->anims.entries + anim_id;

        if (!anims_skipped[anim_id]) {
            total_frames += anim->total_frames;
        }
    }

    out->width = total_frames * MATERIAL_FRAME_WIDTH;
    out->height = MATERIAL_FRAME_HEIGHT;

    size_t buffer_length = out->width * out->height * sizeof(pixel_rgba_t);

    out->values = arena_alloc(composition_arena, buffer_length);
    memset(out->values, 0, buffer_length);

    uint8_t *frame_ids = arena_alloc(composition_arena, model->parts.length);
    uint8_t frame_id_count = 0;

    memset(frame_ids, 0, model->parts.length);

    for (uint8_t anim_id = 0; anim_id < model->anims.length; anim_id++) {
        anim_t *anim = model->anims.entries + anim_id;

        if (anims_skipped[anim_id]) {
            for (uint8_t part_id = 0; part_id < model->parts.length; part_id++) {
                uint8_t variant_id = variant_ids[part_id];

                if (variant_id == MATERIAL_INV_ANIM_ID) {
                    continue;
                }

                model_part_t *part = model->parts.entries + part_id;
                model_part_variant_t *model_part_variant = part->variants.entries + variant_id;

                if (!model_part_variant->exclude_anim_map[anim_id]) {
                    frame_ids[part_id] += anim->total_frames;
                }
            }

            continue;
        }

        for (uint8_t i = 0; i < anim->total_frames; i++) {
            for (uint8_t part_id = model->parts.length; part_id > 0; part_id--) {
                uint8_t variant_id = variant_ids[part_id - 1];
                
                if (variant_id == MATERIAL_INV_ANIM_ID) {
                    continue;
                }

                uint8_t option_id = option_ids[part_id - 1];

                if (option_id == MATERIAL_INV_ANIM_ID) {
                    option_id = 0;
                }

                model_part_t *part = model->parts.entries + part_id - 1;
                model_part_variant_t *model_part_variant = part->variants.entries + variant_id;
                part_variant_t *part_variant = model_part_variant->data;
    
                uint8_t frame_id = frame_ids[part_id - 1]++;
                
                pixel_rgba_t *dest = out->values + frame_id_count * MATERIAL_FRAME_WIDTH;
                part_variant_frame_t *frame = part_variant->frames.entries + frame_id;
                color_t *src = frame->data;
    
                uint8_t min_x = frame->x;
                uint8_t min_y = frame->y;
                uint8_t max_x = frame->x + frame->width;
                uint8_t max_y = frame->y + frame->height;
    
                for (uint8_t y = 0; y < MATERIAL_FRAME_HEIGHT; y++) {
                    for (uint8_t x = 0; x < MATERIAL_FRAME_WIDTH; x++) {
                        if (x < min_x || y < min_y || x >= max_x || y >= max_y) {
                            dest++;
                            continue;
                        }
    
                        uint8_t mode = (src->red & 0xF0) >> 4;
                        uint8_t channel = src->red & 0x0F;
                        uint8_t value = src->green;
                        uint8_t light = src->blue;
    
                        color_sem_t colorsem = value;
                        uint8_t color_id = 0;
    
                        switch (mode) {
                            case 0:
                                if (channel == 1) {
                                    dest->red = 0;
                                    dest->green = 0;
                                    dest->blue = 0;
                                    dest->alpha = 0;
                                }
                                dest++;
                                src++;
                                continue;
                            
                            case 1:
                                break;
                                
                            case 2:
                                if (dest->alpha != 0) {
                                    dest++;
                                    src++;
                                    continue;
                                }
                                break;
                                
                            case 3:
                                if (dest->alpha == 255) {
                                    dest++;
                                    src++;
                                    continue;
                                }
                                break;
                        }
    
                        if (channel == 1) {
                            colorsem = part_variant->colorchn->entries[option_id].entries[value];
                        } else if (channel == 2) {
                            colorsem = model->colorchn->entries[moption_id].entries[value];
                        }
                        
                        switch (light) {
                            case 0:
                                color_id = palette->colorfams[colorsem].lighten;
                                break;
                            case 1:
                                color_id = palette->colorfams[colorsem].normal;
                                break;
                            case 2:
                                color_id = palette->colorfams[colorsem].darken;
                                break;
                        }
    
                        color_t *color = palette->colors.entries + color_id;
    
                        dest->red = color->red;
                        dest->green = color->green;
                        dest->blue = color->blue;
                        dest->alpha = colorsem == COLOR_SEM_OUTLINE ? 1 : 255;
    
                        dest++;
                        src++;
                    }
    
                    dest += out->width - MATERIAL_FRAME_WIDTH;
                }
            }
            
            frame_id_count++;
        }
    }

    for (size_t i = 0; i < buffer_length; i++) {
        pixel_rgba_t *pixel = out->values + i;

        if (pixel->alpha == 1) {
            pixel->alpha = 255;
        }
    }
}

void composition_composite_single_at_model(palette_t *palette, model_t *model, part_variant_t *part_variant, uint8_t model_option_id, uint8_t frame_id, pixels_rgba_t *out) {
    part_variant_frame_t *frame = part_variant->frames.entries + frame_id;
    uint8_t min_x = frame->x;
    uint8_t min_y = frame->y;
    uint8_t max_x = frame->x + frame->width;
    uint8_t max_y = frame->y + frame->height;
    size_t buffer_length = MATERIAL_FRAME_WIDTH * MATERIAL_FRAME_HEIGHT * sizeof(pixel_rgba_t);

    out->values = arena_alloc(composition_arena, buffer_length);
    out->width = MATERIAL_FRAME_WIDTH;
    out->height = MATERIAL_FRAME_HEIGHT;

    memset(out->values, 0, buffer_length);

    pixel_rgba_t *dest = out->values;
    color_t *src = frame->data;

    for (uint8_t y = 0; y < MATERIAL_FRAME_HEIGHT; y++) {
        for (uint8_t x = 0; x < MATERIAL_FRAME_WIDTH; x++) {
            if (x < min_x || y < min_y || x >= max_x || y >= max_y) {
                dest++;
                continue;
            }

            uint8_t mode = (src->red & 0xF0) >> 4;
            uint8_t channel = src->red & 0x0F;
            uint8_t value = src->green;
            uint8_t light = src->blue;

            color_sem_t colorsem = value;
            uint8_t color_id = 0;

            switch (mode) {
                case 0:
                    if (channel == 1) {
                        dest->red = 0;
                        dest->green = 0;
                        dest->blue = 0;
                        dest->alpha = 0;
                    }
                    break;
                
                case 1:
                case 2:
                case 3: {
                    if (channel == 1) {
                        colorsem = part_variant->colorchn->entries[0].entries[value];
                    } else if (channel == 2) {
                        colorsem = model->colorchn->entries[model_option_id].entries[value];
                    }
                    
                    switch (light) {
                        case 0:
                            color_id = palette->colorfams[colorsem].lighten;
                            break;
                        case 1:
                            color_id = palette->colorfams[colorsem].normal;
                            break;
                        case 2:
                            color_id = palette->colorfams[colorsem].darken;
                            break;
                    }

                    color_t *color = palette->colors.entries + color_id;

                    dest->red = color->red;
                    dest->green = color->green;
                    dest->blue = color->blue;
                    dest->alpha = 255;
                    break;
                }
            }

            dest++;
            src++;
        }
    }
}

void composition_composite_single_at_variant(palette_t *palette, model_t *model, part_variant_t *part_variant, uint8_t variant_option_id, uint8_t frame_id, pixels_rgba_t *out) {
    part_variant_frame_t *frame = part_variant->frames.entries + frame_id;
    uint8_t min_x = frame->x;
    uint8_t min_y = frame->y;
    uint8_t max_x = frame->x + frame->width;
    uint8_t max_y = frame->y + frame->height;
    size_t buffer_length = MATERIAL_FRAME_WIDTH * MATERIAL_FRAME_HEIGHT * sizeof(pixel_rgba_t);

    out->values = arena_alloc(composition_arena, buffer_length);
    out->width = MATERIAL_FRAME_WIDTH;
    out->height = MATERIAL_FRAME_HEIGHT;

    memset(out->values, 0, buffer_length);

    pixel_rgba_t *dest = out->values;
    color_t *src = frame->data;

    for (uint8_t y = 0; y < MATERIAL_FRAME_HEIGHT; y++) {
        for (uint8_t x = 0; x < MATERIAL_FRAME_WIDTH; x++) {
            if (x < min_x || y < min_y || x >= max_x || y >= max_y) {
                dest++;
                continue;
            }

            uint8_t mode = (src->red & 0xF0) >> 4;
            uint8_t channel = src->red & 0x0F;
            uint8_t value = src->green;
            uint8_t light = src->blue;

            color_sem_t colorsem = value;
            uint8_t color_id = 0;

            switch (mode) {
                case 0:
                    if (channel == 1) {
                        dest->red = 0;
                        dest->green = 0;
                        dest->blue = 0;
                        dest->alpha = 0;
                    }
                    break;
                
                case 1:
                case 2:
                case 3: {
                    if (channel == 1) {
                        colorsem = part_variant->colorchn->entries[variant_option_id].entries[value];
                    } else if (channel == 2) {
                        colorsem = model->colorchn->entries[0].entries[value];
                    }
                    
                    switch (light) {
                        case 0:
                            color_id = palette->colorfams[colorsem].lighten;
                            break;
                        case 1:
                            color_id = palette->colorfams[colorsem].normal;
                            break;
                        case 2:
                            color_id = palette->colorfams[colorsem].darken;
                            break;
                    }

                    color_t *color = palette->colors.entries + color_id;

                    dest->red = color->red;
                    dest->green = color->green;
                    dest->blue = color->blue;
                    dest->alpha = 255;
                    break;
                }
            }

            dest++;
            src++;
        }
    }
}