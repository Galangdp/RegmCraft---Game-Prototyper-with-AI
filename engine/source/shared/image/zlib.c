#include "image/zlib.h"
#include "zlib/zlib.h"

extern arena_t *image_arena;

LOCAL voidpf zlib_arena_alloc(voidpf opaque, uInt items, uInt size) {
    return arena_alloc((arena_t *) opaque, items * size);
}

LOCAL void zlib_arena_free(voidpf opaque, voidpf ptr) {
    (void) opaque; (void) ptr;
}

status_t zlib_inflate(const uint8_t *idat, uint8_t *unfrgba, size_t idat_length, size_t unfrgba_length) {
    z_stream stream = {
        .zalloc = zlib_arena_alloc,
        .zfree = zlib_arena_free,
        .opaque = image_arena,
        .next_in = (Bytef *) idat,
        .next_out = unfrgba,
        .avail_in = idat_length,
        .avail_out = unfrgba_length
    };

    if (inflateInit(&stream) != Z_OK) {
        return EXIT_FAILURE;
    }

    status_t retcode = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    
    return retcode == Z_STREAM_END ? EXIT_SUCCESS : EXIT_FAILURE;
}

status_t zlib_deflate_bound(size_t unfrgba_length, size_t *idat_length) {
    z_stream stream = {0};
    
    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        return EXIT_FAILURE;
    }

    *idat_length = deflateBound(&stream, unfrgba_length);

    deflateEnd(&stream);
    return EXIT_SUCCESS;
}

status_t zlib_deflate(const uint8_t *unfrgba, uint8_t *idat, size_t unfrgba_length, size_t *idat_length) {
    z_stream stream = {
        .zalloc = zlib_arena_alloc,
        .zfree = zlib_arena_free,
        .opaque = image_arena,
    };

    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        return EXIT_FAILURE;
    }

    stream.next_in = (Bytef *) unfrgba;
    stream.next_out = idat;
    stream.avail_in = unfrgba_length;
    stream.avail_out = *idat_length;

    status_t retcode = deflate(&stream, Z_FINISH);
    deflateEnd(&stream);

    *idat_length = stream.total_out;
    return retcode == Z_STREAM_END ? EXIT_SUCCESS : EXIT_FAILURE;
}
