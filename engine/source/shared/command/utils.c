#include "command/utils.h"

void command_randomize_param(matroot_t *matroot, uint8_t palette_id, model_type_t type, uint8_t *model_id, uint8_t *moption_id, uint8_t **variant_ids, uint8_t **option_ids) {
    palette_t *palette = matroot->palettes.entries + palette_id;
    uint8_t buffer_random[255];
    uint8_t buffer_random_length = 0;
    
    {
        for (size_t i = 0; i < matroot->models.length; i++) {
            model_t *model = matroot->models.entries + i;

            if (model->type == type) {
                buffer_random[buffer_random_length++] = i;
            }
        }
        
        *model_id = buffer_random[rand() % buffer_random_length];
        buffer_random_length = 0;
    }
    
    {
        model_t *model = matroot->models.entries + *model_id;
        *moption_id = MATERIAL_INV_ANIM_ID;

        if (model->colorchn != NULL) {
            colorchn_t *colorchn = model->colorchn;
            
            for (uint8_t option_id = 0; option_id < colorchn->length; option_id++) {
                colorchn_opt_t *colorchn_opt = colorchn->entries + option_id;
                bool_t is_appear = TRUE;
                
                for (size_t m = 0; m < colorchn_opt->length; m++) {
                    color_sem_t colorsem = colorchn_opt->entries[m];
                    
                    if (!palette->colorfams[colorsem].flag) {
                        is_appear = FALSE;
                        break;
                    }
                }

                if (!is_appear) {
                    continue;
                }
                
                buffer_random[buffer_random_length++] = option_id;
            }
            
            *moption_id = buffer_random[rand() % buffer_random_length];
            buffer_random_length = 0;
        }
        
        *variant_ids = arena_alloc(composition_arena, model->parts.length);
        *option_ids = arena_alloc(composition_arena, model->parts.length);
        
        
        for (uint8_t part_id = 0; part_id < model->parts.length; part_id++) {
            model_part_t *part = model->parts.entries + part_id;
            uint8_t total_variants = part->variants.length;
            uint8_t variant_id = rand() % (part->is_optional ? total_variants + 1 : total_variants);

            (*variant_ids)[part_id] = variant_id;

            if (variant_id == total_variants) {
                (*variant_ids)[part_id] = MATERIAL_INV_ANIM_ID;
                (*option_ids)[part_id] = 0;
                continue;
            }
            
            part_variant_t *variant = part->variants.entries[variant_id].data;
            colorchn_t *colorchn = variant->colorchn;
            
            if (colorchn == NULL) {
                (*option_ids)[part_id] = 0;
                continue;
            }

            for (uint8_t option_id = 0; option_id < colorchn->length; option_id++) {
                colorchn_opt_t *colorchn_opt = colorchn->entries + option_id;
                bool_t is_appear = TRUE;

                for (size_t m = 0; m < colorchn_opt->length; m++) {
                    color_sem_t colorsem = colorchn_opt->entries[m];
                    
                    if (!palette->colorfams[colorsem].flag) {
                        is_appear = FALSE;
                        break;
                    }
                }

                if (is_appear) {
                    buffer_random[buffer_random_length++] = option_id;
                }
            }

            (*option_ids)[part_id] = buffer_random[rand() % buffer_random_length];
            buffer_random_length = 0;
        }

        // for (uint8_t i = 0; i < model->parts.length; i++) {
        //     printf("%u: %d %d\n", i, (*variant_ids)[i], (*option_ids)[i]);
        // }
    }
}

void command_anims_skipped_init(matroot_t *matroot, bool_t **anims_skipped, uint8_t model_id, uint8_t *variant_ids, uint8_t *option_ids) {
    model_t *model = matroot->models.entries + model_id;
    *anims_skipped = arena_alloc(composition_arena, model->anims.length);

    memset(*anims_skipped, 0, model->anims.length);
    
    for (uint8_t part_id = 0; part_id < model->parts.length; part_id++) {
        uint8_t variant_id = variant_ids[part_id];
        model_part_t *part = model->parts.entries + part_id;
        
        if (variant_id == MATERIAL_INV_ANIM_ID) {
            for (uint8_t anim_id = 0; anim_id < model->anims.length; anim_id++) {
                anim_t *anim = model->anims.entries + anim_id;

                if (anim->require == part_id) {
                    (*anims_skipped)[anim_id] = TRUE;
                }
            }
            continue;
        }

        model_part_variant_t *part_variant = part->variants.entries + variant_id;
    
        for (uint8_t anim_id = 0; anim_id < model->anims.length; anim_id++) {
            if (part_variant->exclude_anim_map[anim_id]) {
                (*anims_skipped)[anim_id] = TRUE;
            }
        }
    }
}