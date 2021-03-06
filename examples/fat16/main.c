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
#include <limits.h>

static struct fs_command_t cmd_ls;
static struct fs_command_t cmd_open;
static struct fs_command_t cmd_close;
static struct fs_command_t cmd_read;
static struct fs_command_t cmd_write;

static char qinbuf[32];
static struct uart_driver_t uart;
static struct shell_args_t shell_args;

static struct spi_driver_t spi;
static struct sd_driver_t sd;
static struct fat16_t fs;

static int is_file_open = 0;
static struct fat16_file_t file;

static int print_dir_name(chan_t *chan_p,
                          const struct fat16_dir_entry_t *entry_p,
                          int width)
{
    size_t size;
    char b;

    size = strlen(entry_p->name);
    chan_write(chan_p, entry_p->name, size);

    if (entry_p->is_dir == 1) {
        b = '/';
        chan_write(chan_p, &b, sizeof(b));
        size++;
    }

    while (size < width) {
        b = ' ';
        chan_write(chan_p, &b, sizeof(b));
        size++;
    }

    return (0);
}

static int format_entry(chan_t *chan_p,
                        const struct fat16_dir_entry_t *entry_p)
{

    /* Print file name with possible blank fill */
    print_dir_name(chan_p, entry_p, 14);

    /* Print modify date/time if requested */
    std_fprintf(chan_p,
                FSTR("%04u-%02u-%02u %02u:%02u:%02u"),
                entry_p->latest_mod_date.year,
                entry_p->latest_mod_date.month,
                entry_p->latest_mod_date.day,
                entry_p->latest_mod_date.hour,
                entry_p->latest_mod_date.minute,
                entry_p->latest_mod_date.second);

    /* Print size if requested */
    if (entry_p->is_dir != 1) {
        std_fprintf(chan_p, FSTR(" %lu"), (unsigned long)entry_p->size);
    }

    std_fprintf(chan_p, FSTR("\r\n"));

    return (0);
}

static int cmd_ls_cb(int argc,
                     const char *argv[],
                     chan_t *out_p,
                     chan_t *in_p,
                     void *arg_p,
                     void *call_arg_p)
{
    UNUSED(in_p);

    struct fat16_dir_t dir;
    struct fat16_dir_entry_t entry;
    const char *path_p;

    if (argc != 2) {
        std_fprintf(out_p, FSTR("Usage: %s <path>\r\n"), argv[0]);
        return (1);
    }

    path_p = argv[1];

    /* Re-open the directory with read option set. */
    fat16_dir_open(&fs, &dir, path_p, O_READ);

    while (fat16_dir_read(&dir, &entry) == 1) {
        format_entry(out_p, &entry);
    }

    fat16_dir_close(&dir);

    return (0);
}

static int cmd_open_cb(int argc,
                       const char *argv[],
                       chan_t *out_p,
                       chan_t *in_p,
                       void *arg_p,
                       void *call_arg_p)
{
    UNUSED(in_p);

    const char *path_p;
    int flags;

    if (argc != 3) {
        std_fprintf(out_p, FSTR("Usage: %s <path> <r | w>\r\n"), argv[0]);
        return (1);
    }

    if (is_file_open == 1) {
        std_fprintf(out_p,
                    FSTR("Cannot have more than one file open. Please close "
                         "current file before opening a new one.\r\n"));
        return (1);
    }

    path_p = argv[1];

    if (argv[2][0] == 'w') {
        flags = (O_CREAT | O_WRITE | O_SYNC);
    } else {
        flags = O_READ;
    }

    /* Re-open the directory with read option set. */
    if (fat16_file_open(&fs, &file, path_p, flags) != 0) {
        std_fprintf(out_p, FSTR("%s: file not found\r\n"), path_p);
        return (1);
    }

    is_file_open = 1;

    return (0);
}

