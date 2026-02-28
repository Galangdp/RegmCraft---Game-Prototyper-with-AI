#ifndef MATERIAL_MAP_H
#define MATERIAL_MAP_H

#include "material/color.h"
#include "utils/string_map.h"
#include "material/utils.h"

FORCE_INLINE void color_map_init(string_map_t *map);
FORCE_INLINE color_sem_t color_map_get_sem(string_map_t *map, const char *cstr, size_t length);
FORCE_INLINE color_semt_t color_map_get_semt(string_map_t *map, const char *cstr, size_t length);
FORCE_INLINE color_semp_t color_map_get_semp(string_map_t *map, const char *cstr, size_t length);

FORCE_INLINE void color_map_init(string_map_t *map) {
    size_t total_capacity = COLOR_SEM_NONE + COLOR_SEMT_NONE + COLOR_SEMP_NONE;
    size_t capacity = ceil_pow2_64((size_t) ((double) total_capacity + (double) total_capacity * 0.3));

    string_map_ctx_set_arena(material_arena);
    string_map_init(map, capacity);

    #define _DISPATCHER(NAME, STR, STRLEN, _SEMT, _SEMP, _IFAM, _IREQ) string_map_insert(map, STR, STRLEN, (uint16_t) NAME);
    COLOR_SEM_LISTS
    #undef _DISPATCHER

    #define _DISPATCHER(NAME, STR, STRLEN) string_map_insert(map, STR, STRLEN, (uint16_t) COLOR_SEM_NONE + NAME);
    COLOR_SEMT_LISTS
    #undef _DISPATCHER
    
    #define _DISPATCHER(NAME, STR, STRLEN) string_map_insert(map, STR, STRLEN, (uint16_t) COLOR_SEM_NONE + COLOR_SEMP_NONE + NAME);
    COLOR_SEMP_LISTS
    #undef _DISPATCHER
}

FORCE_INLINE color_sem_t color_map_get_sem(string_map_t *map, const char *cstr, size_t length) {
    int32_t value = string_map_get(map, cstr, length);
    return value == STRING_MAP_INVVAL || value >= COLOR_SEM_NONE ? COLOR_SEM_NONE : value;
}

FORCE_INLINE color_semt_t color_map_get_semt(string_map_t *map, const char *cstr, size_t length) {
    int32_t value = string_map_get(map, cstr, length);
    if (value == STRING_MAP_INVVAL) {
        return COLOR_SEMT_NONE;
    }
    value -= COLOR_SEM_NONE;
    return value < 0 || value >= COLOR_SEMT_NONE ? COLOR_SEMT_NONE : value;
}

FORCE_INLINE color_semp_t color_map_get_semp(string_map_t *map, const char *cstr, size_t length) {
    int32_t value = string_map_get(map, cstr, length);
    if (value == STRING_MAP_INVVAL) {
        return COLOR_SEMP_NONE;
    }
    value -= COLOR_SEM_NONE + COLOR_SEMT_NONE;
    return value < 0 || value >= COLOR_SEMP_NONE ? COLOR_SEMP_NONE : value;
}

#endif /* MATERIAL_MAP_H */ 