add_library(zlib
    "zlib/adler32.c"
    "zlib/deflate.c"
    "zlib/inffast.c"
    "zlib/inflate.c"
    "zlib/inftrees.c"
    "zlib/trees.c"
    "zlib/zutil.c"
)

target_compile_definitions(zlib PRIVATE
    Z_NO_STDIO
    Z_NO_GZCOMPRESS
    Z_NO_STDIO
    Z_NO_ERRNO
    Z_NO_ZLIB_COMPAT
    NO_GZIP
)

target_include_directories(zlib PRIVATE "${CMAKE_SOURCE_DIR}/include/extern")
