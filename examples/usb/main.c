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

static char qinbuf[32];
static struct uart_driver_t uart;
static struct shell_args_t shell_args;

static struct usb_host_driver_t usb;
static struct usb_host_device_t host_devices[1];

static struct usb_host_class_hid_driver_t hid;
static struct usb_host_class_hid_device_t hid_devices[1];

static struct usb_host_class_mass_storage_driver_t mass_storage;
static struct usb_host_class_mass_storage_device_t mass_storage_devices[1];

static struct fs_command_t cmd_state;

extern void state();

static int cmd_state_cb(int argc,
                        const char *argv[],
                        chan_t *out_p,
                        chan_t *in_p,
                        void *arg_p,
                        void *call_arg_p)
{
    state();

    return (0);
}

/**
 * A thread that waits for a mouse to be attached and then reads
 * reports from it and prints them to standard output.
 */
static THRD_STACK(mouse_stack, 1024);
static void *mouse_main(void *arg_p)
{
    struct usb_host_device_t *device_p = arg_p;
    int8_t report[5];
    int i, endpoint = 1;

    thrd_set_name("mouse");

    while (1) {
        /* Read a report from the mouse. */
        if (usb_host_device_read(device_p, endpoint, report, sizeof(report))
            != sizeof(report)) {
            std_printf(FSTR("Failed to read the report.  USB device detached?\r\n"));

            return (NULL);
        }

        for (i = 0; i < 3; i++) {
            if (report[0] & (1 << i)) {
                std_printf(FSTR("Button %d pressed.\r\n"), i);
            }
        }

        if (report[1] != 0) {
            std_printf(FSTR("Move %d on the x axis.\r\n"), report[1]);
        }

        if (report[2] != 0) {
            std_printf(FSTR("Move %d on the y axis.\r\n"), report[2]);
        }

        if (report[3] > 0) {
            std_printf(FSTR("Scroll up.\r\n"));
        } else if (report[3] < 0) {
            std_printf(FSTR("Scroll down.\r\n"));
        }
    }

    return (NULL);
}

static int handle_message_add(struct usb_message_add_t *msg_p)
{
    struct usb_host_device_t *device_p;
    uint8_t *buf_p;
    size_t size;
    int i, j;
    struct usb_descriptor_configuration_t *conf_p = NULL;
    struct usb_descriptor_interface_t *int_p = NULL;
    uint8_t buf[512];

    device_p = usb_host_device_open(&usb, msg_p->device);

    buf_p = device_p->descriptors.buf;
    size = device_p->descriptors.size;

    /* Iterate over all interfaces searching for a HID mouse. */
    for (i = 1;
         (conf_p = usb_desc_get_configuration(buf_p, size, i)) != NULL;
         i++) {
        for (j = 0;
             (int_p = usb_desc_get_interface(buf_p, size, i, j)) != NULL;
             j++) {
            if ((int_p->interface_class == USB_CLASS_HID)
                && (int_p->interface_protocol == USB_CLASS_HID_PROTOCOL_MOUSE)) {
                std_printf(FSTR("A HID mouse interface was found.\r\n"));
                break;
            }

            if (int_p->interface_class == USB_CLASS_MASS_STORAGE) {
                std_printf(FSTR("A MASS_STORAGE interface was found.\r\n"));
                std_printf(FSTR("reading from USB device\r\n"));
                
                usb_host_class_mass_storage_device_read(device_p,
                                                        buf,
                                                        0,
                                                        512);
                
                std_printf(FSTR("read from USB device\r\n"));
                
                return (0);
            }
        }

        if (int_p != NULL) {
            break;
        }
    }

    if (int_p == NULL) {
        usb_host_device_close(&usb, msg_p->device);

        return (1);
    }

    return (thrd_spawn(mouse_main,
                       device_p,
                       0,
                       mouse_stack,
                       sizeof(mouse_stack)) == NULL);
}

static THRD_STACK(usb_control_stack, 1024);
static void *usb_control_main(void *arg_p)
{
    union usb_message_t message;

    thrd_set_name("usb_control");

    while (1) {
        chan_read(&usb.control, &message, sizeof(message));

        std_printf(FSTR("The USB control thread read a message of type %d\r\n"),
                   message.header.type);

        switch (message.header.type) {

        case USB_MESSAGE_TYPE_ADD:
            handle_message_add(&message.add);
            break;

        default:
            break;
        }
    }

    return (NULL);
}

static void init(void)
{
    sys_start();
    uart_module_init();

    uart_init(&uart, &uart_device[0], 38400, qinbuf, sizeof(qinbuf));
    uart_start(&uart);

    sys_set_stdout(&uart.chout);

    std_printf(sys_get_info());

    fs_command_init(&cmd_state,
                    FSTR("/state"),
                    cmd_state_cb,
                    NULL);
    fs_command_register(&cmd_state);

    /* Initialize the USB host driver. */
    usb_host_init(&usb,
                  &usb_device[0],
                  host_devices,
                  membersof(host_devices));

    usb_host_class_hid_init(&hid,
                            &usb,
                            hid_devices,
                            membersof(hid_devices));
    usb_host_class_hid_start(&hid);

    usb_host_class_mass_storage_init(&mass_storage,
                                     &usb,
                                     mass_storage_devices,
                                     membersof(mass_storage_devices));
    usb_host_class_mass_storage_start(&mass_storage);

    /* Start the USB driver. */
    usb_host_start(&usb);

    thrd_spawn(usb_control_main,
               &usb,
               0,
               usb_control_stack,
               sizeof(usb_control_stack));
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
