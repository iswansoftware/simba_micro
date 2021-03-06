/**
 * @file adc.c
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

#include "adc_port.i"

int adc_module_init(void)
{
    return (adc_port_module_init());
}

int adc_init(struct adc_driver_t *self_p,
             struct adc_device_t *dev_p,
             struct pin_device_t *pin_dev_p,
             int reference,
             int sampling_rate)
{
    return (adc_port_init(self_p,
                          dev_p,
                          pin_dev_p,
                          reference,
                          sampling_rate));
}

int adc_async_convert(struct adc_driver_t *self_p,
                      uint16_t *samples_p,
                      size_t length)
{
    return (adc_port_async_convert(self_p, samples_p, length));
}

int adc_async_wait(struct adc_driver_t *self_p)
{
    return (adc_port_async_wait(self_p));
}

int adc_convert(struct adc_driver_t *self_p,
                uint16_t *samples_p,
                size_t length)
{
    adc_port_async_convert(self_p, samples_p, length);
    adc_port_async_wait(self_p);
    
    return (0);
}
