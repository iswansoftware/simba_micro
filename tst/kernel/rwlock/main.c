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

static int test_one_thread(struct harness_t *harness_p)
{
    struct rwlock_t foo;
    struct time_t timeout;

    BTASSERT(rwlock_init(&foo) == 0);

    BTASSERT(rwlock_reader_get(&foo, NULL) == 0);
    BTASSERT(rwlock_reader_get(&foo, NULL) == 0);
    BTASSERT(rwlock_reader_put(&foo) == 0);

    timeout.seconds = 0;
    timeout.nanoseconds = 1;

    BTASSERT(rwlock_writer_get(&foo, &timeout) == -ETIMEDOUT);
    BTASSERT(rwlock_reader_put(&foo) == 0);

    BTASSERT(rwlock_writer_get(&foo, NULL) == 0);
    BTASSERT(rwlock_reader_get(&foo, &timeout) == -ETIMEDOUT);
    BTASSERT(rwlock_writer_put(&foo) == 0);

    return (0);
}

int main()
{
    struct harness_t harness;
    struct harness_testcase_t harness_testcases[] = {
        { test_one_thread, "test_one_thread" },
        { NULL, NULL }
    };

    sys_start();
    uart_module_init();

    harness_init(&harness);
    harness_run(&harness, harness_testcases);

    return (0);
}
