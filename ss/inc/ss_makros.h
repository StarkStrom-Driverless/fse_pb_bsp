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

#endif
