#ifndef JSONMAT_JSONMAT_H
#define JSONMAT_JSONMAT_H

#include "utils/file.h"
#include "json/json.h"
#include "material/material.h"
#include "image/image.h"

void jsonmat_get_path_dir(const char *cstr, string_view_t *dir);
void jsonmat_ctx_set_jsonmatk_map(string_map_t *map);

status_t jsonmat_decode_material(matroot_t *matroot, arena_t *perm_arena, arena_t *temp_arena, json_idmap_t *json_idmap, string_map_t *color_map, color_stats_t *color_stats, const string_t *buffer, string_t *error_message);
status_t jsonmat_decode_matroot(matroot_t *matroot, const json_t *json_matroot, json_array_t **palette_files, json_array_t **model_files, json_array_t **part_variant_files, string_t *error_message);
status_t jsonmat_decode_palettes(matroot_t *matroot, const string_view_t *dir_path, const json_array_t *palette_files, string_t *error_message);
status_t jsonmat_decode_part_variants(matroot_t *matroot, const string_view_t *dir_path, const json_array_t *part_variants_files, json_object_t *part_variant_mapper, string_t *error_message);
status_t jsonmat_decode_models(matroot_t *matroot, const string_view_t *dir_path, const json_array_t *model_files, json_object_t *part_variant_mapper, string_t *error_message);

#endif /* JSONMAT_JSONMAT_H */ 