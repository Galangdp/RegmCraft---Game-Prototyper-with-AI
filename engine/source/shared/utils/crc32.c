#include "utils/crc32.h"

#define CRC_BITS_PER_BYTE           8
#define CRC_TABLE_LEN               256

#define CRC_INIT                    0xFFFFFFFF
#define CRC_XORVAL                  0xFFFFFFFF

#ifndef CRC_OREF
#define CRC_POLY                    0x4C11DB7
#define CRC_MASK                    0x80000000
#define CRC_SHIFT                   24

LOCAL uint32_t crc_table[CRC_TABLE_LEN];

void crc32_init() {
    for (uint16_t i = 0; i < CRC_TABLE_LEN; i++) {
        uint32_t crc = i << CRC_SHIFT;

        for (uint8_t j = 0; j < CRC_BITS_PER_BYTE; j++) {
            if ((crc & CRC_MASK) != 0) {
                crc <<= 1;
                crc ^= CRC_POLY;
            } else {
                crc <<= 1;
            }
        }

        crc_table[i] = crc;
    }
}

uint32_t crc32(const uint8_t *buffer, size_t length) {
    uint32_t crc = CRC_INIT;

    for (size_t i = 0; i < length; i++) {
        uint8_t data = buffer[i] ^ (crc >> CRC_SHIFT);
        crc = crc_table[data] ^ (crc << CRC_BITS_PER_BYTE);
    }

    return crc ^ CRC_XORVAL;
}
#endif

#ifndef CRC_NOREF
#define CRC_RPOLY                   0xEDB88320
#define CRC_RMASK                   0x01
#define CRC_BYTE_MASK               0xFF

LOCAL uint32_t crc_table_reflect[CRC_TABLE_LEN];

void crc32_init_reflect() {
    for (uint16_t i = 0; i < CRC_TABLE_LEN; i++) {
        uint32_t crc = i;

        for (uint8_t j = 0; j < CRC_BITS_PER_BYTE; j++) {
            if ((crc & CRC_RMASK) != 0) {
                crc >>= 1;
                crc ^= CRC_RPOLY;
            } else {
                crc >>= 1;
            }
        }

        crc_table_reflect[i] = crc;
    }
}

uint32_t crc32_reflect(const uint8_t *buffer, size_t length) {
    uint32_t crc = CRC_INIT;

    for (size_t i = 0; i < length; i++) {
        uint8_t data = (buffer[i] ^ crc) & CRC_BYTE_MASK;
        crc = crc_table_reflect[data] ^ (crc >> CRC_BITS_PER_BYTE);
    }

    return crc ^ CRC_XORVAL;
}

#endif