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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <openssl/opensslv.h>
#include <cjson/cJSON.h>

#include "common.h"
#include "did.h"

#define DID_MAX_LEN      512
#define DOC_BUFFER_LEN   512

const char *get_time_string(time_t *p_time)
{
    static char time_string[DOC_BUFFER_LEN];
    time_t *temp_time;
    struct tm *ptm;

    if (!p_time)
        return NULL;

    temp_time = p_time;

    if (*temp_time == 0)
        time(temp_time);

    ptm = gmtime(temp_time);
    strftime(time_string, 80, "%FT%H:00:00Z", ptm);
    return time_string;
}


int parse_time(time_t *time, const char *string)
{
    char *ptr;
    struct tm ptm;
    size_t len;
    char string_copy[DID_MAX_LEN];

    if (!time || !string)
        return -1;

    memset(&ptm, 0, sizeof(ptm));

    len = strlen(string);
    if (string[len - 1] != 'Z')
        return -1;

    strcpy(string_copy, string);
    string_copy[len-1] = '\0';      //remove the last 'Z'

    ptr = strrchr(string_copy, ':');
    if (!ptr)
        return -1;
    ptm.tm_sec = atoi(ptr + sizeof(char));
    *ptr = '\0';

    ptr = strrchr(string_copy, ':');
    if (!ptr)
        return -1;
    ptm.tm_min = atoi(ptr + sizeof(char));
    *ptr = '\0';

    ptr = strrchr(string_copy, 'T');
    if (!ptr)
        return -1;
    ptm.tm_hour = atoi(ptr + sizeof(char));
    *ptr = '\0';

    ptr = strrchr(string_copy, '-');
    if (!ptr)
        return -1;
    ptm.tm_mday = atoi(ptr + sizeof(char));
    *ptr = '\0';

    ptr = strrchr(string_copy, '-');
    if (!ptr)
        return -1;
    ptm.tm_mon = atoi(ptr + sizeof(char) -1);
    *ptr = '\0';

    ptm.tm_year = atoi(string_copy) - 1900;

    *time = mktime(&ptm);
    return 0;
}
