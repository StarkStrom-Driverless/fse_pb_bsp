/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#ifndef __SS_BITS_H__
#define __SS_BITS_H__

#include <inttypes.h>

#define SS_BITS_INTEL       0
#define SS_BITS_MOTOROLA    1

static inline uint16_t ss_bits_next_motorola(uint16_t bit) {
    if ((bit & 7) == 0) {
        return bit + 15;
    }

    return bit - 1;
}

static inline uint32_t ss_bits_get(const uint8_t *data, uint16_t len,
                                   uint16_t start_bit, uint8_t length,
                                   uint8_t byte_order) {
    uint32_t value = 0;
    uint16_t bit = start_bit;
    uint8_t i;

    if (length == 0 || length > 32) {
        return 0;
    }

    if (byte_order == SS_BITS_MOTOROLA) {
        for (i = 0; i < length; i++) {
            uint16_t byte = bit >> 3;

            value <<= 1;

            if (byte < len && (data[byte] & (1u << (bit & 7)))) {
                value |= 1u;
            }

            bit = ss_bits_next_motorola(bit);
        }

        return value;
    }

    for (i = 0; i < length; i++) {
        uint16_t byte = (uint16_t) ((start_bit + i) >> 3);

        if (byte < len && (data[byte] & (1u << ((start_bit + i) & 7)))) {
            value |= (1ul << i);
        }
    }

    return value;
}

static inline int32_t ss_bits_get_signed(const uint8_t *data, uint16_t len,
                                         uint16_t start_bit, uint8_t length,
                                         uint8_t byte_order) {
    uint32_t raw = ss_bits_get(data, len, start_bit, length, byte_order);

    if (length >= 32) {
        return (int32_t) raw;
    }

    if (raw & (1ul << (length - 1))) {
        return (int32_t) (raw | (~0ul << length));
    }

    return (int32_t) raw;
}

static inline void ss_bits_set(uint8_t *data, uint16_t len, uint16_t start_bit,
                               uint8_t length, uint32_t value, uint8_t byte_order) {
    uint16_t bit = start_bit;
    uint8_t i;

    if (length == 0 || length > 32) {
        return;
    }

    if (byte_order == SS_BITS_MOTOROLA) {
        for (i = 0; i < length; i++) {
            uint16_t byte = bit >> 3;
            uint32_t src = (value >> (length - 1 - i)) & 1u;

            if (byte < len) {
                if (src) {
                    data[byte] |= (uint8_t) (1u << (bit & 7));
                } else {
                    data[byte] &= (uint8_t) ~(1u << (bit & 7));
                }
            }

            bit = ss_bits_next_motorola(bit);
        }

        return;
    }

    for (i = 0; i < length; i++) {
        uint16_t byte = (uint16_t) ((start_bit + i) >> 3);
        uint8_t pos = (uint8_t) ((start_bit + i) & 7);

        if (byte >= len) {
            continue;
        }

        if ((value >> i) & 1u) {
            data[byte] |= (uint8_t) (1u << pos);
        } else {
            data[byte] &= (uint8_t) ~(1u << pos);
        }
    }
}

#endif // __SS_BITS_H__
