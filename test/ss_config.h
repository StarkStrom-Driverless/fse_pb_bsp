/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#ifndef _SS_CONFIG_H_
#define _SS_CONFIG_H_

/* Set to 1 to compile the module, 0 to exclude it. */

#define COMPILE_SS_CLOCK        1
#define COMPILE_SS_GPIO         1
#define COMPILE_SS_RTOS         1
#define COMPILE_SS_DELAY        1
#define COMPILE_SS_SYSTICK      1

#define COMPILE_SS_ADC          1
#define COMPILE_SS_CAN          1
#define COMPILE_SS_CANBOOT      1
#define COMPILE_SS_ETH          0
#define COMPILE_SS_FM           1
#define COMPILE_SS_FSM          1
#define COMPILE_SS_INIT         1
#define COMPILE_SS_IOB          1
#define COMPILE_SS_LEDS         1
#define COMPILE_SS_PID          1
#define COMPILE_SS_PWM          1
#define COMPILE_SS_SPI          1
#define COMPILE_SS_WATCHDOG     1
#define COMPILE_SS_UART         1
#define COMPILE_SS_TUI          1

#endif