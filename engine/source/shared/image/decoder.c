#include "image/image.h"
#include "image/const.h"

extern arena_t *image_arena;

FORCE_INLINE bool_t image_check_ihdr(const uint8_t *buffer, size_t *index, pixels_rgba_t *out) {
    out->width = read_be32(buffer, *index);
    out->height = read_be32(buffer, *index += sizeof(uint32_t));
    
    *index += sizeof(uint32_t);

    if (memcmp(buffer + *index, IMAGE_PNG_IHDR_DATA, sizeof(IMAGE_PNG_IHDR_DATA)) != 0) {
        return FALSE;
    }
    
    *index += sizeof(IMAGE_PNG_IHDR_DATA);
    return TRUE;
}

FORCE_INLINE int32_t image_paeth_predict(int32_t left, int32_t up, int32_t upleft) {
    int32_t p = (int32_t) left + (int32_t) up - (int32_t) upleft;
    int32_t pleft = abs(p - left);
    int32_t pup = abs(p - up);
    int32_t pupleft = abs(p - upleft);

    return pleft <= pup && pleft <= pupleft ? left :
        pup <= pupleft ? up : upleft;
}

FORCE_INLINE void image_unfilter(const uint8_t *unfrgba, pixels_rgba_t *out) {
    uint8_t *current_out = (uint8_t *) out->values;
    uint8_t *prev_out = current_out;
    const uint8_t *current_in = unfrgba;
    const size_t stride = out->width * IMAGE_PNG_CHANNELS;

    for (size_t y = 0; y < out->height; y++) {
        uint8_t filter = current_in[0];
        current_in++;

        for (size_t x = 0; x < stride; x++) {
            uint8_t left = x >= IMAGE_PNG_CHANNELS ? current_out[x - IMAGE_PNG_CHANNELS] : 0;
            uint8_t up = y > 0 ? prev_out[x] : 0;
            uint8_t upleft = y > 0 && x >= IMAGE_PNG_CHANNELS ? prev_out[x - IMAGE_PNG_CHANNELS] : 0;

            switch (filter) {
                case IMAGE_FILNONE:
                    current_out[x] = current_in[x];
                    break;
                
                case IMAGE_FILSUB:
                    current_out[x] = current_in[x] + left;
                    break;
                
                case IMAGE_FILUP:
                    current_out[x] = current_in[x] + up;
                    break;
                
                case IMAGE_FILAVG:
                    current_out[x] = current_in[x] + ((int32_t) (left + up) >> 1);
                    break;

                case IMAGE_FILPAETH:
                    current_out[x] = current_in[x] + image_paeth_predict(left, up, upleft);
                    break;

                default:
                    current_out[x] = current_in[x];
                    break;
            }
        }

        prev_out = current_out;
        current_in += stride;
        current_out += stride;
    }
}

status_t image_decode(const uint8_t *buffer, size_t buffer_length, pixels_rgba_t *out) {
    if (memcmp(buffer, IMAGE_PNG_SIGN, sizeof(IMAGE_PNG_SIGN)) != 0) {
        return IMAGE_DERR_FNPF;
    }

    size_t index = sizeof(IMAGE_PNG_SIGN);
    size_t saved_index = 0;
    bool_t found_ihdr = FALSE;

    uint8_t *idat = arena_alloc(image_arena, buffer_length - index);
    size_t idat_length = 0;
    
    while (TRUE) {
        if (index + IMAGE_PNG_CHUNK_MINLEN > buffer_length) {
            return IMAGE_DERR_PFDC;
        }
        
        uint32_t chunk_length = read_be32(buffer, index);
        
        index += sizeof(uint32_t);
        saved_index = index;
        index += sizeof(IMAGE_IHDR_ID);
        
        if (index + chunk_length + sizeof(uint32_t) > buffer_length) {
            return IMAGE_DERR_PFDC;
        }

        if (memcmp(buffer + saved_index, IMAGE_IHDR_ID, sizeof(IMAGE_IHDR_ID)) == 0) {
            if (found_ihdr || chunk_length != IMAGE_PNG_IHDRLEN) {
                return IMAGE_DERR_PFDC;
            }
            
            if (!image_check_ihdr(buffer, &index, out)) {
                return IMAGE_DERR_PFDC;
            }

            found_ihdr = TRUE;
        } else if (memcmp(buffer + saved_index, IMAGE_IDAT_ID, sizeof(IMAGE_IDAT_ID)) == 0) {
            if (!found_ihdr) {
                return IMAGE_DERR_PFDC;
            }

            memcpy(idat + idat_length, buffer + index, chunk_length);
            idat_length += chunk_length;
            index += chunk_length;
        } else if (memcmp(buffer + saved_index, IMAGE_IEND_ID, sizeof(IMAGE_IEND_ID)) == 0) {
            if (!found_ihdr || chunk_length != 0) {
                return IMAGE_DERR_PFDC;
            }
            break;
        } else {
            return IMAGE_DERR_PFDC;
        }

        uint32_t crc32 = read_be32(buffer, index);
        
        if (crc32_reflect(buffer + saved_index, index - saved_index) != crc32) {
            return IMAGE_DERR_PFDC;
        }
        
        index += sizeof(uint32_t);
    }

    size_t unfrgba_length = out->height * (1 + out->width * IMAGE_PNG_CHANNELS);
    uint8_t *unfrgba = arena_alloc(image_arena, unfrgba_length);

    if (zlib_inflate(idat, unfrgba, idat_length, unfrgba_length) != EXIT_SUCCESS) {
        return IMAGE_DERR_PFDC;
    }

    out->values = memory_alloc(out->width * out->height * IMAGE_PNG_CHANNELS);

    image_unfilter(unfrgba, out);
    return EXIT_SUCCESS;
}