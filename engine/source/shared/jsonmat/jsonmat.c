#include "jsonmat/utils.h"

string_map_t *jsonmatk_map = NULL;

void jsonmat_ctx_set_jsonmatk_map(string_map_t *map) {
    jsonmatk_map = map;
}

void jsonmat_get_path_dir(const char *cstr, string_view_t *dir) {
    char *last_post = strrchr(cstr, '/');
    
    dir->cstr = (char *) cstr;
    dir->length = last_post - cstr;
}

status_t jsonmat_open_json(const char *path, json_t *json) {
    file_t file;
    string_t buffer;

    if (file_open(&file, path, FILE_OMODE_RD) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    string_init_capacity(&buffer, file.length);

    if (file_read_string(&file, &buffer) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if (json_parse(json, &buffer, TRUE) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    string_deinit(&buffer);
    return EXIT_SUCCESS;
}

status_t jsonmat_decode_material(matroot_t *matroot, arena_t *perm_arena, arena_t *temp_arena, json_idmap_t *json_idmap, string_map_t *color_map, color_stats_t *color_stats, const string_t *buffer, string_t *error_message) {
    json_ctx_set_arena(temp_arena);
    image_ctx_set_arena(temp_arena);
    material_ctx_set_arena(perm_arena);

    arena_init(perm_arena, ARENA_SMALL_CAP);
    arena_init(temp_arena, ARENA_BIG_CAP);

    string_view_t root_path;
    json_object_t part_variant_mapper;
    json_t matroot_json;

    jsonmat_get_path_dir(MATERIAL_MATROOT_PATH, &root_path);
    json_object_init(&part_variant_mapper);

    json_idmap_init(json_idmap);
    color_map_init(color_map);
    color_stats_init(color_stats);
    crc32_init_reflect();

    json_ctx_set_idmap(json_idmap);
    material_ctx_set_color_map(color_map);
    material_ctx_set_color_stats(color_stats);

    if (json_parse(&matroot_json, (string_t *) buffer, TRUE) != EXIT_SUCCESS) {
        string_from_cstr(error_message, "Failed to open and parse matroot as json.");
        return EXIT_FAILURE;
    }

    json_array_t *palette_files;
    json_array_t *model_files;
    json_array_t *part_variant_files;

    if (jsonmat_decode_matroot(matroot, &matroot_json, &palette_files, &model_files, &part_variant_files, error_message) != EXIT_SUCCESS) {
        string_from_cstr(error_message, "Some attribute at matroot invalid.");
        return EXIT_FAILURE;
    }

    if (jsonmat_decode_palettes(matroot, &root_path, palette_files, error_message) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if (jsonmat_decode_part_variants(matroot, &root_path, part_variant_files, &part_variant_mapper, error_message) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if (jsonmat_decode_models(matroot, &root_path, model_files, &part_variant_mapper, error_message) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    arena_reset(temp_arena);
    return EXIT_SUCCESS;
}