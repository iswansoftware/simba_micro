/**
 * @file dac.c
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

#include "dac_port.i"

int dac_module_init(void)
{
    return (dac_port_module_init());
}

int dac_init(struct dac_driver_t *self_p,
             struct dac_device_t *dev_p,
             struct pin_device_t *pin0_dev_p,
             struct pin_device_t *pin1_dev_p,
             int sampling_rate)
{
    return (dac_port_init(self_p,
                          dev_p,
                          pin0_dev_p,
                          pin1_dev_p,
                          sampling_rate));
}

int dac_async_convert(struct dac_driver_t *self_p,
                      uint32_t *samples_p,
                      size_t length)
{
    return (dac_port_async_convert(self_p, samples_p, length));
}

int dac_async_wait(struct dac_driver_t *self_p)
{
    return (dac_port_async_wait(self_p));
}

int dac_convert(struct dac_driver_t *self_p,
                uint32_t *samples_p,
                size_t length)
{
    return (dac_port_convert(self_p, samples_p, length));
}
