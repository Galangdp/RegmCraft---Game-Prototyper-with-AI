#include "command/utils.h"

status_t command_init(matroot_t *matroot, string_t *error_message, json_t *out) {
    json_init_object(out);

    json_object_t *out_jobj = out->jobj;

    json_array_t *palettes_jarr = json_object_cset_empty_array(out_jobj, "palettes");
    json_array_t *characters_jarr = json_object_cset_empty_array(out_jobj, "characters");
    json_array_t *monsters_jarr = json_object_cset_empty_array(out_jobj, "monsters");
    json_array_t *vehicles_jarr = json_object_cset_empty_array(out_jobj, "vehicles");

    size_t frame_id = 0;
    pixels_array_t pixels_array;

    pixels_array_init(&pixels_array);

    for (size_t i = 0; i < matroot->palettes.length; i++) {
        palette_t *palette = matroot->palettes.entries + i;
        json_object_t *palette_jobj = json_array_push_empty_object(palettes_jarr);

        json_object_cset_string(palette_jobj, "name", &palette->name);
        json_object_cset_int64(palette_jobj, "total", palette->colors.length);
    }
    
    palette_t *current_palette = matroot->palettes.entries;
    
    for (size_t i = 0; i < matroot->models.length; i++) {
        model_t *model = matroot->models.entries + i;
        json_object_t *model_jobj = NULL;

        switch (model->type) {
            case MODEL_TYPE_CHARACTER:
                model_jobj = json_array_push_empty_object(characters_jarr);
                break;

                case MODEL_TYPE_MONSTER:
                model_jobj = json_array_push_empty_object(monsters_jarr);
                break;

            case MODEL_TYPE_VEHICLE:
                model_jobj = json_array_push_empty_object(vehicles_jarr);
                break;
            
            break;
        }

        json_object_cset_string(model_jobj, "name", &model->name);
        json_object_cset_int64(model_jobj, "id", i);
        
        if (model->colorchn != NULL) {
            json_object_t *variant_jobj = json_object_cset_empty_object(model_jobj, "variant");
            json_array_t *variant_data_jarr = json_object_cset_empty_array(variant_jobj, "options");
            colorchn_t *colorchn = model->colorchn;
            
            json_object_cset_string(variant_jobj, "name", &colorchn->name);

            part_variant_t *base_variant = model->parts.entries[model->base].variants.entries[0].data;
            
            for (size_t j = 0; j < colorchn->length; j++) {
                json_object_t *option_jobj = json_array_push_empty_object(variant_data_jarr);
                colorchn_opt_t *colorchn_opt = colorchn->entries + j;
                
                json_object_cset_string(option_jobj, "name", &colorchn_opt->name);
                json_object_cset_int64(option_jobj, "frameId", frame_id++);

                pixels_rgba_t *pixels = pixels_array_alloc(&pixels_array);
                composition_composite_single_at_model(current_palette, model, base_variant, j, 0, pixels);
            }
        }

        json_array_t *parts_jarr = json_object_cset_empty_array(model_jobj, "parts");
        
        for (size_t j = 0; j < model->parts.length; j++) {
            json_object_t *part_jobj = json_array_push_empty_object(parts_jarr);
            json_array_t *part_variant_jarr = json_object_cset_empty_array(part_jobj, "variants");
            model_part_t *part = model->parts.entries + j;
            
            json_object_cset_string(part_jobj, "name", &part->name);

            if (part->is_optional) {
                json_object_t *variant_jobj = json_array_push_empty_object(part_variant_jarr);

                json_object_cset_int64(variant_jobj, "id", -1);
                json_object_cset_cstr(variant_jobj, "name", "None");
            }
            
            for (size_t k = 0; k < part->variants.length; k++) {
                json_object_t *variant_jobj = json_array_push_empty_object(part_variant_jarr);
                model_part_variant_t *variant = part->variants.entries + k;
                part_variant_t *variant_data = variant->data;
                colorchn_t *colorchn = variant_data->colorchn;

                json_object_cset_int64(variant_jobj, "id", k);
                json_object_cset_string(variant_jobj, "name", &variant_data->name);
                json_object_cset_int64(variant_jobj, "frameId", frame_id++);
                
                pixels_rgba_t *pixels = pixels_array_alloc(&pixels_array);
                composition_composite_single_at_variant(current_palette, model, variant_data, 0, 0, pixels);
                
                if (colorchn != NULL) {
                    json_array_t *colorchn_opt_jarr = json_object_cset_empty_array(variant_jobj, "options");
                    
                    for (size_t l = 0; l < colorchn->length; l++) {
                        json_object_t *colorchn_opt_jobj = json_array_push_empty_object(colorchn_opt_jarr);
                        colorchn_opt_t *colorchn_opt = colorchn->entries +l;
                        bool_t is_appear = TRUE;
                        
                        for (size_t m = 0; m < colorchn_opt->length; m++) {
                            color_sem_t colorsem = colorchn_opt->entries[m];
                            
                            if (!current_palette->colorfams[colorsem].flag) {
                                is_appear = FALSE;
                                break;
                            }
                        }

                        if (!is_appear) {
                            continue;
                        }

                        json_object_cset_string(colorchn_opt_jobj, "name", &colorchn_opt->name);
                        json_object_cset_int64(colorchn_opt_jobj, "id", l);
                        json_object_cset_int64(colorchn_opt_jobj, "frameId", frame_id++);

                        pixels_rgba_t *pixels = pixels_array_alloc(&pixels_array);
                        composition_composite_single_at_variant(current_palette, model, variant_data, l, 0, pixels);
                    }
                }
            }
        }
    }

    json_object_cset_cstr(out_jobj, "base", COMMAND_BASE_PATH);
    json_object_cset_cstr(out_jobj, "status", "ok");
    json_object_cset_int64(out_jobj, "paletteId", 0);

    pixels_rgba_t base_pixels;

    base_pixels.width = frame_id * MATERIAL_FRAME_WIDTH;
    base_pixels.height = MATERIAL_FRAME_HEIGHT;
    base_pixels.values = memory_alloc(base_pixels.width * base_pixels.height * sizeof(pixel_rgba_t));

    size_t width_pad = base_pixels.width - MATERIAL_FRAME_WIDTH;

    for (size_t i = 0; i < frame_id; i++) {
        pixels_rgba_t *pixels = pixels_array.values + i;
        pixel_rgba_t *dest = base_pixels.values + i * MATERIAL_FRAME_WIDTH;
        pixel_rgba_t *src = pixels->values;
        
        for (uint8_t y = 0; y < MATERIAL_FRAME_HEIGHT; y++) {
            for (uint8_t x = 0; x < MATERIAL_FRAME_WIDTH; x++) {
                dest->red = src->red;
                dest->green = src->green;
                dest->blue = src->blue;
                dest->alpha = src->alpha;
                
                dest++;
                src++;
            }

            dest += width_pad;
        }
    }

    file_t base_file;
    size_t bound = 0;
    
    if (image_encode_bound(&base_pixels, &bound) != EXIT_SUCCESS) {
        string_from_cstr(error_message, "Fail to calculate pixel bound.");
        pixels_array_deinit(&pixels_array);
        json_deinit(out);
        return EXIT_FAILURE;
    }

    uint8_t *buffer = arena_alloc(composition_arena, bound);

    if (image_encode(&base_pixels, buffer, &bound)) {
        string_from_cstr(error_message, "Fail to encode pixel to png.");
        pixels_array_deinit(&pixels_array);
        json_deinit(out);
        return EXIT_FAILURE;
    }
    
    if (file_open(&base_file, COMMAND_BASE_PATH, FILE_OMODE_WR)) {
        string_from_cstr(error_message, "Fail to open file for writing png.");
        pixels_array_deinit(&pixels_array);
        json_deinit(out);
        return EXIT_FAILURE;
    }
    
    if (file_write_byte(&base_file, buffer, bound)) {
        string_from_cstr(error_message, "Fail to write png.");
        pixels_array_deinit(&pixels_array);
        json_deinit(out);
        file_close(&base_file);
        return EXIT_FAILURE;
    }
    
    pixels_array_deinit(&pixels_array);
    file_close(&base_file);

    return EXIT_SUCCESS;
}