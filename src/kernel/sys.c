/**
 * @file sys.c
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2014-2015, Erik Moqvist
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

struct sys_t sys = {
    .tick = 0,
    .on_fatal_callback = sys_stop,
    .std_out_p = NULL
};

extern void timer_tick(void);
extern void thrd_tick(void);

static void sys_tick() {
    sys.tick++;
    timer_tick();
    thrd_tick();
}

#include "sys_port.i"

int sys_module_init()
{
    return (sys_port_module_init());
}

int sys_start(void)
{
    std_module_init();
    log_module_init();
    spin_module_init();
    sem_module_init();
    chan_module_init();
    thrd_module_init();
    sys_port_module_init();

    return (0);
}

void sys_set_on_fatal_callback(void (*callback)(int error))
{
    sys.on_fatal_callback = callback;
}

void sys_set_stdout(chan_t *chan_p)
{
    sys.std_out_p = chan_p;
}

chan_t *sys_get_stdout(void)
{
    return (sys.std_out_p);
}
