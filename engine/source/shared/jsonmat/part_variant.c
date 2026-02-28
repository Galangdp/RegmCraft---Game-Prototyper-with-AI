#include "jsonmat/utils.h"

FORCE_INLINE status_t jsonmat_open_part_variant_image(const string_t *image_path, pixels_rgba_t *pixels) {
    file_t file;

    if (file_open(&file, image_path->cstr, FILE_OMODE_RD) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    
    uint8_t *buffer = memory_alloc(file.length);
    
    if (file_read_byte(&file, buffer) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    
    if (image_decode(buffer, file.length, pixels) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    memory_free(buffer);
    return EXIT_SUCCESS;
}

FORCE_INLINE status_t jsonmat_decode_part_variant_image(part_variant_t *part_variant, colorrp_t *colorrp, const pixels_rgba_t *pixels, string_t *error_message) {
    uint8_t rows = pixels->width / MATERIAL_FRAME_WIDTH;
    uint8_t cols = pixels->height / MATERIAL_FRAME_HEIGHT;
    uint8_t total_frames = part_variant->frames.length;

    pixels_rgba_t *pixels_list = memory_alloc(total_frames * sizeof(pixels_rgba_t));
    pixel_bound_t *bound_list = memory_alloc(total_frames * sizeof(pixel_bound_t));
    size_t x_offset = 0;
    size_t y_offset = 0;

    for (uint8_t y = 0, i = 0; y < cols; y++) {
        x_offset = 0;
        for (uint8_t x = 0; x < rows; x++, i++) {
            pixels_rgba_sub(pixels_list + i, pixels, x_offset, y_offset, MATERIAL_FRAME_WIDTH, MATERIAL_FRAME_HEIGHT);
            x_offset += MATERIAL_FRAME_WIDTH;
        }
        y_offset += MATERIAL_FRAME_HEIGHT;
    }

    for (uint8_t i = 0; i < total_frames; i++) {
        pixels_rgba_t *pixels = pixels_list + i;
        pixel_bound_t *bound = bound_list + i;

        pixel_bound_init(bound, pixels);
        pixels_rgba_deinit(pixels);
    }
    
    for (uint8_t i = 0; i < total_frames; i++) {
        pixel_bound_t *bound = bound_list + i;

        for (uint8_t j = i; j > 0; j--) {
            pixel_bound_t *bound_checker = bound_list + j - 1;

            if (
                bound->hash == bound_checker->hash &&
                bound->width == bound_checker->width &&
                bound->height == bound_checker->height &&
                memcmp(bound->data, bound_checker->data, bound->width * bound->height) == 0
            ) {
                bound->is_own = FALSE;
                bound->owner_id = j - 1;
                break;
            }
        }
    }
    
    for (uint8_t i = 0; i < total_frames; i++) {
        pixel_bound_t *bound = bound_list + i;
        part_variant_frame_t *frame = part_variant->frames.entries + i;
        
        part_variant_frame_init(frame, bound->x, bound->y, bound->width, bound->height);

        if (!bound->is_own) {
            part_variant_frame_set_from(frame, part_variant->frames.entries + bound->owner_id);
            pixel_bound_deinit(bound);
            continue;
        }

        size_t dimension = bound->width * bound->height;

        part_variant_frame_set_new(frame);

        for (size_t j = 0; j < dimension; j++) {
            pixel_rgba_t *pixel = bound->data + j;
            color_t *color = frame->data + j;

            if (pixel->alpha == 0) {
                color->red = 0;
                color->green = 0;
                color->blue = 0;
                continue;
            }

            color_t *color_replacer = colorrp_search(colorrp, pixel->red, pixel->green, pixel->blue);

            if (color_replacer == NULL) {
                string_from_cstr(error_message, "Invalid color not found in data map.");
                return EXIT_FAILURE;
            }

            
            color->red = color_replacer->red;
            color->green = color_replacer->green;
            color->blue = color_replacer->blue;
        }

        pixel_bound_deinit(bound);
    }

    // for (uint8_t i = 0; i < total_frames; i++) {
    //     part_variant_frame_t *frame = part_variant->frames.entries + i;

    //     printf("%u %u %u %u %p\n", frame->x, frame->y, frame->width, frame->height, frame->data);

    //     for (size_t j = 0; j < frame->width * frame->height; j++) {
    //         if (j % frame->width == 0) {
    //             putchar('\n');
    //         }
    //         color_t *c = frame->data + j;
    //         printf("%02X%02X%02X ", c->r, c->g, c->b);
    //     }

    //     putchar('\n');
    //     putchar('\n');
    // }
    
    memory_free(pixels_list);
    memory_free(bound_list);

    return EXIT_SUCCESS;
}

FORCE_INLINE status_t jsonmat_decode_part_variant_json(part_variant_t *part_variant, const json_object_t *part_variant_jobj, colorrp_t *colorrp, string_t *image_path, string_t *error_message) {
    json_value_t *name = json_object_sget((json_object_t *) part_variant_jobj, JSONMAT_STR_NAME, sizeof(JSONMAT_STR_NAME));
    json_value_t *colorchn = json_object_sget((json_object_t *) part_variant_jobj, JSONMAT_STR_CHANNELS, sizeof(JSONMAT_STR_CHANNELS));
    json_value_t *data = json_object_sget((json_object_t *) part_variant_jobj, JSONMAT_STR_DATA, sizeof(JSONMAT_STR_DATA));

    if (name == NULL || data == NULL) {
        string_from_cstr(error_message, "Some part variant properties are not found.");
        return EXIT_FAILURE;
    }

    if (name->type != JSON_VTYPE_STRING || data->type != JSON_VTYPE_OBJECT) {
        string_from_cstr(error_message, "Some part variant properties's type mismatch.");
        return EXIT_FAILURE;
    }

    if (name->string->length == 0 || name->string->length > 32) {
        string_from_cstr(error_message, "Part variant name length must be >= 1 and <= 32.");
        return EXIT_FAILURE;
    }

    uint8_t total_frames = 0;

    {
        json_value_t *name = json_object_sget(data->jobj, JSONMAT_STR_NAME, sizeof(JSONMAT_STR_NAME));
        json_value_t *total = json_object_sget(data->jobj, JSONMAT_STR_TOTAL, sizeof(JSONMAT_STR_TOTAL));
        json_value_t *map = json_object_sget(data->jobj, JSONMAT_STR_MAP, sizeof(JSONMAT_STR_MAP));

        if (name == NULL || total == NULL || map == NULL) {
            string_from_cstr(error_message, "Missing some prop in part variant data.");
            return EXIT_FAILURE;
        }

        if (name->type != JSON_VTYPE_STRING || total->type != JSON_VTYPE_INT64 || map->type != JSON_VTYPE_ARRAY) {
            string_from_cstr(error_message, "Some prop type mismatch.");
            return EXIT_FAILURE;
        }

        total_frames = total->int64;
        
        json_array_t *map_jarr = map->jarr;
        
        string_push_cstrl(image_path, name->string->cstr, name->string->length);
        colorrp_init(colorrp, map_jarr->length);

        for (size_t i = 0; i < map_jarr->length; i++) {
            json_value_t *entry = json_array_atuns(map_jarr, i);

            if (entry->type != JSON_VTYPE_OBJECT) {
                string_from_cstr(error_message, "Data map must be array of object.");
                return EXIT_FAILURE;
            }

            json_value_t *from = json_object_sget(entry->jobj, JSONMAT_STR_FROM, sizeof(JSONMAT_STR_FROM));
            json_value_t *to = json_object_sget(entry->jobj, JSONMAT_STR_TO, sizeof(JSONMAT_STR_TO));

            if (from == NULL || to == NULL) {
                string_from_cstr(error_message, "Data map value missing prop.");
                return EXIT_FAILURE;
            }
            
            if (from->type != JSON_VTYPE_STRING || to->type != JSON_VTYPE_STRING) {
                string_from_cstr(error_message, "Data map prop type invalid.");
                return EXIT_FAILURE;
            }
            
            color_t fromc;
            color_t toc;

            if (hexstring_to_color(from->string, &fromc) != EXIT_SUCCESS || hexstring_to_color(to->string, &toc) != EXIT_SUCCESS) {
                string_from_cstr(error_message, "Data map prop hex string invalid.");
                return EXIT_FAILURE;
            }

            colorrp_push(colorrp, &fromc, &toc);
        }
    }

    part_variant_init(part_variant, total_frames);
    part_variant_set_name(part_variant, name->string);

    if (colorchn != NULL) {
        if (colorchn->type != JSON_VTYPE_OBJECT) {
            string_from_cstr(error_message, "Some palette properties's type mismatch.");
            return EXIT_FAILURE;
        }

        colorchn_t *colorchn_dest = part_variant_alloc_colorchn(part_variant);
        json_object_t *colorchn_obj = colorchn->jobj;
        json_value_t *name = json_object_sget((json_object_t *) colorchn_obj, JSONMAT_STR_NAME, sizeof(JSONMAT_STR_NAME));
        json_value_t *families = json_object_sget((json_object_t *) colorchn_obj, JSONMAT_STR_FAMILIES, sizeof(JSONMAT_STR_FAMILIES));

        if (name == NULL || families == NULL) {
            string_from_cstr(error_message, "Missing property of channel part.");
            return EXIT_FAILURE;
        }

        if (name->type != JSON_VTYPE_STRING || families->type != JSON_VTYPE_ARRAY) {
            string_from_cstr(error_message, "Error type mismatch for channel part.");
            return EXIT_FAILURE;
        }

        if (name->string->length == 0 || name->string->length > 32) {
            string_from_cstr(error_message, "Channel name length must be >= 1 and <= 32.");
            return EXIT_FAILURE;
        }

        json_array_t *families_jarr = families->jarr;

        colorchn_init(colorchn_dest, families_jarr->length);
        colorchn_set_name(colorchn_dest, name->string);

        for (size_t i = 0; i < families_jarr->length; i++) {
            json_value_t *family = json_array_atuns(families_jarr, i);
            colorchn_opt_t *colorchn_opt = colorchn_alloc_opt(colorchn_dest);

            if (family->type != JSON_VTYPE_OBJECT) {
                string_from_cstr(error_message, "Color family list must be array of object.");
                return EXIT_FAILURE;
            }

            json_value_t *name = json_object_sget((json_object_t *) family->jobj, JSONMAT_STR_NAME, sizeof(JSONMAT_STR_NAME));
            json_value_t *value = json_object_sget((json_object_t *) family->jobj, JSONMAT_STR_VALUE, sizeof(JSONMAT_STR_VALUE));

            if (name == NULL || value == NULL) {
                string_from_cstr(error_message, "Error missing some prop of color channel detail.");
                return EXIT_FAILURE;
            }

            if (name->type != JSON_VTYPE_STRING || value->type != JSON_VTYPE_ARRAY) {
                string_from_cstr(error_message, "Type mismatch some prop of color channel detail.");
                return EXIT_FAILURE;
            }

            if (name->string->length == 0 || name->string->length > 32) {
                string_from_cstr(error_message, "Channel family name length must be >= 1 and <= 32.");
                return EXIT_FAILURE;
            }

            json_array_t *value_jarr = value->jarr;

            colorchn_opt_init(colorchn_opt, value_jarr->length);
            colorchn_opt_set_name(colorchn_opt, name->string);

            for (size_t j = 0; j < value_jarr->length; j++) {
                json_value_t *colorsem_raw = json_array_atuns(value_jarr, j);
                
                if (colorsem_raw->type != JSON_VTYPE_STRING) {
                    string_from_cstr(error_message, "Color family must be semantic string.");
                    return EXIT_FAILURE;
                }

                color_sem_t colorsem = color_map_get_sem(material_color_map, colorsem_raw->string->cstr, colorsem_raw->string->length);

                if (colorsem == COLOR_SEM_NONE || colorsem <= COLOR_SEM_MOUTH) {
                    string_from_cstr(error_message, "Invalid color family semantic string.");
                    return EXIT_FAILURE;
                }

                colorchn_opt_push_colorsem(colorchn_opt, colorsem);
            }
        }
    }

    return EXIT_SUCCESS;
}

status_t jsonmat_decode_part_variants(matroot_t *matroot, const string_view_t *dir_path, const json_array_t *part_variants_files, json_object_t *part_variant_mapper, string_t *error_message) {
    string_t json_path;
    string_t image_path;

    string_init_capacity(&json_path, dir_path->length + 32);
    string_init_capacity(&image_path, dir_path->length + 32);
    
    if (dir_path->length == 0) {
        string_push_char(&json_path, '.');
        string_push_char(&image_path, '.');
    } else {
        string_push_cstrl(&json_path, dir_path->cstr, dir_path->length);
        string_push_cstrl(&image_path, dir_path->cstr, dir_path->length);
    }

    string_push_char(&json_path, '/');
    string_push_cstrl(&json_path, JSONMAT_STR_PARTS, sizeof(JSONMAT_STR_PARTS));
    string_push_char(&json_path, '/');
    string_push_char(&image_path, '/');
    string_push_cstrl(&image_path, JSONMAT_STR_IMAGES, sizeof(JSONMAT_STR_IMAGES));
    string_push_char(&image_path, '/');
    
    size_t last_json_path_length = json_path.length;
    size_t last_image_path_length = image_path.length;

    for (size_t i = 0; i < part_variants_files->length; i++) {
        json_value_t *raw_pathv = json_array_atuns((json_array_t *) part_variants_files, i);
        part_variant_t *part_variant = matroot_alloc_part_variant(matroot);

        if (raw_pathv->type != JSON_VTYPE_STRING) {
            string_from_cstr(error_message, "Invalid part variant list must string value.");
            return EXIT_FAILURE;
        }

        json_t part_variant_json;
        pixels_rgba_t pixels;
        colorrp_t colorrp;

        string_push_cstrl(&json_path, raw_pathv->string->cstr, raw_pathv->string->length);
        string_terminate(&json_path);
        
        if (jsonmat_open_json(json_path.cstr, &part_variant_json) != EXIT_SUCCESS || part_variant_json.is_array) {
            string_from_cstr(error_message, "Invalid open and parse part variant as json.");
            return EXIT_FAILURE;
        }
        
        if (jsonmat_decode_part_variant_json(part_variant, part_variant_json.jobj, &colorrp, &image_path, error_message) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        string_terminate(&image_path);

        if (jsonmat_open_part_variant_image(&image_path, &pixels) != EXIT_SUCCESS) {
            string_from_cstr(error_message, "Invalid open and read image of part variant.");
            return EXIT_FAILURE;
        }

        if (jsonmat_decode_part_variant_image(part_variant, &colorrp, &pixels, error_message) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        colorrp_deinit(&colorrp);

        json_path.length = last_json_path_length;
        image_path.length = last_image_path_length;

        json_object_vset_int64(part_variant_mapper, raw_pathv->string, i);
    }

    string_deinit(&json_path);
    string_deinit(&image_path);

    return EXIT_SUCCESS;
}