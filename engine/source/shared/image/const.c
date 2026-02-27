#include "image/const.h"

const char IMAGE_IHDR_ID[4] = "IHDR";
const char IMAGE_IDAT_ID[4] = "IDAT";
const char IMAGE_IEND_ID[4] = "IEND";
const uint8_t IMAGE_PNG_SIGN[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
const uint8_t IMAGE_PNG_IHDR_DATA[5] = { 0x08, 0x06, 0x00, 0x00, 0x00 };