/**
 * @file kernel/heap.h
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
 * MERHEAPTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * This file is part of the Simba project.
 */

#ifndef __KERNEL_HEAP_H__
#define __KERNEL_HEAP_H__

#include "simba.h"

#define HEAP_FIXED_SIZES_MAX 8

struct heap_fixed_t {
    void *free_p;
    size_t size;
};

struct heap_dynamic_t {
    void *free_p;
};

/* Heap. */
struct heap_t {
    void *buf_p;
    size_t size;
    void *next_p;
    struct heap_fixed_t fixed[HEAP_FIXED_SIZES_MAX];
    struct heap_dynamic_t dynamic;
};

/**
 * Initialize given heap.
 *
 * @param[in] self_p Heap to initialize.
 * @param[in] buf_p Buffer.
 * @param[in] size Size of buffer.
 *
 * @return zero(0) or negative error code
 */
int heap_init(struct heap_t *self_p,
              void *buf_p,
              size_t size,
              size_t sizes[HEAP_FIXED_SIZES_MAX]);

/**
 * Allocate a buffer from given heap.
 *
 * @param[in] self_p Heap.
 * @param[in] size Size to allocate.
 *
 * @return Allocated buffer.
 */
void *heap_alloc(struct heap_t *self_p,
                 size_t size);

/**
 * Free given buffer.
 *
 * @param[in] self_p Heap.
 * @param[in] buf_p Buffer to free.
 *
 * @return zero(0) or negative error code.
 */
int heap_free(struct heap_t *self_p,
              void *buf_p);

/**
 * Share given buffer.
 *
 * @param[in] self_p Heap.
 * @param[in] buf_p Buffer to share.
 * @param[in] count Share count.
 *
 * @return zero(0) or negative error code.
 */
int heap_share(struct heap_t *self_p,
               const void *buf_p,
               int count);

#endif
