/**
 * @file uart_port.i
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

#include <avr/interrupt.h>

/* Register access macros. */
#define UCSRnA(dev_p) ((dev_p)->sfr_p + 0)
#define UCSRnB(dev_p) ((dev_p)->sfr_p + 1)
#define UCSRnC(dev_p) ((dev_p)->sfr_p + 2)
#define UBRRn(dev_p) ((volatile uint16_t *)((dev_p)->sfr_p + 4))
#define UDRn(dev_p) ((dev_p)->sfr_p + 6)

static struct fs_counter_t rx_channel_overflow;
static struct fs_counter_t rx_errors;

static int uart_port_module_init()
{
    fs_counter_init(&rx_channel_overflow,
                    FSTR("/drivers/uart/rx_channel_overflow"),
                    0);
    fs_counter_register(&rx_channel_overflow);
    
    fs_counter_init(&rx_errors,
                    FSTR("/drivers/uart/rx_errors"),
                    0);
    fs_counter_register(&rx_errors);
    
    return (0);
}

static int uart_port_start(struct uart_driver_t *self_p)
{
    uint16_t baudrate = (F_CPU / 16 / self_p->baudrate - 1);
    struct uart_device_t *dev_p = self_p->dev_p;

    *UBRRn(dev_p) = baudrate;
    *UCSRnA(dev_p) = 0;
    *UCSRnB(dev_p) = (_BV(RXCIE0) | _BV(TXCIE0) | _BV(RXEN0) | _BV(TXEN0));
    *UCSRnC(dev_p) = (_BV(UCSZ00) | _BV(UCSZ01));

    dev_p->drv_p = self_p;

    return (0);
}

static int uart_port_stop(struct uart_driver_t *self_p)
{
    *UCSRnA(self_p->dev_p) = 0;
    *UCSRnB(self_p->dev_p) = 0;
    *UCSRnC(self_p->dev_p) = 0;

    self_p->dev_p->drv_p = NULL;

    return (0);
}

static ssize_t uart_port_write_cb(void *arg_p,
                                  const void *txbuf_p,
                                  size_t size)
{
    struct uart_driver_t *self_p;

    self_p = container_of(arg_p, struct uart_driver_t, chout);

    sem_get(&self_p->sem, NULL);

    /* Initiate transfer by writing the first byte. */
    self_p->txbuf_p = (txbuf_p + 1);
    self_p->txsize = (size - 1);
    self_p->thrd_p = thrd_self();

    sys_lock();

    /* Write the first byte. The rest are written from the interrupt
       routine. */
    *UDRn(self_p->dev_p) = self_p->txbuf_p[-1];

    thrd_suspend_isr(NULL);

    sys_unlock();

    sem_put(&self_p->sem, 1);

    return (size);
}

static void tx_isr(int index)
{
    struct uart_driver_t *drv_p = uart_device[index].drv_p;

    if (drv_p == NULL) {
        return;
    }

    /* Write the next byte or resume suspended thread if all data have
       been written. */
    if (drv_p->txsize > 0) {
        *UDRn(drv_p->dev_p) = *drv_p->txbuf_p++;
        drv_p->txsize--;
    } else {
        thrd_resume_isr(drv_p->thrd_p, 0);
    }
}

static void rx_isr(int index)
{
    struct uart_driver_t *drv_p = uart_device[index].drv_p;
    char c;
    uint8_t error;

    if (drv_p == NULL) {
        return;
    }

    error = (*UCSRnA(drv_p->dev_p) & (_BV(FE0) | _BV(DOR0) | _BV(UPE0)));
    c = *UDRn(drv_p->dev_p);

    /* Error frames are discarded. */
    if (error == 0) {
        /* Write data to input queue. */
        if (queue_write_isr(&drv_p->chin, &c, 1) != 1) {
            fs_counter_increment(&rx_channel_overflow, 1);
        }
    } else {
        fs_counter_increment(&rx_errors, 1);
    }
}

static void udre_isr(int index)
{
}

#define UART_ISR(vector, index)                 \
    ISR(vector ## _RX_vect) {                   \
        rx_isr(index);                          \
    }                                           \
                                                \
    ISR(vector ## _TX_vect) {                   \
        tx_isr(index);                          \
    }                                           \
                                                \
    ISR(vector ## _UDRE_vect) {                 \
        udre_isr(index);                        \
    }

#if (UART_DEVICE_MAX >= 1)
UART_ISR(USART0, 0)
#endif

#if (UART_DEVICE_MAX >= 2)
UART_ISR(USART1, 1)
#endif

#if (UART_DEVICE_MAX >= 3)
UART_ISR(USART2, 2)
#endif

#if (UART_DEVICE_MAX >= 4)
UART_ISR(USART3, 3)
#endif
