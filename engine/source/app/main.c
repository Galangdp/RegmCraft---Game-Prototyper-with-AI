#include <time.h>
#include "json/json.h"
#include "jsonmat/jsonmat.h"
#include "utils/file.h"
#include "image/image.h"
#include "utils/console.h"
#include "command/command.h"
#include "composition/composition.h"

const char CSTR_FTL_TPFRONT[] = "{\"status\":\"fatal\",\"message\":\"";
const char CSTR_ERR_TPFRONT[] = "{\"status\":\"error\",\"message\":\"";
const char CSTR_ALE_TPBACK[] = "\"}";

int main(int argc, const char *argv[]) {
    arena_t perm_arena;
    arena_t temp_arena;
    
    json_idmap_t json_idmap;
    string_map_t color_map;
    color_stats_t color_stats;
    string_t error_message;

    matroot_t matroot;
    
    {
        file_t matroot_file;
        string_t buffer;

        if (file_open(&matroot_file, MATERIAL_MATROOT_PATH, FILE_OMODE_RD) != EXIT_SUCCESS) {
            console_write_cstr("{\"status\":\"fatal\",\"message\":\"Error when open matroot.\"}");
            return EXIT_FAILURE;
        }

        string_init_capacity(&buffer, 500);

        if (file_read_string(&matroot_file, &buffer) != EXIT_SUCCESS) {
            console_write_cstr("{\"status\":\"fatal\",\"message\":\"Error when read matroot.\"}");
            return EXIT_FAILURE;
        }

        if (jsonmat_decode_material(&matroot, &perm_arena, &temp_arena, &json_idmap, &color_map, &color_stats, &buffer, &error_message) != EXIT_SUCCESS) {
            string_t temp;

            string_from_cstr(&temp, CSTR_FTL_TPFRONT);
            string_push_string(&temp, &error_message);
            string_push_cstr(&temp, CSTR_ALE_TPBACK);
            console_write(&temp);
            return EXIT_FAILURE;
        }
    }

    srand(time(NULL));
    composition_ctx_set_arena(&temp_arena);

    json_t in;
    json_t out;
    string_t buffer;

    while (TRUE) {
        console_read(&buffer);
        bool_t is_success = TRUE;
        
        if (json_parse(&in, &buffer, TRUE) != EXIT_SUCCESS) {
            string_from_cstr(&error_message, "Cant parse json, invalid json are passed.");
            is_success = FALSE;
        } else if (command_process(&matroot, &error_message, &in, &out) != EXIT_SUCCESS) {
            is_success = FALSE;
        }
        
        json_deinit(&in);
        string_clear(&buffer);
        
        if (is_success) {
            json_stringify(&out, &buffer);
            json_deinit(&out);
        } else {
            string_push_cstrl(&buffer, CSTR_ERR_TPFRONT, sizeof(CSTR_ERR_TPFRONT) - 1);
            string_push_string(&buffer, &error_message);
            string_push_cstrl(&buffer, CSTR_ALE_TPBACK, sizeof(CSTR_ALE_TPBACK) - 1);
        }

        console_write(&buffer);
        arena_reset(&temp_arena);
        string_deinit(&buffer);
    }

    return EXIT_SUCCESS;
}
