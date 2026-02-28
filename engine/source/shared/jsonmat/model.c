#include "jsonmat/utils.h"

FORCE_INLINE status_t jsonmat_decode_model(model_t *model, part_variant_t *part_variant_basep, const json_object_t *model_jobj, json_object_t *part_variant_mapper, string_t *error_message) {
    json_value_t *name = json_object_sget((json_object_t *) model_jobj, JSONMAT_STR_NAME, sizeof(JSONMAT_STR_NAME));
    json_value_t *type = json_object_sget((json_object_t *) model_jobj, JSONMAT_STR_TYPE, sizeof(JSONMAT_STR_TYPE));
    json_value_t *colorchn = json_object_sget((json_object_t *) model_jobj, JSONMAT_STR_CHANNELS, sizeof(JSONMAT_STR_CHANNELS));
    json_value_t *anims = json_object_sget((json_object_t *) model_jobj, JSONMAT_STR_ANIMS, sizeof(JSONMAT_STR_ANIMS));
    json_value_t *parts = json_object_sget((json_object_t *) model_jobj, JSONMAT_STR_PARTS, sizeof(JSONMAT_STR_PARTS));
    json_value_t *base = json_object_sget((json_object_t *) model_jobj, JSONMAT_STR_BASE, sizeof(JSONMAT_STR_BASE));

    uint8_t total_anims = 0;

    if (name == NULL || type == NULL || anims == NULL || parts == NULL || base == NULL) {
        string_from_cstr(error_message, "Some model properties are not found.");
        return EXIT_FAILURE;
    }

    if (name->type != JSON_VTYPE_STRING || type->type != JSON_VTYPE_INT64 || anims->type != JSON_VTYPE_ARRAY || parts->type != JSON_VTYPE_ARRAY || base->type != JSON_VTYPE_INT64) {
        string_from_cstr(error_message, "Some model properties type mismatch.");
        return EXIT_FAILURE;
    }

    if (type->int64 != 0 && type->int64 != 1 && type->int64 != 2) {
        string_from_cstr(error_message, "Model type must be 0, 1, or 2.");
        return EXIT_FAILURE;
    }

    if (name->string->length == 0 || name->string->length > 32) {
        string_from_cstr(error_message, "Model name length must be >= 1 and <= 32.");
        return EXIT_FAILURE;
    }

    model_init(model, type->int64, anims->jarr->length, parts->jarr->length, base->int64);
    model_set_name(model, name->string);

    if (colorchn != NULL) {
        if (colorchn->type != JSON_VTYPE_OBJECT) {
            string_from_cstr(error_message, "Some palette properties type mismatch.");
            return EXIT_FAILURE;
        }

        colorchn_t *colorchn_dest = model_alloc_colorchn(model);
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

            for (size_t i = 0; i < value_jarr->length; i++) {
                json_value_t *colorsem_raw = json_array_atuns(value_jarr, i);
                
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

    {
        json_array_t *anims_jarr = anims->jarr;
        total_anims = anims_jarr->length;

        for (uint8_t i = 0; i < total_anims; i++) {
            json_value_t *anims = json_array_atuns(anims_jarr, i);
            anim_t *anim = model_alloc_anim(model);

            if (anims->type != JSON_VTYPE_OBJECT) {
                string_from_cstr(error_message, "Expect object as anims array value.");
                return EXIT_FAILURE;
            }

            json_value_t *name = json_object_sget(anims->jobj, JSONMAT_STR_NAME, sizeof(JSONMAT_STR_NAME));
            json_value_t *loop = json_object_sget(anims->jobj, JSONMAT_STR_LOOP, sizeof(JSONMAT_STR_LOOP));
            json_value_t *total = json_object_sget(anims->jobj, JSONMAT_STR_TOTAL, sizeof(JSONMAT_STR_TOTAL));
            json_value_t *delays = json_object_sget(anims->jobj, JSONMAT_STR_DELAYS, sizeof(JSONMAT_STR_DELAYS));
            json_value_t *require = json_object_sget(anims->jobj, JSONMAT_STR_REQUIRE, sizeof(JSONMAT_STR_REQUIRE));

            if (name == NULL || loop == NULL || total == NULL || delays == NULL) {
                string_from_cstr(error_message, "Some anim prop are missing.");
                return EXIT_FAILURE;
            }
            
            if (name->type != JSON_VTYPE_STRING || loop->type != JSON_VTYPE_BOOL || total->type != JSON_VTYPE_INT64 || delays->type != JSON_VTYPE_ARRAY || (require != NULL && require->type != JSON_VTYPE_INT64)) {
                string_from_cstr(error_message, "Some anim prop type mismatch.");
                return EXIT_FAILURE;
            }

            if (name->string->length == 0 || name->string->length > 32) {
                string_from_cstr(error_message, "Anim name length must be >= 1 and <= 32.");
                return EXIT_FAILURE;
            }

            if (delays->jarr->length != total->int64) {
                string_from_cstr(error_message, "Delays array length doesn't match total frames.");
                return EXIT_FAILURE;
            }
            
            anim_init(anim, total->int64, loop->bool);
            anim_set_name(anim, name->string);

            if (require != NULL) {
                anim_set_require(anim, require->int64);
            }

            for (uint8_t j = 0; j < delays->jarr->length; j++) {
                json_value_t *delay = json_array_atuns(delays->jarr, j);

                if (delay->type != JSON_VTYPE_INT64) {
                    string_from_cstr(error_message, "Delay must be an int.");
                    return EXIT_FAILURE;
                }

                anim_set_delay(anim, j, delay->int64);
            }
        }
    }

    {
        json_array_t *parts_jarr = parts->jarr;

        for (uint8_t i = 0; i < parts_jarr->length; i++) {
            json_value_t *parts = json_array_atuns(parts_jarr, i);
            model_part_t *part = model_alloc_part(model);

            if (parts->type != JSON_VTYPE_OBJECT) {
                string_from_cstr(error_message, "Expect object as parts array value.");
                return EXIT_FAILURE;
            }

            json_value_t *name = json_object_sget(parts->jobj, JSONMAT_STR_NAME, sizeof(JSONMAT_STR_NAME));
            json_value_t *optional = json_object_sget(parts->jobj, JSONMAT_STR_OPTIONAL, sizeof(JSONMAT_STR_OPTIONAL));
            json_value_t *variants = json_object_sget(parts->jobj, JSONMAT_STR_VARIANTS, sizeof(JSONMAT_STR_FAMILIES));

            if (name == NULL || optional == NULL || variants == NULL) {
                string_from_cstr(error_message, "Some anim prop are missing.");
                return EXIT_FAILURE;
            }

            if (name->type != JSON_VTYPE_STRING || optional->type != JSON_VTYPE_BOOL || variants->type != JSON_VTYPE_ARRAY) {
                string_from_cstr(error_message, "Some anim prop are mismatch.");
                return EXIT_FAILURE;
            }

            if (name->string->length == 0 || name->string->length > 32) {
                string_from_cstr(error_message, "Prop name length must be >= 1 and <= 32.");
                return EXIT_FAILURE;
            }

            model_part_init(part, variants->jarr->length, optional->bool);
            model_part_set_name(part, name->string);

            for (uint8_t j = 0; j < variants->jarr->length; j++) {
                json_value_t *partvd_raw = json_array_atuns(variants->jarr, j);
                model_part_variant_t *partvd = model_part_alloc_variant(part);

                if (partvd_raw->type != JSON_VTYPE_OBJECT) {
                    string_from_cstr(error_message, "Variant must be an object.");
                    return EXIT_FAILURE;
                }

                json_value_t *name = json_object_sget(partvd_raw->jobj, JSONMAT_STR_NAME, sizeof(JSONMAT_STR_NAME));
                json_value_t *exclude = json_object_sget(partvd_raw->jobj, JSONMAT_STR_EXCLUDE, sizeof(JSONMAT_STR_EXCLUDE));

                if (name == NULL || exclude == NULL) {
                    string_from_cstr(error_message, "Some part variant stat prop are missing.");
                    return EXIT_FAILURE;
                }

                if (name->type != JSON_VTYPE_STRING || exclude->type != JSON_VTYPE_ARRAY) {
                    string_from_cstr(error_message, "Some part variant stat prop are mismatch.");
                    return EXIT_FAILURE;
                }

                if (name->string->length == 0 || name->string->length > 32) {
                    string_from_cstr(error_message, "Prop name length must be >= 1 and <= 32.");
                    return EXIT_FAILURE;
                }

                json_value_t *data_raw = json_object_sget(part_variant_mapper, name->string->cstr, name->string->length);

                if (data_raw == NULL) {
                    printf("Part \"%.*s\" is not listed in matroot.\n", (int) name->string->length, name->string->cstr);
                    return EXIT_FAILURE;
                }

                model_part_variant_init(partvd, part_variant_basep + data_raw->int64, total_anims);

                for (size_t k = 0; k < exclude->jarr->length; k++) {
                    json_value_t *anim_id = json_array_atuns(exclude->jarr, k);

                    if (anim_id->type != JSON_VTYPE_INT64) {
                        string_from_cstr(error_message, "Exclude array value must be int.");
                        return EXIT_FAILURE;
                    }

                    if (anim_id->int64 < 0 || anim_id->int64 >= total_anims) {
                        string_from_cstr(error_message, "Exclude array value out of bound.");
                        return EXIT_FAILURE;
                    }

                    model_part_variant_set_exclude(partvd, anim_id->int64);
                }
            }
        }
    }
    return EXIT_SUCCESS;
}

status_t jsonmat_decode_models(matroot_t *matroot, const string_view_t *dir_path, const json_array_t *model_files, json_object_t *part_variant_mapper, string_t *error_message) {
    string_t path;

    string_init_capacity(&path, dir_path->length + 32);
    
    if (dir_path->length == 0) {
        string_push_char(&path, '.');
    } else {
        string_push_cstrl(&path, dir_path->cstr, dir_path->length);
    }

    string_push_char(&path, '/');
    string_push_cstrl(&path, JSONMAT_STR_MODELS, sizeof(JSONMAT_STR_MODELS));
    string_push_char(&path, '/');
    
    size_t last_length = path.length;

    for (size_t i = 0; i < model_files->length; i++) {
        json_value_t *raw_pathv = json_array_atuns((json_array_t *) model_files, i);
        model_t *model = matroot_alloc_model(matroot);

        if (raw_pathv->type != JSON_VTYPE_STRING) {
            string_from_cstr(error_message, "Invalid model list must string value.");
            return EXIT_FAILURE;
        }

        json_t model_json;

        string_push_cstrl(&path, raw_pathv->string->cstr, raw_pathv->string->length);
        string_terminate(&path);

        if (jsonmat_open_json(path.cstr, &model_json) != EXIT_SUCCESS || model_json.is_array) {
            string_from_cstr(error_message, "Invalid open and read model as json.");
            return EXIT_FAILURE;
        }

        if (jsonmat_decode_model(model, matroot->part_variants.entries, model_json.jobj, part_variant_mapper, error_message) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        path.length = last_length;
    }

    return EXIT_SUCCESS;
}