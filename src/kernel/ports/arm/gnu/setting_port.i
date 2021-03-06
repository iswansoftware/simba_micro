/**
 * @file arm/gnu/setting_port.i
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

#define PRIMARY 0
#define SECONDARY  1

extern uint8_t setting_area[2][SETTING_AREA_SIZE];
static struct flash_driver_t drv;

static uint32_t calculate_area_crc(const uint8_t *area_p)
{
    uint32_t crc;
    uint8_t buf[256];
    size_t size;
    int i;

    /* Calculate the crc. */
    crc = 0;

    for (i = 0; i < SETTING_AREA_SIZE / sizeof(buf); i++) {
        flash_read(&drv,
                   buf,
                   (size_t)&area_p[i * sizeof(buf)],
                   sizeof(buf));

        size = sizeof(buf);

        /* Don't include the crc at the end of the area. */
        if (i == (SETTING_AREA_SIZE / sizeof(buf)) - 1) {
            size -= 4;
        }

        crc = crc_32(crc, buf, size);
    }

    return (crc);
}

/**
 * Check if the area is ok. That means, the crc of the content matches
 * the crc at the end of the area.
 *
 * return true(1) if the area is ok, otherwise false(0)
 */
static int is_area_ok(const uint8_t *area_p)
{
    uint32_t expected_crc, real_crc;

    /* Read the expected crc.*/
    flash_read(&drv,
               &expected_crc,
               (size_t)&area_p[SETTING_AREA_CRC_OFFSET],
               sizeof(expected_crc));

    real_crc = calculate_area_crc(area_p);

    return (expected_crc == real_crc);
}

static int copy_area(uint8_t *dst_p, const uint8_t *src_p)
{
    uint8_t buf[256];
    int i;

    for (i = 0; i < SETTING_AREA_SIZE / sizeof(buf); i++) {
        if (flash_read(&drv,
                       buf,
                       (size_t)&src_p[i * sizeof(buf)],
                       sizeof(buf)) != sizeof(buf)) {
            return (-1);
        }

        if (flash_write(&drv,
                        (size_t)&dst_p[i * sizeof(buf)],
                        buf,
                        sizeof(buf)) != sizeof(buf)) {
            return (-1);
        }
    }

    return (0);
}

static int setting_port_module_init(void)
{
    flash_init(&drv, &flash_0_dev);

    /* Is the primary area ok? */
    if (!is_area_ok(&setting_area[PRIMARY][0])) {
        /* Is the backup area ok? */
        if (!is_area_ok(&setting_area[SECONDARY][0])) {
            return (-1);
        }

        if (copy_area(&setting_area[PRIMARY][0], &setting_area[SECONDARY][0]) != 0) {
            return (-1);
        }
    }

    return (0);
}

static ssize_t setting_port_read(void *dst_p, size_t src, size_t size)
{
    return (flash_read(&drv, dst_p, (size_t)&setting_area[PRIMARY][src], size));
}

static ssize_t setting_port_write(size_t dst, const void *src_p, size_t size)
{
    uint32_t crc;

    /* Copy the primary area to the secondary area. */
    if (copy_area(&setting_area[SECONDARY][0], &setting_area[PRIMARY][0]) != 0) {
        return (-1);
    }

    /* Update the primary area. */
    if (flash_write(&drv, (size_t)&setting_area[PRIMARY][dst], src_p, size) != size) {
        return (-1);
    }

    /* Update the crc. */
    crc = calculate_area_crc(&setting_area[PRIMARY][0]);

    if (flash_write(&drv,
                    (size_t)&setting_area[PRIMARY][SETTING_AREA_CRC_OFFSET],
                    &crc,
                    sizeof(crc)) != sizeof(crc)) {
        return (-1);
    }

    return (size);
}
