/**
 * @file arpa/inet.h
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

#ifndef __ARPA_INET_H__
#define __ARPA_INET_H__

static inline uint32_t htonl(uint32_t v)
{
    return (((v) << 24)
            | (((v) & 0x0000ff00) << 8)
            | (((v) & 0x00ff0000) >> 8)
            | (((v) & 0xff000000) >> 24));
}

#define ntohl(v) htonl(v)

static inline uint16_t htons(uint16_t v)
{
    return (((v) << 8) | (((v) & 0xff00) >> 8));
}

#define ntohs(v) htons(v)

#endif