static int cmd_close_cb(int argc,
                        const char *argv[],
                        chan_t *out_p,
                        chan_t *in_p,
                        void *arg_p,
                        void *call_arg_p)
{
    UNUSED(in_p);

    if (argc != 1) {
        std_fprintf(out_p, FSTR("Usage: %s\r\n"), argv[0]);
        return (1);
    }

    if (is_file_open == 0) {
        std_fprintf(out_p, FSTR("No file is open.\r\n"));
        return (1);
    }

    fat16_file_close(&file);

    is_file_open = 0;

    return (0);
}

static int cmd_read_cb(int argc,
                       const char *argv[],
                       chan_t *out_p,
                       chan_t *in_p,
                       void *arg_p,
                       void *call_arg_p)
{
    UNUSED(in_p);

    ssize_t read, n;
    long size;
    char buf[64];

    if (argc == 1) {
        size = INT_MAX;
    } else if (argc == 2) {
        if (std_strtol(argv[1], &size) != 0) {
            std_fprintf(out_p, FSTR("%s: expected posotove integer\r\n"), argv[1]);
            return (1);
        }
    } else {
        std_fprintf(out_p, FSTR("Usage: %s [<size to read>]\r\n"), argv[0]);

        return (1);
    }

    /* A file must be open. */
    if (is_file_open == 0) {
        std_fprintf(out_p, FSTR("The file must be opened before it can be read from.\r\n"));

        return (1);
    }

    /* Read from file. */
    read = 0;

    while (read < size) {
        if ((size - read) < sizeof(buf)) {
            n = (size - read);
        } else {
            n = sizeof(buf);
        }

        if ((n = fat16_file_read(&file, buf, n)) <= 0) {
            break;
        }

        chan_write(out_p, buf, n);
        read += n;
    }

    return (0);
}

static int cmd_write_cb(int argc,
                        const char *argv[],
                        chan_t *out_p,
                        chan_t *in_p,
                        void *arg_p,
                        void *call_arg_p)
{
    UNUSED(in_p);

    if (argc != 2) {
        std_fprintf(out_p, FSTR("Usage: %s <data to write>\r\n"), argv[0]);
        return (1);
    }

    /* A file must be open. */
    if (is_file_open == 0) {
        std_fprintf(out_p, FSTR("The file must be opened before it can be written to.\r\n"));

        return (1);
    }

    fat16_file_write(&file, argv[1], strlen(argv[1]));

    return (0);
}

static void init(void)
{
    sys_start();
    uart_module_init();

    uart_init(&uart, &uart_device[0], 38400, qinbuf, sizeof(qinbuf));
    uart_start(&uart);

    sys_set_stdout(&uart.chout);

    fs_command_init(&cmd_ls,
                    FSTR("/ls"),
                    cmd_ls_cb,
                    NULL);
    fs_command_register(&cmd_ls);

    fs_command_init(&cmd_open,
                    FSTR("/open"),
                    cmd_open_cb,
                    NULL);
    fs_command_register(&cmd_open);

    fs_command_init(&cmd_close,
                    FSTR("/close"),
                    cmd_close_cb,
                    NULL);
    fs_command_register(&cmd_close);

    fs_command_init(&cmd_read,
                    FSTR("/read"),
                    cmd_read_cb,
                    NULL);
    fs_command_register(&cmd_read);

    fs_command_init(&cmd_write,
                    FSTR("/write"),
                    cmd_write_cb,
                    NULL);
    fs_command_register(&cmd_write);


    std_printf(sys_get_info());

    spi_init(&spi,
             &spi_device[0],
             &pin_d53_dev,
             SPI_MODE_MASTER,
             SPI_SPEED_1MBPS,
             0,
             1);

    sd_init(&sd, &spi);
    sd_start(&sd);
    std_printf(FSTR("sd card started\r\n"));
    fat16_init(&fs,
               (fat16_read_t)sd_read_block,
               (fat16_write_t)sd_write_block,
               &sd,
               0);
    fat16_start(&fs);
    std_printf(FSTR("fat16 started\r\n"));
}

int main()
{
    init();

    shell_args.chin_p = &uart.chin;
    shell_args.chout_p = &uart.chout;
    shell_args.username_p = NULL;
    shell_args.password_p = NULL;
    shell_main(&shell_args);

    return (0);
}
