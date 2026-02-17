/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#ifndef __SS_MAKROS_H__
#define __SS_MAKROS_H__

/**
 *  GPIO Makros
*/
#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank) - 'A') << 4) | (num))
#define PINNO(pin) (pin & 15)
#define PINBANK(pin) (pin >> 4)

/**
 *  TIMER Makros
*/
#define TIMCH(channel, addr) ((uint32_t)(addr) | ((uint32_t)(channel) << 28))
#define TIM(id) (0x0FFFFFFF & id)
#define CH(id) (id >> 28)

/**
 * Ethernet Makros
 */
#define SS_ETH_IP(a, b, c, d)           (uint32_t)((d << 24) | (c << 16) | (b << 8) | a)
#define SS_ETH_MAC(a, b, c, d, e, f) \
    ((uint64_t)(f) << 40) | \
    ((uint64_t)(e) << 32) | \
    ((uint64_t)(d) << 24) | \
    ((uint64_t)(c) << 16) | \
    ((uint64_t)(b) << 8)  | \
    ((uint64_t)(a))
#define SS_ETH_IP_GET_POS(ip, pos)      (0xFF & (a >> pos * 8))



/**
 * Arrays
 */
#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])



#endif
