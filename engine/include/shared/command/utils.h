#ifndef COMMAND_UTILS_H
#define COMMAND_UTILS_H

#include "command/command.h"

#define COMMAND_BASE_PATH       "temp/base.png"
#define COMMAND_TEMP_PATH       "temp/temp.png"

extern arena_t *composition_arena;

void command_randomize_param(matroot_t *matroot, uint8_t palette_id, model_type_t type, uint8_t *model_id, uint8_t *moption_id, uint8_t **variant_ids, uint8_t **option_ids);
void command_anims_skipped_init(matroot_t *matroot, bool_t **anims_skipped, uint8_t model_id, uint8_t *variant_ids, uint8_t *option_ids);

#endif /* COMMAND_UTILS_H */ 