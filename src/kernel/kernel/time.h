/**
 * @file kernel/time.h
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

#ifndef __KERNEL_TIME_H__
#define __KERNEL_TIME_H__

#include "simba.h"

/**
 * A time in seconds and nanoseconds. ``seconds`` and ``nanoseconds``
 * shall be added to get the time.
 */
struct time_t {
    /** Number of seconds. */
    int32_t seconds;
    /** Number of nanoseconds. */
    int32_t nanoseconds;
};

/**
 * A date in year, month, date, day, hour, minute and seconds.
 */
struct date_t {
    /** Second [0..59]. */
    int second;
    /** Minute [0..59]. */
    int minute;
    /** Hour [0..23]. */
    int hour;
    /** Weekday [1..7], where 1 is Monday and 7 is Sunday. */
    int day;
    /** Day in month [1..31] */
    int date;
    /** Month [1..12] where 1 is January and 12 is December. */
    int month;
    /** Year [1970..]. */
    int year;
};

/**
 * Get current time in seconds and nanoseconds. The resolution of the
 * time is implementation specific and may vary a lot between
 * different architectures.
 *
 * @param[out] now_p Read current time.
 *
 * @return zero(0) or negative error code.
 */
int time_get(struct time_t *now_p);

/**
 * Set current time in seconds and nanoseconds.
 *
 * @param[in] new_p New current time.
 *
 * @return zero(0) or negative error code.
 */
int time_set(struct time_t *new_p);

/**
 * Subtract given times.
 *
 * @param[out] diff_p The result of the subtrancting ``left_p`` from
 *                    ``right_p``.
 * @param[in] left_p The operand to subtract from.
 * @param[in] right_p The operand to subtract.
 *
 * @return zero(0) or negative error code.
 */
int time_diff(struct time_t *diff_p,
              struct time_t *left_p,
              struct time_t *right_p);

/**
 * Sleep (busy wait) for given number of microseconds.
 *
 * @param[in] usec Microseconds to sleep.
 *
 * @return void
 */
void time_sleep(long usec);

/**
 * Convert given unix time to a date.
 *
 * @param[out] date_p Converted time.
 * @param[in] time_p Unix time to convert.
 *
 * @return zero(0) or negative error code.
 */
int time_unix_time_to_date(struct date_t *date_p,
                           struct time_t *time_p);

#endif
