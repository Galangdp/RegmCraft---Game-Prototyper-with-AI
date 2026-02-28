#ifndef IMAGE_CONST_H
#define IMAGE_CONST_H

#include "utils/types.h"

#define IMAGE_PNG_CHANNELS      4
#define IMAGE_PNG_IHDRLEN       13

#define IMAGE_PNG_CHUNK_MINLEN  12

#define IMAGE_FILNONE           0
#define IMAGE_FILSUB            1
#define IMAGE_FILUP             2
#define IMAGE_FILAVG            3
#define IMAGE_FILPAETH          4

extern const char IMAGE_IHDR_ID[4];
extern const char IMAGE_IDAT_ID[4];
extern const char IMAGE_IEND_ID[4];
extern const uint8_t IMAGE_PNG_SIGN[8];
extern const uint8_t IMAGE_PNG_IHDR_DATA[5];

#endif /* IMAGE_CONST_H */ 