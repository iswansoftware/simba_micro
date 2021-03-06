/**
 * @file mcus/atmega328p/mcu.c
 * @version 0.5.0
 *
 * @section License
 * Copyright (C) 2014-2016, Erik Moqvist
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * This file is part of the Simba project.
 */

#include "simba.h"

struct pin_device_t pin_device[PIN_DEVICE_MAX] = {
    /* PIN D */
    { .sfr_p = &PIND, .mask = _BV(PIND0) },
    { .sfr_p = &PIND, .mask = _BV(PIND1) },
    { .sfr_p = &PIND, .mask = _BV(PIND2) },
    { .sfr_p = &PIND, .mask = _BV(PIND3) },
    { .sfr_p = &PIND, .mask = _BV(PIND4) },
    { .sfr_p = &PIND, .mask = _BV(PIND5) },
    { .sfr_p = &PIND, .mask = _BV(PIND6) },
    { .sfr_p = &PIND, .mask = _BV(PIND7) },

    /* PIN B */
    { .sfr_p = &PINB, .mask = _BV(PINB0) },
    { .sfr_p = &PINB, .mask = _BV(PINB1) },
    { .sfr_p = &PINB, .mask = _BV(PINB2) },
    { .sfr_p = &PINB, .mask = _BV(PINB3) },
    { .sfr_p = &PINB, .mask = _BV(PINB4) },
    { .sfr_p = &PINB, .mask = _BV(PINB5) },

    /* PIN C */
    { .sfr_p = &PINC, .mask = _BV(PINC0) },
    { .sfr_p = &PINC, .mask = _BV(PINC1) },
    { .sfr_p = &PINC, .mask = _BV(PINC2) },
    { .sfr_p = &PINC, .mask = _BV(PINC3) },
    { .sfr_p = &PINC, .mask = _BV(PINC4) },
    { .sfr_p = &PINC, .mask = _BV(PINC5) },
};

struct exti_device_t exti_device[EXTI_DEVICE_MAX] = {
    { .drv_p = NULL, .pin_p = &pin_device[2], .id = 0 },
    { .drv_p = NULL, .pin_p = &pin_device[3], .id = 1 }
};

struct spi_device_t spi_device[SPI_DEVICE_MAX] = {
    {
        .drv_p = NULL,
        .mosi_p = &pin_d11_dev,
        .miso_p = &pin_d12_dev,
        .sck_p = &pin_d13_dev,
        .sem = { .count = 1, .head_p = NULL }
    }
};

struct uart_device_t uart_device[UART_DEVICE_MAX] = {
    { .drv_p = NULL, .sfr_p = &UCSR0A }
};

struct pwm_device_t pwm_device[PWM_DEVICE_MAX] = {
    { .index = 0, .pin_dev_p = &pin_device[9] },  /* timer1 */
    { .index = 1, .pin_dev_p = &pin_device[10] }, /* timer1 */
    { .index = 2, .pin_dev_p = &pin_device[11] }, /* timer2 */
    { .index = 3, .pin_dev_p = &pin_device[3] }   /* timer2 */
};

struct adc_device_t adc_device[ADC_DEVICE_MAX] = {
    { .jobs = { .head_p = NULL, .tail_p = NULL } },
};
