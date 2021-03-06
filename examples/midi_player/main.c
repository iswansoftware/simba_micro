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

static struct uart_driver_t uart_midi;
static uint8_t uart_midi_in[8];
static struct sem_t sem;

/* Player operation codes. */
#define COMMAND  0
#define DELAY    1
#define SPACING  2
#define PACE     3

struct operation_t {
    int code;
    size_t size;
    uint8_t buf[4];
};

//#include "murderers.h"
#include "christmas.h"

static size_t write(void *buf_p, size_t size)
{
    sem_get(&sem, NULL);
    uart_write(&uart_midi, buf_p, size);
    sem_put(&sem, 1);

    return (size);
}

static int set_instrument(int channel, int instrument)
{
    uint8_t message[2];

    message[0] = (MIDI_SET_INTRUMENT | channel);
    message[1] = instrument;

    return (write(message,
                  sizeof(message)) != sizeof(message));
}

/**
 * A thread for passing MIDI messages from the rx to tx, so called
 * MIDI pass through. The messages received on rx must be multiplexed
 * with the messages created by the player thread. All read data is
 * parsed because we must keep track of whole messages. If all read
 * byte would be immediatly written to the uart, a player message
 * might be sent in the middle of a MIDI through message, and make it
 * corrupt.
 *
 * Just NOTE ON, NOTE OFF and SET INSTRUMENT messages are handled in
 * this implementaion.
 */
static THRD_STACK(midi_through_stack, 256);

static void *midi_through_main(void *arg_p)
{
    uint8_t buf[4], previous_status = 0;
    uint8_t status;
    int command;

    while (1) {
        /* Wait for a MIDI command on the serial port. */
        uart_read(&uart_midi, &buf[1], 1);

        /* Use the previous status if a data byte is read. */
        if ((buf[1] & 0x80) != 0) {
            status = buf[1];
        } else {
            status = previous_status;
        }

        /* Extract the command and the channel from the status. */
        command = (status & 0xf0);

        switch (command) {

        case MIDI_NOTE_OFF:
            uart_read(&uart_midi, &buf[1], 2);
            write(buf, 3);
            break;

        case MIDI_NOTE_ON:
            if (buf[1] & 0x80) {
                uart_read(&uart_midi, &buf[2], 2);
                write(&buf[1], 3);
            } else {
                buf[0] = status;
                uart_read(&uart_midi, &buf[2], 1);
                write(buf, 3);
            }

            previous_status = status;
            break;

        case MIDI_SET_INTRUMENT:
            uart_read(&uart_midi, &buf[1], 1);
            write(buf, 2);
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

    uart_init(&uart_midi,
              &uart_device[0],
              MIDI_BAUDRATE,
              uart_midi_in,
              sizeof(uart_midi_in));
    uart_start(&uart_midi);

    sem_init(&sem, 1);

    thrd_spawn(midi_through_main,
               NULL,
               10,
               midi_through_stack,
               sizeof(midi_through_stack));
}

int main()
{
    int i, j;
    struct operation_t operation;
    unsigned long pace = 200;

    init();

    while (1) {
        for (i = 0; i < 128; i++) {
            set_instrument(0, i);

            for (j = 0; j < membersof(song); j++) {
                operation = song[j];

                switch (operation.code) {

                case COMMAND:
                    write(operation.buf, operation.size);
                    break;

                case DELAY:
                    thrd_usleep(pace * operation.size);
                    break;

                case SPACING:
                    thrd_usleep(5000);
                    break;

                case PACE:
                    pace = 1000UL * operation.size;
                    break;

                default:
                    std_printf(FSTR("bad op code %d\r\n"),
                               operation.code);
                    break;
                }
            }
        }
    }

    return (0);
}
