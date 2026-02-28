#ifndef MATERIAL_MATERIAL_H
#define MATERIAL_MATERIAL_H

#include "material/color_stat.h"
#include "material/matroot.h"
#include "material/map.h"
#include "utils/arena.h"

#define MATERIAL_FRAME_WIDTH    32
#define MATERIAL_FRAME_HEIGHT   32
#define MATERIAL_INV_ANIM_ID    255

void material_ctx_set_arena(arena_t *arena);
void material_ctx_set_color_map(string_map_t *color_map);
void material_ctx_set_color_stats(color_stats_t *color_stats);

#endif /* MATERIAL_MATERIAL_H */ 