/**
 * @file main.c
 * @version 0.5.0
 *
 * @section License
 * Copyright (C) 2015-2016, Erik Moqvist
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

#include "bootloader.h"

#if !defined(APPLICATION_ADDRESS)
#    define APPLICATION_ADDRESS                      0x00000000
#endif

#if !defined(APPLICATION_SIZE)
#    define APPLICATION_SIZE                         0x20000000
#endif

static struct console_t console;
static struct uart_driver_t uart;
static uint8_t uart_rx_buffer[32];

int main()
{
    struct flash_driver_t flash;

    /* Start the system. */
    sys_start();

    /* Initialize the pin moudle. */
    pin_module_init();

    /* Initialize and start the UART. */
    uart_module_init();
    uart_init(&uart,
              &uart_device[0],
              115200,
              uart_rx_buffer,
              sizeof(uart_rx_buffer));
    uart_start(&uart);

    console_module_init();

    /* Initialize the flash memory objects. */
    flash_module_init();
    flash_init(&flash, &flash_device[0]);

    /* Print the bootloader application information. */
    sys_set_stdout(&uart.chout);
    std_printf(sys_get_info());

    /* Initialize the bootloader object and enter the main loop. */
    console_init(&console,
                 &uart.chin,
                 &uart.chout,
                 APPLICATION_ADDRESS,
                 APPLICATION_SIZE,
                 &flash);

    /* Enter the bootloader main loop. */
    console_main(&console);

    return (0);
}
