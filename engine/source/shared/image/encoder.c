#include "image/image.h"
#include "image/const.h"

extern arena_t *image_arena;

FORCE_INLINE void image_filter(const pixels_rgba_t *buffer, uint8_t *unfrgba) {
    const uint8_t *curr_in = (const uint8_t *) buffer->values;
    const uint8_t *prev_in = curr_in;
    uint8_t *curr_out = (uint8_t *) unfrgba;

    size_t stride = buffer->width * IMAGE_PNG_CHANNELS;
    uint8_t *filsub_row = arena_alloc(image_arena, stride);
    uint8_t *filup_row = arena_alloc(image_arena, stride);

    for (size_t y = 0; y < buffer->height; y++) {
        uint8_t *scanline = curr_out;

        scanline[0] = 0;
        curr_out += 1;
        
        uint64_t score_filnone = 0;
        uint64_t score_filsub = 0;
        uint64_t score_filup = 0;

        for (size_t x = 0; x < stride; x++) {
            uint8_t value = curr_in[x];

            score_filnone += abs((int8_t) value);
            curr_out[x] = value;
        }
        
        for (size_t x = 0; x < stride; x++) {
            uint8_t left = x >= IMAGE_PNG_CHANNELS ? curr_in[x - IMAGE_PNG_CHANNELS] : 0;
            uint8_t value = curr_in[x] - left;

            score_filsub += abs((int8_t) value);
            filsub_row[x] = value;
        }

        for (size_t x = 0; x < stride; x++) {
            uint8_t up = y > 0 ? prev_in[x] : 0;
            uint8_t value = curr_in[x] - up;

            score_filup += abs((int8_t) value);
            filup_row[x] = value;
        }

        if (score_filsub < score_filnone && score_filsub < score_filup) {
            scanline[0] = 1;
            memcpy(curr_out, filsub_row, stride);
        } else if (score_filup < score_filnone && score_filup < score_filsub) {
            scanline[0] = 2;
            memcpy(curr_out, filup_row, stride);
        }

        prev_in = curr_in;
        curr_in += stride;
        curr_out += stride;
    }
}

status_t image_encode_bound(const pixels_rgba_t *buffer, size_t *out_length) {
    size_t header = sizeof(IMAGE_PNG_SIGN);
    size_t ihdr_length = sizeof(IMAGE_PNG_IHDR_DATA) + (sizeof(uint32_t) << 1) + IMAGE_PNG_CHUNK_MINLEN;
    size_t iend_length = IMAGE_PNG_CHUNK_MINLEN;

    size_t idat_length = 0;

    if (zlib_deflate_bound(buffer->height * (1 + buffer->width * IMAGE_PNG_CHANNELS), &idat_length) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    idat_length += IMAGE_PNG_CHUNK_MINLEN;
    *out_length = header + ihdr_length + idat_length + iend_length;

    return EXIT_SUCCESS;
}

status_t image_encode(const pixels_rgba_t *buffer, uint8_t *out, size_t *out_length) {
    if (sizeof(IMAGE_PNG_SIGN) > *out_length) {
        return IMAGE_EERR_BFOV;
    }

    memcpy(out, IMAGE_PNG_SIGN, sizeof(IMAGE_PNG_SIGN));
    
    size_t index = sizeof(IMAGE_PNG_SIGN);
    size_t ihdr_length = sizeof(IMAGE_PNG_IHDR_DATA) + (sizeof(uint32_t) << 1) + IMAGE_PNG_CHUNK_MINLEN;
    
    if (index + ihdr_length > *out_length) {
        return IMAGE_EERR_BFOV;
    }
    
    write_be32(out, index, 13);
    memcpy(out + (index += sizeof(uint32_t)), IMAGE_IHDR_ID, sizeof(IMAGE_IHDR_ID));
    
    size_t saved_index = index;
    
    write_be32(out, (index += sizeof(IMAGE_IHDR_ID)), buffer->width);
    write_be32(out, (index += sizeof(uint32_t)), buffer->height);
    memcpy(out + (index += sizeof(uint32_t)), IMAGE_PNG_IHDR_DATA, sizeof(IMAGE_PNG_IHDR_DATA));
    index += sizeof(IMAGE_PNG_IHDR_DATA);
    
    uint32_t crc32 = crc32_reflect(out + saved_index, index - saved_index);
    
    write_be32(out, index, crc32);
    index += 4;

    size_t unfrgba_length = buffer->height * (1 + buffer->width * IMAGE_PNG_CHANNELS);
    size_t idat_length = 0;

    if (zlib_deflate_bound(unfrgba_length, &idat_length) != EXIT_SUCCESS) {
        return IMAGE_EERR_IENP;
    }

    if (index + idat_length + IMAGE_PNG_CHUNK_MINLEN > *out_length) {
        return IMAGE_EERR_BFOV;
    }

    uint8_t *unfrgba = arena_alloc(image_arena, unfrgba_length);
    image_filter(buffer, unfrgba);

    if (zlib_deflate(unfrgba, out + index + sizeof(uint32_t) + sizeof(IMAGE_IDAT_ID), unfrgba_length, &idat_length) != EXIT_SUCCESS) {
        return IMAGE_EERR_IENP;
    }

    write_be32(out, index, idat_length);
    memcpy(out + (index += sizeof(uint32_t)), IMAGE_IDAT_ID, sizeof(IMAGE_IDAT_ID));

    saved_index = index;
    index += sizeof(IMAGE_IDAT_ID) + idat_length;
    crc32 = crc32_reflect(out + saved_index, index - saved_index);

    write_be32(out, index, crc32);
    index += 4;

    if (index + IMAGE_PNG_CHUNK_MINLEN > *out_length) {
        return IMAGE_EERR_BFOV;
    }
    
    write_be32(out, index, 0);
    memcpy(out + (index += sizeof(uint32_t)), IMAGE_IEND_ID, sizeof(IMAGE_IEND_ID));
    saved_index = index;
    index += sizeof(IMAGE_IEND_ID);
    
    crc32 = crc32_reflect(out + saved_index, index - saved_index);
    
    write_be32(out, index, crc32);
    index += 4;

    *out_length = index;
    return EXIT_SUCCESS;
}