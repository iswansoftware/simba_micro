/**
 * @file main.c
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

#define SERVER_ADDRESS 0x12345678
#define CLIENT_ADDRESS 0x87654321

#define DS18B20_ID { 0x28, 0x09, 0x1e, 0xa3, 0x05, 0x00, 0x00, 0x42 }

static struct fs_command_t cmd_set_min_max;

static volatile long temp_min = 230000;
static volatile long temp_max = 290000;

static int cmd_set_min_max_cb(int argc,
                              const char *argv[],
                              chan_t *out_p,
                              chan_t *in_p,
                              void *arg_p,
                              void *call_arg_p)
{
    long min, max;

    UNUSED(in_p);

    if (argc != 3) {
        std_fprintf(out_p, FSTR("2 argument required.\r\n"));
        return (1);
    }

    if ((std_strtol(argv[1], &min) != 0) ||
        (std_strtol(argv[2], &max) != 0)) {
        std_fprintf(out_p,
                    FSTR("bad min or max value '%s' '%s'\r\n"),
                    argv[1],
                    argv[2]);
        return (1);
    }

    temp_min = 10000 * min;
    temp_max = 10000 * max;
    std_fprintf(out_p,
                FSTR("min set to %ld and max set to %ld\r\n"),
                temp_min / 10000,
                temp_max / 10000);

    return (0);
}

static char qinbuf[32];
static struct uart_driver_t uart;
static struct nrf24l01_driver_t nrf24l01;

int main()
{
    struct owi_driver_t owi;
    struct ds18b20_driver_t ds;
    struct owi_device_t devices[4];
    int read_temp;
    long temp, resolution;
    uint8_t state;
    uint8_t id[8] = DS18B20_ID;

    sys_start();
    uart_module_init();

    uart_init(&uart, &uart_device[0], 38400, qinbuf, sizeof(qinbuf));
    uart_start(&uart);

    sys_set_stdout(&uart.chout);

    fs_command_init(&cmd_set_min_max,
                    FSTR("/temp/set_min_max"),
                    cmd_set_min_max_cb,
                    NULL);
    fs_command_register(&cmd_set_min_max);

    nrf24l01_init(&nrf24l01,
                  &spi_device[0],
                  &pin_d10_dev,
                  &pin_d6_dev,
                  &exti_device[1],
                  CLIENT_ADDRESS);
    nrf24l01_start(&nrf24l01);

    /* Initialize temperature sensor. */
    owi_init(&owi, &pin_d5_dev, devices, membersof(devices));
    ds18b20_init(&ds, &owi);

    /* Read temperature periodically. */
    while (1) {
        /* Read temperature. */
        ds18b20_convert(&ds);
        ds18b20_get_temperature(&ds, id, &read_temp);

        temp = read_temp;
        temp = (10000 * (temp >> 4) + 625 * (temp & 0xf));

        /* Update led. */
        if (temp <= temp_min) {
            state = 0x1;
        } else if (temp >= temp_max) {
            state = 0x7;
        } else {
            temp -= temp_min;
            resolution = ((temp_max - temp_min) / 8);
            temp /= resolution;
            state = temp;

            if (state == 0x0) {
                state = 0x1;
            }
        }

        std_printf(FSTR("state = 0x%x\r\n"), (int)state);

        /* Send state to server. */
        nrf24l01_write(&nrf24l01,
                       SERVER_ADDRESS,
                       1,
                       &state,
                       sizeof(state));

        std_printf(FSTR("written state = 0x%x\r\n"), (int)state);
    }

    return (0);
}
