/**
 * @file drivers/flash.h
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

/* Flash commands. */
#define SAM_FLASH_GETD  0x00  /* Get Flash Descriptor */
#define SAM_FLASH_WP    0x01  /* Write page */
#define SAM_FLASH_WPL   0x02  /* Write page and lock */
#define SAM_FLASH_EWP   0x03  /* Erase page and write page */
#define SAM_FLASH_EWPL  0x04  /* Erase page and write page then lock */
#define SAM_FLASH_EA    0x05  /* Erase all */
#define SAM_FLASH_SLB   0x08  /* Set Lock Bit */
#define SAM_FLASH_CLB   0x09  /* Clear Lock Bit */
#define SAM_FLASH_GLB   0x0A  /* Get Lock Bit */
#define SAM_FLASH_SGPB  0x0B  /* Set GPNVM Bit */
#define SAM_FLASH_CGPB  0x0C  /* Clear GPNVM Bit */
#define SAM_FLASH_GGPB  0x0D  /* Get GPNVM Bit */
#define SAM_FLASH_STUI  0x0E  /* Start Read Unique Identifier */
#define SAM_FLASH_SPUI  0x0F  /* Stop Read Unique Identifier */
#define SAM_FLASH_GCALB 0x10  /* Get CALIB Bit */

#define PAGE_BUFFER_SIZE_32 (FLASH0_PAGE_SIZE / sizeof(uint32_t))

static uint32_t page_buffer[PAGE_BUFFER_SIZE_32];

typedef uint32_t (*iap_fn_t)(uint32_t index, uint32_t command);

/**
 * Get bank, page and offset from given address.
 *
 * @return zero(0) or negative error code.
 */
static int address_to_bank(size_t address,
                           struct flash_device_t *dev_p,
                           struct flash_device_bank_t **bank_pp,
                           size_t *page_p,
                           size_t *offset_p)
{
    int bank;
    struct flash_device_bank_t *bank_p;

    for (bank = 0; bank < membersof(dev_p->banks); bank++) {
        bank_p = &dev_p->banks[bank];

        if ((address >= bank_p->begin) && (address < bank_p->end)) {
            *bank_pp = bank_p;
            *page_p = ((address - bank_p->begin) / bank_p->page_size);
            *offset_p = ((address - bank_p->begin) % bank_p->page_size);

            return (0);
        }
    }

    return (-1);
}

/**
 * Execute given command with argument.
 *
 * @return zero(0) or negative error code.
 */
static ssize_t bank_execute_command(struct flash_device_bank_t *bank_p,
                                    uint8_t command,
                                    uint16_t argument)
{
    iap_fn_t iap_fn;
    uint32_t status = 0;

    iap_fn = *((iap_fn_t *)(FLASH_IAP_ADDRESS));

    sys_lock();
    
    status = iap_fn(bank_p->index,
                    (EEFC_FCR_FKEY(0x5a)
                     | EEFC_FCR_FARG(argument)
                     | EEFC_FCR_FCMD(command)));

    sys_unlock();
    
    return ((status & (EEFC_FSR_FLOCKE | EEFC_FSR_FCMDE)) == 0 ? 0 : -1);
}

/**
 * Write data to given bank at page and offset. The data must not span
 * multiple pages.
 *
 * @return zero(0) or negative error code.
 */
static ssize_t bank_page_write(struct flash_device_bank_t *bank_p,
                               size_t page,
                               size_t offset,
                               const void *src_p,
                               size_t size)
{
    size_t address;
    uint32_t *dst_p;
    ssize_t res;
    size_t i;

    address = (bank_p->begin + page * bank_p->page_size);

    /* Unlock the page. */
    res = bank_execute_command(bank_p, SAM_FLASH_CLB, page);

    if (res != 0) {
        return (res);
    }

    /* Copy the whole page to a local buffer. */
    memcpy(page_buffer, (void *)address, bank_p->page_size);

    /* Overwrite part of the buffer. */
    memcpy(&((char *)page_buffer)[offset], src_p, size);

    /* Write the buffer to the flash temporary buffer. */
    dst_p = (uint32_t *)address;

    for (i = 0; i < bank_p->page_size / sizeof(uint32_t); i++) {
        *dst_p++ = page_buffer[i];
    }


    /* Erase the page and write from the temporary page buffer to the
       flash memory. */
    bank_p->regs_p->FMR = EEFC_FMR_FWS(6);
    res = bank_execute_command(bank_p, SAM_FLASH_EWP, page);
    bank_p->regs_p->FMR = EEFC_FMR_FWS(4);

    if (res != 0) {
        return (res);
    }

    /* Lock the page again. */
    res = bank_execute_command(bank_p, SAM_FLASH_SLB, page);

    if (res != 0) {
        return (res);
    }

    return (0);
}

int flash_port_module_init(void)
{
    return (0);
}

ssize_t flash_port_read(struct flash_driver_t *self_p,
                        void *dst_p,
                        size_t src,
                        size_t size)
{
    struct flash_device_t *dev_p = self_p->dev_p;

    sem_get(&dev_p->sem, NULL);
    memcpy(dst_p, (void *)src, size);
    sem_put(&dev_p->sem, 1);

    return (size);
}

ssize_t flash_port_write(struct flash_driver_t *self_p,
                         size_t dst,
                         const void *src_p,
                         size_t size)
{
    ssize_t res = size;
    struct flash_device_t *dev_p;
    size_t n, left, page, offset;
    struct flash_device_bank_t *bank_p;
    const uint8_t *u8src_p = src_p;

    dev_p = self_p->dev_p;
    left = size;

    sem_get(&dev_p->sem, NULL);

    /* Write one page at a time. */
    while (left > 0) {
        if (address_to_bank(dst, dev_p, &bank_p, &page, &offset) != 0) {
            res = -1;
            break;
        }

        n = MIN(left, bank_p->page_size - offset);

        if (bank_page_write(bank_p, page, offset, u8src_p, n) != 0) {
            res = -1;
            break;
        }

        left -= n;
        dst += n;
        u8src_p += n;
    }

    sem_put(&dev_p->sem, 1);

    return (res);
}
