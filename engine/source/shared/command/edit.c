#include "command/utils.h"

status_t command_edit(matroot_t *matroot, string_t *error_message, uint8_t palette_id, uint8_t model_id, uint8_t moption_id, uint8_t *variant_ids, uint8_t *option_ids, json_t *out) {
    json_init_object(out);

    model_t *model = matroot->models.entries + model_id;
    json_object_t *out_jobj = out->jobj;
    
    json_object_cset_int64(out_jobj, "modelId", model_id);
    json_object_cset_int64(out_jobj, "moptionId", moption_id == MATERIAL_INV_ANIM_ID ? -1 : moption_id);

    {
        json_array_t *parts = json_object_cset_empty_array(out_jobj, "parts");
    
        for (size_t part_id = 0; part_id < model->parts.length; part_id++) {
            json_object_t *part_jobj = json_array_push_empty_object(parts);
            uint8_t variant_id = variant_ids[part_id];
            uint8_t option_id = option_ids[part_id];

            if (variant_id == MATERIAL_INV_ANIM_ID) {
                json_object_cset_int64(part_jobj, "variantId", -1);
            } else {
                json_object_cset_int64(part_jobj, "variantId", variant_id);
                json_object_cset_int64(part_jobj, "optionId", option_id == MATERIAL_INV_ANIM_ID ? -1 : option_id);
            }
        }
    }

    pixels_rgba_t pixels;
    bool_t *anims_skipped;

    command_anims_skipped_init(
        matroot, 
        &anims_skipped, 
        model_id, 
        variant_ids, 
        option_ids
    );

    {
        json_array_t *anims = json_object_cset_empty_array(out_jobj, "anims");
        size_t frame_id = 0;

        for (size_t anim_id = 0; anim_id < model->anims.length; anim_id++) {
            anim_t *anim = model->anims.entries + anim_id;
            
            if (anims_skipped[anim_id]) {
                continue;
            }

            json_object_t *anim_jobj = json_array_push_empty_object(anims);

            json_object_cset_string(anim_jobj, "name", &anim->name);
            json_object_cset_int64(anim_jobj, "from", frame_id);
            json_object_cset_int64(anim_jobj, "to", (frame_id += anim->total_frames) - 1);
            json_object_cset_bool(anim_jobj, "loop", anim->is_looping);

            uint16_t *delays = anim->frame_delays;
            json_array_t *delays_jarr =json_object_cset_empty_array(anim_jobj, "delays");

            for (uint8_t delay_id = 0; delay_id < anim->total_frames; delay_id++) {
                json_array_push_double(delays_jarr, (double) delays[delay_id] / 100.0);
            }
        }
    }

    json_object_cset_cstr(out_jobj, "base", COMMAND_TEMP_PATH);
    json_object_cset_cstr(out_jobj, "status", "ok");

    composition_composite(
        matroot,
        anims_skipped,
        palette_id, 
        model_id, 
        moption_id,
        variant_ids, 
        option_ids, 
        &pixels
    );

    file_t temp_file;
    size_t bound = 0;
    
    if (image_encode_bound(&pixels, &bound) != EXIT_SUCCESS) {
        string_from_cstr(error_message, "Fail to calculate pixel bound.");
        json_deinit(out);
        return EXIT_FAILURE;
    }

    uint8_t *buffer = arena_alloc(composition_arena, bound);

    if (image_encode(&pixels, buffer, &bound)) {
        string_from_cstr(error_message, "Fail to encode pixel to png.");
        json_deinit(out);
        return EXIT_FAILURE;
    }
    
    if (file_open(&temp_file, COMMAND_TEMP_PATH, FILE_OMODE_WR)) {
        string_from_cstr(error_message, "Fail to open file for writing png.");
        json_deinit(out);
        return EXIT_FAILURE;
    }
    
    if (file_write_byte(&temp_file, buffer, bound)) {
        string_from_cstr(error_message, "Fail to write png.");
        json_deinit(out);
        file_close(&temp_file);
        return EXIT_FAILURE;
    }
    
    file_close(&temp_file);
    return EXIT_SUCCESS;
}