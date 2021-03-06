/**
 * @file sem.c
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

struct sem_elem_t {
    struct sem_elem_t *next_p;
    struct sem_elem_t *prev_p;
    struct thrd_t *thrd_p;
};

int sem_module_init(void)
{
    return (0);
}

int sem_init(struct sem_t *self_p,
             int count)
{
    self_p->count = count;
    self_p->head_p = NULL;

    return (0);
}

int sem_get(struct sem_t *self_p,
            struct time_t *timeout_p)
{
    int err = 0;
    struct sem_elem_t elem;

    sys_lock();

    if (self_p->count == 0) {
        elem.thrd_p = thrd_self();
        elem.next_p = self_p->head_p;
        elem.prev_p = NULL;
        self_p->head_p = &elem;
        err = thrd_suspend_isr(timeout_p);

        if (err == -ETIMEDOUT) {
            if (elem.prev_p != NULL) {
                elem.prev_p->next_p = elem.next_p;
            } else {
                self_p->head_p = elem.next_p;
            }

            if (elem.next_p != NULL) {
                elem.next_p->prev_p = elem.prev_p;
            }
        }
    } else {
        self_p->count--;
    }

    sys_unlock();

    return (err);
}

int sem_put(struct sem_t *self_p,
            int count)
{
    sys_lock();
    sem_put_isr(self_p, count);
    sys_unlock();

    return (0);
}

int sem_put_isr(struct sem_t *self_p,
                int count)
{
    struct sem_elem_t *elem_p;

    self_p->count += count;

    while ((self_p->count > 0) && (self_p->head_p != NULL)) {
        self_p->count--;
        elem_p = self_p->head_p;
        self_p->head_p = elem_p->next_p;

        if (elem_p->next_p != NULL) {
            elem_p->next_p->prev_p = NULL;
        }

        thrd_resume_isr(elem_p->thrd_p, 0);
    }

    return (0);
}
