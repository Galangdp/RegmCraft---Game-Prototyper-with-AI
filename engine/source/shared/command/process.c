#include "command/utils.h"

typedef struct command_string_chent {
    const char *cstr;
    size_t length;
} command_string_chent_t;

#define _DISPATCHER(NAME, STR, STRLEN) (command_string_chent_t) {.cstr = STR, .length = STRLEN },
command_string_chent_t COMMAND_STRING_CHEMAP[] = {
    COMMAND_ID_LISTS
};
#undef _DISPATCHER

FORCE_INLINE command_id_t command_resolve_cmd_string(const string_view_t *cmd) {
    for (size_t i = 0, length = sizeof(COMMAND_STRING_CHEMAP) / sizeof(command_string_chent_t); i < length; i++) {
        command_string_chent_t *checker = COMMAND_STRING_CHEMAP + i;

        if (cmd->length == checker->length && memcmp(cmd->cstr, checker->cstr, cmd->length) == 0) {
            return i;
        }
    }

    return COMMAND_ID_NONE;
}

status_t command_process(matroot_t *matroot, string_t *error_message, const json_t *in, json_t *out) {
    if (in->is_array) {
        string_from_cstr(error_message, "Invalid command format.");
        return EXIT_FAILURE;
    }

    json_object_t *in_jobj = in->jobj;
    json_value_t *cmd = json_object_cget(in_jobj, "cmd");

    if (cmd == NULL || cmd->type != JSON_VTYPE_STRING) {
        string_from_cstr(error_message, "Cmd prop missing or invalid.");
        return EXIT_FAILURE;
    }

    command_id_t command_id = command_resolve_cmd_string(cmd->string);
    
    switch (command_id) {
        case COMMAND_ID_INIT:
            return command_init(matroot, error_message, out);

        case COMMAND_ID_CREATE: {
            json_value_t *palette_id = json_object_cget(in_jobj, "paletteId");
            json_value_t *type = json_object_cget(in_jobj, "type");

            if (palette_id == NULL || type == NULL || palette_id->type != JSON_VTYPE_INT64 || type->type != JSON_VTYPE_INT64) {
                string_from_cstr(error_message, "Some prop e.g. paletteId or type missing or invalid.");
                return EXIT_FAILURE;
            }

            if (palette_id->int64 < 0 || palette_id->int64 >= matroot->palettes.length) {
                string_from_cstr(error_message, "Prop paletteId is invalid.");
                return EXIT_FAILURE;
            }

            if (type->int64 < MODEL_TYPE_CHARACTER || type->int64 > MODEL_TYPE_VEHICLE) {
                string_from_cstr(error_message, "P type is invalid, must be either 0, 1 or 2.");
                return EXIT_FAILURE;
            }

            return command_create(matroot, error_message, palette_id->int64, type->int64, out);
        }

        case COMMAND_ID_EDIT: {
            json_value_t *palette_id = json_object_cget(in_jobj, "paletteId");
            json_value_t *model_id = json_object_cget(in_jobj, "modelId");
            json_value_t *moption_id = json_object_cget(in_jobj, "moptionId");
            json_value_t *parts = json_object_cget(in_jobj, "parts");
            
            if (palette_id == NULL || model_id == NULL || parts == NULL || palette_id->type != JSON_VTYPE_INT64 || model_id->type != JSON_VTYPE_INT64 || parts->type != JSON_VTYPE_ARRAY) {
                string_from_cstr(error_message, "Some prop e.g. paletteId, modelId or parts missing or invalid.");
                return EXIT_FAILURE;
            }
            
            if (palette_id->int64 < 0 || palette_id->int64 >= matroot->palettes.length) {
                string_from_cstr(error_message, "Prop paletteId is invalid.");
                return EXIT_FAILURE;
            }
            
            if (model_id->int64 < 0 || model_id->int64 >= matroot->models.length) {
                string_from_cstr(error_message, "Prop modelId is invalid.");
                return EXIT_FAILURE;
            }

            
            model_t *model = matroot->models.entries + model_id->int64;
            uint8_t total_parts = model->parts.length;
            
            if (moption_id != NULL) {
                if (model->colorchn == NULL) {
                    string_from_cstr(error_message, "This model doesn't have channel but channel value provided.");
                    return EXIT_FAILURE;
                }
                
                if (moption_id->type != JSON_VTYPE_INT64 || moption_id->int64 < 0 || moption_id->int64 >= model->colorchn->length) {
                    string_from_cstr(error_message, "Prop moptionId is invalid.");
                    return EXIT_FAILURE;
                }
            }
            
            if (parts->jarr->length != total_parts) {
                string_from_cstr(error_message, "Parts length with model parts is mismatch.");
                return EXIT_FAILURE;
            }
            
            uint8_t *variant_ids = arena_alloc(composition_arena, total_parts);
            uint8_t *option_ids = arena_alloc(composition_arena, total_parts);
            
            for (size_t part_id = 0; part_id < total_parts; part_id++) {
                json_value_t *part_raw = json_array_atuns(parts->jarr, part_id);
                model_part_t *part = model->parts.entries + part_id;

                if (part_raw->type != JSON_VTYPE_OBJECT) {
                    string_from_cstr(error_message, "Parts must be array of object.");
                    return EXIT_FAILURE;
                }
                
                json_value_t *variant_id_raw = json_object_cget(part_raw->jobj, "variantId");
                json_value_t *option_id_raw = json_object_cget(part_raw->jobj, "optionId");

                if (variant_id_raw == NULL || variant_id_raw->type != JSON_VTYPE_INT64) {
                    string_from_cstr(error_message, "Invalid prop variantId.");
                    return EXIT_FAILURE;
                }
                
                if (variant_id_raw->int64 == -1) {
                    variant_ids[part_id] = MATERIAL_INV_ANIM_ID;
                    option_ids[part_id] = MATERIAL_INV_ANIM_ID;
                    continue;
                }
                
                if (variant_id_raw->int64 < 0 || variant_id_raw->int64 >= part->variants.length) {
                    string_from_cstr(error_message, "Prop variantId is out of bound.");
                    return EXIT_FAILURE;
                }
                
                uint8_t variant_id = variant_id_raw->int64;
                uint8_t option_id = option_id_raw == NULL ? MATERIAL_INV_ANIM_ID : option_id_raw->int64;
                part_variant_t *part_variant = part->variants.entries[variant_id].data;
                
                variant_ids[part_id] = variant_id;
                
                if (part_variant->colorchn == NULL) {
                    option_ids[part_id] = MATERIAL_INV_ANIM_ID;
                    continue;
                }

                if (option_id_raw == NULL || option_id_raw->type != JSON_VTYPE_INT64) {
                    string_from_cstr(error_message, "Invalid prop optionId.");
                    return EXIT_FAILURE;
                }

                if (option_id < 0 || option_id >= part_variant->colorchn->length) {
                    string_from_cstr(error_message, "Prop optionId is out of bound.");
                    return EXIT_FAILURE;
                }

                option_ids[part_id] = option_id;
            }

            return command_edit(matroot, error_message, palette_id->int64, model_id->int64, moption_id->int64, variant_ids, option_ids, out);
        }

        case COMMAND_ID_NONE:
            break;
    }
    
    string_from_cstr(error_message, "Unknown cmd.");
    return EXIT_FAILURE;
}