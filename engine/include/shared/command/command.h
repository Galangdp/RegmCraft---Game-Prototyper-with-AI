#ifndef COMMAND_COMMAND_H
#define COMMAND_COMMAND_H

#include "composition/composition.h"
#include "image/pixels_array.h"
#include "json/json.h"
#include "material/material.h"

#define COMMAND_ID_LISTS \
    _DISPATCHER(COMMAND_ID_INIT, "init", (sizeof("init") - 1)) \
    _DISPATCHER(COMMAND_ID_CREATE, "create", (sizeof("create") - 1)) \
    _DISPATCHER(COMMAND_ID_EDIT, "edit", (sizeof("edit") - 1)) \
\

#define _DISPATCHER(NAME, _STR, _STRLEN) NAME,
typedef enum command_id {
    COMMAND_ID_LISTS
    COMMAND_ID_NONE
} command_id_t;
#undef _DISPATCHER

status_t command_process(matroot_t *matroot, string_t *error_message, const json_t *in, json_t *out);

status_t command_init(matroot_t *matroot, string_t *error_message, json_t *out);
status_t command_create(matroot_t *matroot, string_t *error_message, uint8_t palette_id, model_type_t type, json_t *out);
status_t command_edit(matroot_t *matroot, string_t *error_message, uint8_t palette_id, uint8_t model_id, uint8_t moption_id, uint8_t *variant_ids, uint8_t *option_ids, json_t *out);

#endif /* COMMAND_COMMAND_H */ 