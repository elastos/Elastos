/*
 * Copyright (c) 2019 Elastos Foundation
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

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <openssl/opensslv.h>
#include <cjson/cJSON.h>
#include <time.h>

#include "common.h"
#include "did.h"

#define DID_MAX_LEN      512

const char *get_time_string(char *timestring, size_t len, time_t *p_time)
{
    time_t t;
    struct tm tm;

    if (len < DOC_BUFFER_LEN || !p_time)
        return NULL;

    if (*p_time == 0)
        time(&t);
    else
        t = *p_time;

    gmtime_r(&t, &tm);
    strftime(timestring, 80, "%Y-%m-%dT%H:%M:%SZ", &tm);

    return timestring;
}

int parse_time(time_t *time, const char *string)
{
    struct tm tm;

    if (!time || !string)
        return -1;

    memset(&tm, 0, sizeof(tm));

    if (!strptime(string, "%Y-%m-%dT%H:%M:%SZ", &tm))
        return -1;

    *time = timegm(&tm);
    return 0;
}