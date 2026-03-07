#include "command/utils.h"

const char COMMAND_INSTRUCTION_STRING[383] =
    "You are a semantic translator to structured json.\n"
    "Please translate the user prompt into structured json.\n\n"
    "The user prompt is mainly for creating some entity of pixel game assets separate with array based on how many user want to create.\n"
    "Entity type is person.\n\n"
    "Every entity type would have some part e.g parameter like weapon, leg etc.\n"
    "Different entity type have different parts too.";

FORCE_INLINE status_t command_write_instruction(matroot_t *matroot, string_t *error_message) {
    string_t instruction = {
        .cstr = (char *) COMMAND_INSTRUCTION_STRING,
        .length = sizeof(COMMAND_INSTRUCTION_STRING),
        .capacity = sizeof(COMMAND_INSTRUCTION_STRING)
    };

    file_t instruction_file;

    if (file_open(&instruction_file, COMMAND_INSTRUCTION_PATH, FILE_OMODE_WR) != EXIT_SUCCESS) {
        string_from_cstr(error_message, "Fail to open file for writing instruction string.");
        return EXIT_FAILURE;
    }
    
    if (file_write_string(&instruction_file, &instruction) != EXIT_SUCCESS) {
        string_from_cstr(error_message, "Fail to write instruction string.");
        file_close(&instruction_file);
        return EXIT_FAILURE;
    }
    
    file_close(&instruction_file);
    return EXIT_SUCCESS;
}

FORCE_INLINE status_t command_build_schema(matroot_t *matroot, string_t *error_message) {
    json_t json;

    json_init_object(&json);

    json_object_t *root_jobj = json.jobj;

    json_object_cset_cstr(root_jobj, "type", "array");
    json_object_cset_cstr(root_jobj, "description", "List of items user want to create. The prop are custom.");
    
    json_object_t *items_jobj = json_object_cset_empty_object(root_jobj, "items");
    json_array_t *oneof_jarr = json_object_cset_empty_array(items_jobj, "oneOf");

    for (size_t model_id = 0; model_id < matroot->models.length; model_id++) {
        json_object_t *oneof_root_jobj = json_array_push_empty_object(oneof_jarr);
        model_t *model = matroot->models.entries + model_id;

        json_object_cset_cstr(oneof_root_jobj, "type", "object");
        
        json_object_t *properties_jobj = json_object_cset_empty_object(oneof_root_jobj, "properties");
        json_array_t *required_jarr = json_object_cset_empty_array(oneof_root_jobj, "required");
        json_object_t *property_model_jobj = json_object_cset_empty_object(properties_jobj, "model");
        
        json_object_cset_cstr(property_model_jobj, "type", "string");
        json_object_cset_string(property_model_jobj, "const", &model->name);
        json_object_cset_string(property_model_jobj, "description", &model->description);
        json_array_push_cstr(required_jarr, "model");

        if (model->colorchn != NULL) {
            json_object_t *property_colorchn_jobj = json_object_vset_empty_object(properties_jobj, model->colorchn_alias);
            json_array_t *property_colorchn_enum_jarr = json_object_cset_empty_array(property_colorchn_jobj, "enum");
            colorchn_t *colorchn = model->colorchn;

            json_object_cset_cstr(property_colorchn_jobj, "type", "string");
            json_object_cset_string(property_colorchn_jobj, "description", model->colorchn_description);
            json_array_push_string(required_jarr, model->colorchn_alias);

            for (size_t chid = 0; chid < colorchn->length; chid++) {
                json_array_push_string(property_colorchn_enum_jarr, &colorchn->entries[chid].name);
            }
        }

        for (size_t partid = 0; partid < model->parts.length; partid++) {
            model_part_t *model_part = model->parts.entries + partid;
            json_object_t *property_part_jobj = json_object_vset_empty_object(properties_jobj, &model_part->alias);
            json_array_t *part_oneof_jarr = json_object_cset_empty_array(property_part_jobj, "oneOf");

            json_object_cset_string(property_part_jobj, "description", &model_part->description);

            if (!model_part->is_optional) {
                json_array_push_string(required_jarr, &model_part->alias);
            }

            for (size_t variantid = 0; variantid < model_part->variants.length; variantid++) {
                model_part_variant_t *variant = model_part->variants.entries + variantid;
                part_variant_t *part_variant = variant->data;
                json_object_t *part_variant_jobj = json_array_push_empty_object(part_oneof_jarr);
                json_object_t *part_variant_properties_jobj = json_object_cset_empty_object(part_variant_jobj, "properties");
                json_object_t *part_variant_name_jobj = json_object_cset_empty_object(part_variant_properties_jobj, "name");
                json_array_t *part_variant_required_jarr = json_object_cset_empty_array(part_variant_jobj, "required");

                json_array_push_cstr(part_variant_required_jarr, "name");
                json_object_cset_cstr(part_variant_jobj, "type", "object");

                json_object_cset_cstr(part_variant_name_jobj, "type", "string");
                json_object_cset_string(part_variant_name_jobj, "const", &part_variant->name);
                
                if (part_variant->colorchn != NULL) {
                    json_object_t *part_variant_colorchn_jobj = json_object_vset_empty_object(part_variant_properties_jobj, part_variant->colorchn_alias);

                    json_object_cset_cstr(part_variant_colorchn_jobj, "type", "string");
                    json_object_cset_string(part_variant_colorchn_jobj, "description", part_variant->colorchn_description);
                    json_array_push_string(part_variant_required_jarr, part_variant->colorchn_alias);
                }
            }
        }
    }

    file_t schema_file;
    string_t schema;

    string_init_capacity(&schema, 512);
    json_stringify(&json, &schema);

    if (file_open(&schema_file, COMMAND_SCHEMA_PATH, FILE_OMODE_WR) != EXIT_SUCCESS) {
        string_from_cstr(error_message, "Fail to open file for writing schema.");
        string_deinit(&schema);
        json_deinit(&json);
        return EXIT_FAILURE;
    }
    
    if (file_write_string(&schema_file, &schema) != EXIT_SUCCESS) {
        string_from_cstr(error_message, "Fail to write schema.");
        string_deinit(&schema);
        json_deinit(&json);
        file_close(&schema_file);
        return EXIT_FAILURE;
    }
    
    file_close(&schema_file);
    string_deinit(&schema);
    json_deinit(&json);

    return EXIT_SUCCESS;
}

status_t command_init(matroot_t *matroot, string_t *error_message, json_t *out) {
    json_init_object(out);

    json_object_t *out_jobj = out->jobj;

    if (command_build_schema(matroot, error_message) != EXIT_SUCCESS) {
        json_deinit(out);
        return EXIT_FAILURE;
    }
    
    if (command_write_instruction(matroot, error_message) != EXIT_SUCCESS) {
        json_deinit(out);
        return EXIT_FAILURE;
    }

    json_object_cset_cstr(out_jobj, "status", "ok");
    json_object_cset_cstr(out_jobj, "schema", COMMAND_SCHEMA_PATH);
    json_object_cset_cstr(out_jobj, "instruction", COMMAND_INSTRUCTION_PATH);

    return EXIT_SUCCESS;
}
