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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <cjson/cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DOC_BUFFER_LEN   512
#define CHECK(func)        do { if (func == -1) return -1; } while(0)

const char *get_time_string(char *timestring, size_t len, time_t *p_time);

int parse_time(time_t *time, const char *string);

int test_path(const char *path);

int list_dir(const char *path, const char *pattern,
        int (*callback)(const char *name, void *context), void *context);

void delete_file(const char *path);

int get_dir(char* path, bool create, int count, ...);

int get_file(char *path, bool create, int count, ...);

int store_file(const char *path, const char *string);

const char *load_file(const char *path);

bool is_empty(const char *path);

int mkdirs(const char *path, mode_t mode);

//for json
bool cJSON_IsDouble(cJSON *item);

bool cJSON_IsInt(cJSON *item);

#ifdef __cplusplus
}
#endif

#endif //__COMMON_H__