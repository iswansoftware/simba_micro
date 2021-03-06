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

struct test_time_t {
    struct time_t time_in;
    sys_tick_t tick;
    struct time_t time_out;
};

static void on_fatal(int error)
{
    std_printf(FSTR("on_fatal: error: %d\r\n"), error);
}

int test_set_on_fatal_callback(struct harness_t *harness_p)
{
    sys_set_on_fatal_callback(on_fatal);
    ASSERT(0);

    return (0);
}

int test_info(struct harness_t *harness_p)
{
    std_printf(sys_get_info());

    return (0);
}

int test_time(struct harness_t *harness_p)
{
    int i;
    struct time_t time;
    sys_tick_t tick;

    static struct test_time_t test_times[] = {
        {
            .time_in = { .seconds = 1, .nanoseconds = 0 },
            .tick = 100,
            .time_out = { .seconds = 1, .nanoseconds = 0 }
        },

        {
            .time_in = { .seconds = 2, .nanoseconds = 0 },
            .tick = 200,
            .time_out = { .seconds = 2, .nanoseconds = 0 }
        },

        {
            .time_in = { .seconds = 2, .nanoseconds = 10000000 },
            .tick = 201,
            .time_out = { .seconds = 2, .nanoseconds = 10000000 }
        },
        
        /* nanoseconds rounded up. */
        {
            .time_in = { .seconds = 0, .nanoseconds = 15000000 },
            .tick = 2,
            .time_out = { .seconds = 0, .nanoseconds = 20000000 }
        },
        
        /* All zeroes. */
        {
            .time_in = { .seconds = 0, .nanoseconds = 0 },
            .tick = 0,
            .time_out = { .seconds = 0, .nanoseconds = 0 }
        },
        
        /* All zeroes. */
        {
            .time_in = { .seconds = 4325, .nanoseconds = 735000000 },
            .tick = 432574,
            .time_out = { .seconds = 4325, .nanoseconds = 740000000 }
        },
        
        /* Max time. */
        {
            .time_in = { .seconds = 4294967295 / 2, .nanoseconds = 0 },
            .tick = 214748364700,
            .time_out = { .seconds = 4294967295 / 2, .nanoseconds = 0 }
        },
                
        /* Max tick. */
        {
            .time_in = { .seconds = 42949672, .nanoseconds = 949999999 },
            .tick = 4294967295,
            .time_out = { .seconds = 42949672, .nanoseconds = 950000000 }
        }
    };

    std_printf(FSTR("SYS_TICK_FREQUENCY = %d\r\n"), SYS_TICK_FREQUENCY);

    for (i = 0; i < membersof(test_times); i++) {
        /* Convertion from the time struct to ticks. */
        tick = t2st(&test_times[i].time_in);
        BTASSERT(tick == test_times[i].tick);
        
        /* Convertion from ticks to the time struct. */
        st2t(tick, &time);
        BTASSERT(time.seconds == test_times[i].time_out.seconds)
        BTASSERT(time.nanoseconds == test_times[i].time_out.nanoseconds)
    }

    return (0);
}

int main()
{
    struct harness_t harness;
    struct harness_testcase_t harness_testcases[] = {
        { test_set_on_fatal_callback, "test_set_on_fatal_callback" },
        { test_info, "test_info" },
        { test_time, "test_time" },
        { NULL, NULL }
    };

    sys_start();
    uart_module_init();

    harness_init(&harness);
    harness_run(&harness, harness_testcases);

    return (0);
}
