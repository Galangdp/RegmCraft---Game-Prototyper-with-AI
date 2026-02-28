#include "jsonmat/utils.h"

status_t jsonmat_decode_matroot(matroot_t *matroot, const json_t *json_matroot, json_array_t **palette_files, json_array_t **model_files, json_array_t **part_variant_files, string_t *error_message) {
    if (json_matroot->is_array) {
        string_from_cstr(error_message, "Invalid matroot format.");
        return EXIT_FAILURE;
    }

    json_value_t *palettes = json_object_sget(json_matroot->jobj, JSONMAT_STR_PALETTES, sizeof(JSONMAT_STR_PALETTES));
    json_value_t *models = json_object_sget(json_matroot->jobj, JSONMAT_STR_MODELS, sizeof(JSONMAT_STR_MODELS));
    json_value_t *part_variants = json_object_sget(json_matroot->jobj, JSONMAT_STR_PART_VARIANTS, sizeof(JSONMAT_STR_PART_VARIANTS));

    if (palettes == NULL || models == NULL || part_variants == NULL) {
        string_from_cstr(error_message, "Some matroot properties are not found.");
        return EXIT_FAILURE;
    }

    if (palettes->type != JSON_VTYPE_ARRAY || models->type != JSON_VTYPE_ARRAY || part_variants->type != JSON_VTYPE_ARRAY) {
        string_from_cstr(error_message, "Some matroot properties are not array.");
        return EXIT_FAILURE;
    }

    matroot_init(matroot, palettes->jarr->length, part_variants->jarr->length, models->jarr->length);

    *palette_files = palettes->jarr;
    *model_files = models->jarr;
    *part_variant_files = part_variants->jarr;

    return EXIT_SUCCESS;
}