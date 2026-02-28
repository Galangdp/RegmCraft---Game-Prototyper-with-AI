add_library(pnglib
    "image/zlib.c"
)

target_include_directories(pnglib PRIVATE "${CMAKE_SOURCE_DIR}/include/extern")
target_link_libraries(pnglib PRIVATE zlib)
target_link_libraries(pnglib PUBLIC global_interface)

add_library(sharedlib
    "image/const.c"
    "image/decoder.c"
    "image/encoder.c"
    "image/image.c"
    "image/pixels_array.c"

    "json/parser/double.c"
    "json/parser/identifier.c"
    "json/parser/int64.c"
    "json/parser/key.c"
    "json/parser/parser.c"
    "json/parser/recovery.c"
    "json/parser/search_colon.c"
    "json/parser/search_comma.c"
    "json/parser/search_contmark.c"
    "json/parser/search_eof.c"
    "json/parser/string_key.c"
    "json/parser/string_value.c"
    "json/parser/value.c"
    "json/stringifier/array.c"
    "json/stringifier/object.c"
    "json/stringifier/stringifier.c"
    "json/stringifier/value.c"
    "json/array.c"
    "json/const.c"
    "json/idmap.c"
    "json/json.c"
    "json/object.c"
    "json/psst.c"
    "json/value.c"

    "jsonmat/colorrp.c"
    "jsonmat/const.c"
    "jsonmat/hexstring.c"
    "jsonmat/jsonmat.c"
    "jsonmat/matroot.c"
    "jsonmat/model.c"
    "jsonmat/palette.c"
    "jsonmat/part_variant.c"

    "material/color_stat.c"
    "material/const.c"
    "material/material.c"

    "command/create.c"
    "command/edit.c"
    "command/init.c"
    "command/process.c"
    "command/utils.c"

    "composition/composition.c"

    "utils/arena.c"
    "utils/console.c"
    "utils/crc32.c"
    "utils/file.c"
    "utils/hash.c"
    "utils/string_map.c"
    "utils/string.c"
)

target_link_libraries(sharedlib PUBLIC pnglib)