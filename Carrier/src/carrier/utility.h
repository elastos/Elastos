/*
 * Copyright (c) 2020 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __CARRIER_UTILTIY_H__
#define __CARRIER_UTILTIY_H__

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "ela_carrier.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline ElaConnectionStatus connection_status(bool connected)
{
    return connected ? ElaConnectionStatus_Connected : ElaConnectionStatus_Disconnected;
}

static inline void gettimeofday_elapsed(struct timeval *tm, int elapsed)
{
    struct timeval interval;

    interval.tv_sec  = elapsed;
    interval.tv_usec = 0;

    gettimeofday(tm, NULL);
    timeradd(tm, &interval, tm);
}

#ifdef __cplusplus
}
#endif

#endif /* __CARRIER_UTILTIY_H__ */
