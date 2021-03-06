/**
 * @file drivers/linux/spi_port.i
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

/**
 * SPI TX complete interrupt.
 */
ISR(spi0)
{
    struct spi_device_t *dev_p = &spi_device[0];
    struct spi_driver_t *drv_p = dev_p->drv_p;
    uint32_t sr;

    if (drv_p == NULL) {
        return;
    }

    sr = dev_p->regs_p->SR;
    sr &= dev_p->regs_p->IMR;

    if (sr == 0) {
        return;
    }

    /* Read incoming byte. */
    if (drv_p->rxbuf_p != NULL) {
        *drv_p->rxbuf_p++ = drv_p->dev_p->regs_p->RDR;
    }

    /* Resume on complete transfer. */
    if (drv_p->size == 0) {
        /* Disable tx interrupt. */
        dev_p->regs_p->IDR = (SPI_IDR_TXEMPTY);
        thrd_resume_isr(drv_p->thrd_p, 0);
    } else {
        /* Write next byte. */
        if (drv_p->txbuf_p != NULL) {
            drv_p->dev_p->regs_p->TDR = SPI_TDR_TD(*drv_p->txbuf_p++);
        } else {
            drv_p->dev_p->regs_p->TDR = SPI_TDR_TD(0xff);
        }
        
        drv_p->size--;
    }
}

static int spi_port_module_init(void)
{
    return (0);
}

static int spi_port_init(struct spi_driver_t *self_p,
                         struct spi_device_t *dev_p,
                         struct pin_device_t *ss_pin_p,
                         int mode,
                         int speed,
                         int cpol,
                         int cpha)
{
    if (mode == SPI_MODE_MASTER) {
        pin_init(&self_p->ss, ss_pin_p, PIN_OUTPUT);
        pin_write(&self_p->ss, 1);
    } else {
        pin_init(&self_p->ss, ss_pin_p, PIN_INPUT);
    }

    return (0);
}

static int spi_port_start(struct spi_driver_t *self_p)
{
    uint32_t mask;
    struct spi_device_t *dev_p = self_p->dev_p;
    volatile struct sam_pio_t *pio_p;

    /* Configure miso pin. */
    mask = dev_p->miso_p->mask;
    pio_p = dev_p->miso_p->pio_p;
    pio_p->PDR = mask;
    pio_p->ABSR &= ~mask;

    /* Configure mosi pin. */
    mask = dev_p->mosi_p->mask;
    pio_p = dev_p->mosi_p->pio_p;
    pio_p->PDR = mask;
    pio_p->ABSR &= ~mask;

    /* Configure sck pin. */
    mask = dev_p->sck_p->mask;
    pio_p = dev_p->sck_p->pio_p;
    pio_p->PDR = mask;
    pio_p->ABSR &= ~mask;

    pmc_peripheral_clock_enable(dev_p->id);
    nvic_enable_interrupt(dev_p->id);

    dev_p->regs_p->CSR[0] = (SPI_CSR_SCBR(self_p->speed)
                             | (SPI_CSR_NCPHA * self_p->cpha)
                             | (SPI_CSR_CPOL * self_p->cpol));

    /* Set mode. */
    if (self_p->mode == SPI_MODE_MASTER) {
        dev_p->regs_p->MR = (SPI_MR_MODFDIS | SPI_MR_MSTR);
    } else {
        dev_p->regs_p->MR &= ~SPI_MR_MSTR;
    }

    /* Enable the peripheral. */
    dev_p->regs_p->CR = SPI_CR_SPIEN;

    return (0);
}

static int spi_port_stop(struct spi_driver_t *self_p)
{
    struct spi_device_t *dev_p = self_p->dev_p;

    dev_p->regs_p->CR = SPI_CR_SPIDIS;

    return (0);
}

static ssize_t spi_port_transfer(struct spi_driver_t *self_p,
                                 void *rxbuf_p,
                                 const void *txbuf_p,
                                 size_t n)
{
    struct spi_device_t *dev_p = self_p->dev_p;

    self_p->rxbuf_p = rxbuf_p;
    self_p->txbuf_p = txbuf_p;
    self_p->size = (n - 1);
    self_p->thrd_p = thrd_self();

    sys_lock();

    /* Write first byte. The rest are written from isr. */
    if (self_p->txbuf_p != NULL) {
        dev_p->regs_p->TDR = SPI_TDR_TD(*self_p->txbuf_p++);
    } else {
        dev_p->regs_p->TDR = SPI_TDR_TD(0xff);
    }

    /* Enable tx interrupt. */
    dev_p->regs_p->IER = (SPI_IER_TXEMPTY);

    thrd_suspend_isr(NULL);

    sys_unlock();

    return (n);
}
