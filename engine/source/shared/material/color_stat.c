#include "material/color_stat.h"
#include "material/utils.h"

void color_stats_init(color_stats_t *stat) {
    *stat = arena_alloc(material_arena, COLOR_SEM_NONE * sizeof(color_stat_t));

    size_t index = 0;

    #define _DISPATCHER(NAME, STR, STRLEN, SEMT, SEMP, IFAM, IREQ) \
        (*stat)[index++] = (color_stat_t) { \
            .string = { \
                .cstr = (char *) STR, \
                .length = STRLEN, \
                .hash = fnv1amix((const uint8_t *) STR, STRLEN) \
            }, \
            .semt = SEMT, \
            .semp = SEMP, \
            .is_family = IFAM, \
            .is_required = IREQ \
        };\
    \

    COLOR_SEM_LISTS
    #undef _DISPATCHER
}