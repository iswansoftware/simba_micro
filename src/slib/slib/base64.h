/**
 * @file slib/base64.h
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

#ifndef __SLIB_BASE64_H__
#define __SLIB_BASE64_H__

#include "simba.h"

/**
 * Encode given buffer.
 *
 * @param[out] dst_p Encoded output data.
 * @param[in] src_p Input data.
 * @param[in] size Number of bytes in the input data.
 *
 * @return zero(0) or negative error code.
 */
int base64_encode(char *dst_p, const void *src_p, size_t size);

/**
 * Decode given base64 encoded buffer.
 *
 * @param[out] dst_p Output data.
 * @param[in] src_p Encoded input data.
 * @param[in] size Number of bytes in the encoded input data.
 *
 * @return zero(0) or negative error code.
 */
int base64_decode(void *dst_p, const char *src_p, size_t size);

#endif
