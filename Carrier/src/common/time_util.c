/*
 * Copyright (c) 2018 Elastos Foundation
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

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#if defined (WIN32) || defined(WIN64)
#include <windows.h>
#include <limits.h>
#elif defined(__APPLE__)
#include <mach/mach_time.h>
#endif

#include "time_util.h"

#if defined (WIN32) || defined(WIN64)
uint64_t get_monotonic_time(void)
{
    uint64_t ticks;
    uint32_t ticks32;
    uint32_t ticks_as_32bit;

    /* There are four sources for the monotonic time on Windows:
     *
     * Three are based on a (1 msec accuracy, but only read periodically) clock chip:
     * - GetTickCount (GTC)
     *    32bit msec counter, updated each ~15msec, wraps in ~50 days
     * - GetTickCount64 (GTC64)
     *    Same as GetTickCount, but extended to 64bit, so no wrap
     *    Only available in Vista or later
     * - timeGetTime (TGT)
     *    similar to GetTickCount by default: 15msec, 50 day wrap.
     *    available in winmm.dll (thus known as the multimedia timers)
     *    However apps can raise the system timer clock frequency using timeBeginPeriod()
     *    increasing the accuracy up to 1 msec, at a cost in general system performance
     *    and battery use.
     *
     * One is based on high precision clocks:
     * - QueryPrecisionCounter (QPC)
     *    This has much higher accuracy, but is not guaranteed monotonic, and
     *    has lots of complications like clock jumps and different times on different
     *    CPUs. It also has lower long term accuracy (i.e. it will drift compared to
     *    the low precision clocks.
     *
     * Additionally, the precision available in the timer-based wakeup such as
     * MsgWaitForMultipleObjectsEx (which is what the mainloop is based on) is based
     * on the TGT resolution, so by default it is ~15msec, but can be increased by apps.
     *
     * The QPC timer has too many issues to be used as is. The only way it could be used
     * is to use it to interpolate the lower precision clocks. Firefox does something like
     * this:
     *   https://bugzilla.mozilla.org/show_bug.cgi?id=363258
     *
     * However this seems quite complicated, so we're not doing this right now.
     *
     * The approach we take instead is to use the TGT timer, extending it to 64bit
     * either by using the GTC64 value, or if that is not available, a process local
     * time epoch that we increment when we detect a timer wrap (assumes that we read
     * the time at least once every 50 days).
     *
     * This means that:
     *  - We have a globally consistent monotonic clock on Vista and later
     *  - We have a locally monotonic clock on XP
     *  - Apps that need higher precision in timeouts and clock reads can call
     *    timeBeginPeriod() to increase it as much as they want
     */

    ticks = GetTickCount64();
    ticks32 = timeGetTime();

    /* GTC64 and TGT are sampled at different times, however they
     * have the same base and source (msecs since system boot).
     * They can differ by as much as -16 to +16 msecs.
     * We can't just inject the low bits into the 64bit counter
     * as one of the counters can have wrapped in 32bit space and
     * the other not. Instead we calculate the signed difference
     * in 32bit space and apply that difference to the 64bit counter.
     */
    ticks_as_32bit = (uint32_t)ticks;

    /* We could do some 2's complement hack, but we play it safe */
    if (ticks32 - ticks_as_32bit <= INT32_MAX)
        ticks += ticks32 - ticks_as_32bit;
    else
        ticks -= ticks_as_32bit - ticks32;

    return ticks * 1000;
}
#elif defined(__APPLE__) /* Mac OS */
uint64_t get_monotonic_time(void)
{
    static mach_timebase_info_data_t timebase_info;

    if (timebase_info.denom == 0) {
        /* This is a fraction that we must use to scale
         * mach_absolute_time() by in order to reach nanoseconds.
         *
         * We've only ever observed this to be 1/1, but maybe it could be
         * 1000/1 if mach time is microseconds already, or 1/1000 if
         * picoseconds.  Try to deal nicely with that.
         */
        mach_timebase_info (&timebase_info);

        /* We actually want microseconds... */
        if (timebase_info.numer % 1000 == 0)
            timebase_info.numer /= 1000;
        else
            timebase_info.denom *= 1000;

        /* We want to make the numer 1 to avoid having to multiply... */
        if (timebase_info.denom % timebase_info.numer == 0) {
            timebase_info.denom /= timebase_info.numer;
            timebase_info.numer = 1;
        } else {
            /* We could just multiply by timebase_info.numer below, but why
             * bother for a case that may never actually exist...
             *
             * Plus -- performing the multiplication would risk integer
             * overflow.  If we ever actually end up in this situation, we
             * should more carefully evaluate the correct course of action.
             */
            mach_timebase_info (&timebase_info); /* Get a fresh copy for a better message */
            assert(0 && "get_monotonic_time() error: Got weird mach timebase info.");
        }
    }

    return mach_absolute_time () / timebase_info.denom;
}
#else
uint64_t get_monotonic_time(void)
{
    struct timespec ts;
    int result;

    result = clock_gettime(CLOCK_MONOTONIC, &ts);

    if (result != 0) {
        assert(0 && "get_monotonic_time() requires working CLOCK_MONOTONIC");
        return 0;
    }
    
    return (((uint64_t) ts.tv_sec) * 1000000) + (ts.tv_nsec / 1000);
}
#endif

