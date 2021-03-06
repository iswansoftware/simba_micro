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

struct command_t {
    char *command_p;
    int res;
};

int test_print(struct harness_t *harness_p)
{
    struct log_object_t foo;
    struct log_object_t bar;

    /* Initialize the log objects. */
    BTASSERT(log_object_init(&foo,
                             "foo",
                             LOG_UPTO(INFO)) == 0);
    BTASSERT(log_object_init(&bar,
                             "bar",
                             LOG_UPTO(DEBUG)) == 0);

    /* Write on INFO level. */
    BTASSERT(log_object_print(&foo, LOG_INFO, FSTR("x = %d\r\n"), 1) == 1);
    BTASSERT(log_object_print(&bar, LOG_INFO, FSTR("y = %d\r\n"), 2) == 1);

    /* Write on DEBUG level. */
    BTASSERT(log_object_print(&foo, LOG_DEBUG, FSTR("m = %d\r\n"), 3) == 0);
    BTASSERT(log_object_print(&bar, LOG_DEBUG, FSTR("n = %d\r\n"), 4) == 1);

    /* Write using the thread log mask instead of the log object
     * mask. */
    BTASSERT(log_object_print(NULL, LOG_DEBUG, FSTR("k = %d\r\n"), 5) == 0);
    BTASSERT(log_object_print(NULL, LOG_ERR, FSTR("l = %d\r\n"), 6) == 1);

    return (0);
}

int test_object(struct harness_t *harness_p)
{
    struct log_object_t foo;

    /* Initialize the log objects. */
    BTASSERT(log_object_init(&foo,
                             "foo",
                             LOG_UPTO(INFO)) == 0);

    /* Add our object, and the remove it. */
    BTASSERT(log_add_object(&foo) == 0);
    BTASSERT(log_remove_object(&foo) == 0);
    BTASSERT(log_remove_object(&foo) == 1);

    return (0);
}

int test_handler(struct harness_t *harness_p)
{
    struct log_object_t foo;
    struct log_handler_t handler;

    /* Initialize the log objects. */
    BTASSERT(log_object_init(&foo,
                             "foo",
                             LOG_UPTO(INFO)) == 0);
    BTASSERT(log_handler_init(&handler, sys_get_stdout()) == 0);
    
    /* This should be printed once for the default handler. */
    BTASSERT(log_object_print(&foo,
                              LOG_INFO,
                              FSTR("one handler\r\n")) == 1);

    /* Add our handler. */
    BTASSERT(log_add_handler(&handler) == 0);

    /* This should be printed twice, one for the default handler and
     * once for our handler. */
    BTASSERT(log_object_print(&foo,
                              LOG_INFO,
                              FSTR("two handlers\r\n")) == 2);

    /* Remove our handler. */
    BTASSERT(log_remove_handler(&handler) == 0);
    BTASSERT(log_remove_handler(&handler) == 1);

    /* This should be printed once for the default handler. */
    BTASSERT(log_object_print(&foo,
                              LOG_INFO,
                              FSTR("one handler again\r\n")) == 1);

    return (0);
}

int test_fs(struct harness_t *harness_p)
{
    char command[64];
    struct command_t *command_p;
    struct command_t commands[] = {
        { "/kernel/log/list", 0 },
        { "/kernel/log/print foo", 0 },
        { "/kernel/log/set_log_mask log 0xff", 0 },
        
        { "/kernel/log/list d", -EINVAL },
        { "/kernel/log/print d d", -EINVAL },
        { "/kernel/log/set_log_mask invalid_object 0xff", -EINVAL },
        { "/kernel/log/set_log_mask missing_mask", -EINVAL },
        { "/kernel/log/set_log_mask bad_mask ds", -EINVAL },
        { NULL, 0 }
    };

    command_p = &commands[0];

    while (command_p->command_p != NULL) {
        strcpy(command, command_p->command_p);
        BTASSERT(fs_call(command,
                         NULL,
                         sys_get_stdout(),
                         NULL) == command_p->res);

        command_p++;
    }

    return (0);
}

int main()
{
    struct harness_t harness;
    struct harness_testcase_t harness_testcases[] = {
        { test_print, "test_print" },
        { test_object, "test_object" },
        { test_handler, "test_handler" },
        { test_fs, "test_fs" },
        { NULL, NULL }
    };

    sys_start();
    uart_module_init();

    harness_init(&harness);
    harness_run(&harness, harness_testcases);

    return (0);
}
