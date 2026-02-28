#include "jsonmat/utils.h"

FORCE_INLINE status_t jsonmat_decode_palette(matroot_t *matroot, const json_object_t *palette_jobj, string_t *error_message) {
    json_value_t *name = json_object_sget((json_object_t *) palette_jobj, JSONMAT_STR_NAME, sizeof(JSONMAT_STR_NAME));
    json_value_t *colors = json_object_sget((json_object_t *) palette_jobj, JSONMAT_STR_COLORS, sizeof(JSONMAT_STR_COLORS));
    json_value_t *semantics = json_object_sget((json_object_t *) palette_jobj, JSONMAT_STR_SEMANTICS, sizeof(JSONMAT_STR_SEMANTICS));
    palette_t *palette = matroot_alloc_palette(matroot);
    
    uint8_t total_colors = 1;

    if (name == NULL || colors == NULL || semantics == NULL) {
        string_from_cstr(error_message, "Some palette properties are not found.");
        return EXIT_FAILURE;
    }

    if (name->type != JSON_VTYPE_STRING || colors->type != JSON_VTYPE_ARRAY || semantics->type != JSON_VTYPE_OBJECT) {
        string_from_cstr(error_message, "Some palette properties's type mismatch.");
        return EXIT_FAILURE;
    }
    
    {
        json_array_t *colors_jarr = colors->jarr;

        total_colors = colors_jarr->length;

        if (total_colors == 0) {
            string_from_cstr(error_message, "Total color at palette must be >= 1 and <= 256.");
            return EXIT_FAILURE;
        }

        palette_init(palette, total_colors);

        for (size_t i = colors_jarr->start, max_i = colors_jarr->start + total_colors; i < max_i; i++) {
            json_value_t *color = colors_jarr->values + i;

            if (color->type != JSON_VTYPE_STRING) {
                string_from_cstr(error_message, "Error palette color is not string.");
                return EXIT_FAILURE;
            }

            if (palette_push_color(palette, color->string) != EXIT_SUCCESS) {
                string_from_cstr(error_message, "Error palette color is not valid color string.");
                return EXIT_FAILURE;
            }
        }
    }

    {
        json_object_t *semantics_jobj = semantics->jobj;

        for (size_t i = 0; i < COLOR_SEM_NONE; i++) {
            color_stat_t *color_stat = *material_color_stats + i;
            json_value_t *value = json_object_get(semantics_jobj, &color_stat->string);

            if (value == NULL) {
                if (color_stat->is_required) {
                    string_from_cstr(error_message, "Missing required color semantic.");
                    return EXIT_FAILURE;
                }
                palette_unset_color_fam(palette, i);
                continue;
            }

            if (!color_stat->is_family) {
                if (value->type != JSON_VTYPE_INT64) {
                    string_from_cstr(error_message, "Invalid color semantic must be int.");
                    return EXIT_FAILURE;
                }

                if (value->int64 >= total_colors) {
                    string_from_cstr(error_message, "Invalid color id is out of bound.");
                    return EXIT_FAILURE;
                }
                
                palette_set_color_fam(palette, i, value->int64, 0, 0);
                continue;
            }
            
            if (value->type != JSON_VTYPE_ARRAY) {
                string_from_cstr(error_message, "Invalid color semantic must be array.");
                return EXIT_FAILURE;
            }

            json_array_t *value_jarr = value->jarr;

            if (value_jarr->length != 3) {
                string_from_cstr(error_message, "Invalid color semantic array must be length of 3.");
                return EXIT_FAILURE;
            }

            json_value_t *lighten = json_array_atuns(value_jarr, 0);
            json_value_t *normal = json_array_atuns(value_jarr, 1);
            json_value_t *darken = json_array_atuns(value_jarr, 2);

            if (lighten->type != JSON_VTYPE_INT64 || normal->type != JSON_VTYPE_INT64 || darken->type != JSON_VTYPE_INT64) {
                string_from_cstr(error_message, "Invalid color semantic must be array of int.");
                return EXIT_FAILURE;
            }

            if (lighten->int64 >= total_colors || normal->int64 >= total_colors || darken->int64 >= total_colors) {
                string_from_cstr(error_message, "Invalid color id is out of bound.");
                return EXIT_FAILURE;
            }

            palette_set_color_fam(palette, i, lighten->int64, normal->int64, darken->int64);
        }
    }

    {
        string_view_t *name_string = name->string;

        if (name_string->length == 0 || name_string->length > 32) {
            string_from_cstr(error_message, "Palette name length must be >= 1 and <= 32.");
            return EXIT_FAILURE;
        }

        palette_set_name(palette, name_string);
    }

    return EXIT_SUCCESS;
}

status_t jsonmat_decode_palettes(matroot_t *matroot, const string_view_t *dir_path, const json_array_t *palette_files, string_t *error_message) {
    string_t path;

    string_init_capacity(&path, dir_path->length + 32);
    
    if (dir_path->length == 0) {
        string_push_char(&path, '.');
    } else {
        string_push_cstrl(&path, dir_path->cstr, dir_path->length);
    }

    string_push_char(&path, '/');
    string_push_cstrl(&path, JSONMAT_STR_PALETTES, sizeof(JSONMAT_STR_PALETTES));
    string_push_char(&path, '/');
    
    size_t last_length = path.length;

    for (size_t i = 0; i < palette_files->length; i++) {
        json_value_t *raw_pathv = json_array_atuns((json_array_t *) palette_files, i);

        if (raw_pathv->type != JSON_VTYPE_STRING) {
            string_from_cstr(error_message, "Invalid palette list must string value.");
            return EXIT_FAILURE;
        }

        json_t palette_json;

        string_push_cstrl(&path, raw_pathv->string->cstr, raw_pathv->string->length);
        string_terminate(&path);
        
        if (jsonmat_open_json(path.cstr, &palette_json) != EXIT_SUCCESS) {
            string_from_cstr(error_message, "Invalid open and parse palette as json.");
            return EXIT_FAILURE;
        }

        if (palette_json.is_array) {
            string_from_cstr(error_message, "Invalid palette format.");
            return EXIT_FAILURE;
        }

        if (jsonmat_decode_palette(matroot, palette_json.jobj, error_message) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        path.length = last_length;
    }

    string_deinit(&path);

    return EXIT_SUCCESS;
}