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
#ifdef HAVE_GLOB_H
#include <glob.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <limits.h>

#include "common.h"
#include "did.h"

#define DID_MAX_LEN      512

static const char *PATH_SEP = "/";

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

int test_path(const char *path)
{
    struct stat s;

    if (!path || !*path)
        return -1;

    if (stat(path, &s) < 0)
        return -1;

    if (s.st_mode & S_IFDIR)
        return S_IFDIR;
    else if (s.st_mode & S_IFREG)
        return S_IFREG;
    else
        return -1;
}

int list_dir(const char *path, const char *pattern,
        int (*callback)(const char *name, void *context), void *context)
{
    char full_pattern[PATH_MAX];
    size_t len;
    int rc = 0;

    if (!path || !*path || !pattern || !callback)
        return -1;

    len = snprintf(full_pattern, sizeof(full_pattern), "%s/{.*,%s}", path, pattern);
    if (len == sizeof(full_pattern))
        full_pattern[len-1] = 0;

#if defined(_WIN32) || defined(_WIN64)
    struct _finddata_t c_file;
    intptr_t hFile;

    if ((hFile = _findfirst(full_pattern, &c_file )) == -1L)
        return -1;

    do {
        rc = callback(c_file.name, context);
        if(rc < 0) {
            break;
        }
    } while (_findnext(hFile, &c_file) == 0);

    _findclose(hFile);
#else
    glob_t gl;
    size_t pos = strlen(path) + 1;

    memset(&gl, 0, sizeof(gl));
    glob(full_pattern, GLOB_DOOFFS | GLOB_BRACE, NULL, &gl);

    for (int i = 0; i < gl.gl_pathc; i++) {
        char *fn = gl.gl_pathv[i] + pos;
        rc = callback(fn, context);
        if(rc < 0)
            break;
    }

    globfree(&gl);
#endif

    if (!rc)
        callback(NULL, context);

    return rc;
}

void delete_file(const char *path);

static int delete_file_helper(const char *path, void *context)
{
    char fullpath[PATH_MAX];
    int len;

    if (!path)
        return 0;

    if (strcmp(path, ".") != 0 && strcmp(path, "..") != 0) {
        len = snprintf(fullpath, sizeof(fullpath), "%s/%s", (char *)context, path);
        if (len < 0 || len > PATH_MAX)
            return -1;

        delete_file(fullpath);
    }

    return 0;
}

void delete_file(const char *path)
{
    int rc;

    if (!path || !*path)
        return;

    rc = test_path(path);
    if (rc < 0)
        return;

    if (rc == S_IFDIR) {
        list_dir(path, ".*", delete_file_helper, (void *)path);

        if (list_dir(path, "*", delete_file_helper, (void *)path) == 0)
            rmdir(path);
    } else {
        remove(path);
    }
}

static int get_dirv(char *path, bool create, int count, va_list components)
{
    struct stat st;
    int rc;

    assert(path);
    assert(count > 0);

    *path = 0;
    for (int i = 0; i < count; i++) {
        const char *component = va_arg(components, const char *);
        assert(component != NULL);
        strcat(path, component);

        rc = stat(path, &st);
        if (!create && rc < 0)
            return -1;

        if (create) {
            if (rc < 0) {
                if (errno != ENOENT || (errno == ENOENT && mkdir(path, S_IRWXU) < 0))
                    return -1;
            } else {
                if (!S_ISDIR(st.st_mode)) {
                    if (remove(path) < 0)
                        return -1;

                    if (mkdir(path, S_IRWXU) < 0)
                        return -1;
                }
            }
        }

        if (i < (count - 1))
            strcat(path, PATH_SEP);
    }

    return 0;
}

int get_dir(char* path, bool create, int count, ...)
{
    va_list components;
    int rc;

    if (!path || count <= 0)
        return -1;

    va_start(components, count);
    rc = get_dirv(path, create, count, components);
    va_end(components);

    return rc;
}

int get_file(char *path, bool create, int count, ...)
{
    const char *filename;
    va_list components;
    int rc;

    if (!path || count <= 0)
        return -1;

    va_start(components, count);
    rc = get_dirv(path, create, count - 1, components);
    if (rc < 0)
        return -1;

    va_end(components);
    va_start(components, count);
    for (int i = 0; i < count - 1; i++)
        va_arg(components, const char *);

    filename = va_arg(components, const char *);
    strcat(path, PATH_SEP);
    strcat(path, filename);

    va_end(components);
    return 0;
}

int store_file(const char *path, const char *string)
{
    int fd;
    size_t len, size;

    if (!path || !*path || !string)
        return -1;

    fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
        return -1;

    len = strlen(string);
    size = write(fd, string, len);
    if (size < len) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

const char *load_file(const char *path)
{
    int fd;
    size_t size;
    struct stat st;
    const char *data;

    if (!path)
        return NULL;

    fd = open(path, O_RDONLY);
    if (fd == -1)
        return NULL;

    if (fstat(fd, &st) < 0) {
        close(fd);
        return NULL;
    }

    size = st.st_size;
    data = (const char*)calloc(1, size + 1);
    if (!data) {
        close(fd);
        return NULL;
    }

    if (read(fd, (char*)data, size) != size) {
        free((char*)data);
        close(fd);
        return NULL;
    }

    close(fd);
    return data;
}

static int is_empty_helper(const char *path, void *context)
{
    if (!path) {
        *(int *)context = 0;
        return 0;
    }

    *(int *)context = 1;
    return -1;
}

bool is_empty(const char *path)
{
    int flag = 0;

    if (!path || !*path)
        return false;

    if (list_dir(path, "*", is_empty_helper, &flag) < 0 && flag)
        return false;

    return true;
}

static int mkdir_internal(const char *path, mode_t mode)
{
    struct stat st;
    int rc = 0;

    if (stat(path, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            rc = -1;
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        rc = -1;
    }

    return rc;
}

int mkdirs(const char *path, mode_t mode)
{
    int rc = 0;
    char *pp;
    char *sp;
    char copypath[PATH_MAX];

    strncpy(copypath, path, sizeof(copypath));
    copypath[sizeof(copypath) - 1] = 0;

    pp = copypath;
    while (rc == 0 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            /* Neither root nor double slash in path */
            *sp = '\0';
            rc = mkdir_internal(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }

    if (rc == 0)
        rc = mkdir_internal(path, mode);

    return rc;
}

bool cJSON_IsDouble(cJSON *item)
{
    return (item && cJSON_IsNumber(item) && (double)item->valueint != item->valuedouble);
}

bool cJSON_IsInt(cJSON *item)
{
    return (item && cJSON_IsNumber(item) && (double)item->valueint == item->valuedouble);
}