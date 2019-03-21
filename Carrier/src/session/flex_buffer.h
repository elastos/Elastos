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

#ifndef __FLEX_BUFFER_H__
#define __FLEX_BUFFER_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include <crystal.h>

#include "ela_session.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FLEX_PADDING_LEN        128
#define FLEX_BUFFER_MAX_LEN     (FLEX_PADDING_LEN * 2 + ELA_MAX_USER_DATA_LEN)

/*
 * To reduce the times of buffer copy, using flex buffer mechanism.
 * The specific buffer format is depicted below:
 *
 * +----------------------------+--------------------------------------+
 * |<--        offset       --->|<--            data  size          -->|
 * +----------------------------+--------------------------------------+
 * |<--  buffer pointer.
 *                              |<--  payload pointer.
 *
 *
 *                              |<- N bytes ->|
 *
 * Forwarding flexing for N bytes (when unpacking received data).
 * +----------------+-----------+-------------+------------------------+
 * |<--                 offset             -->|<--    data  size    -->|
 * +----------------------------+--------------------------------------+
 * |<--  buffer pointer.
 *                                            |<--  payload pointer.
 *
 *
 *              |<-  N bytes -->|
 *
 * Backforward flexing for N bytes (when packing sending data).
 * +------------+---------------+--------------------------------------+
 * |<- offset ->|<--                  data  size                    -->|
 * +------------------------------+------------------------------------+
 *
 */
typedef struct FlexBuffer {
    size_t capacity;    /* Total space in buffer */
    size_t offset;      /* First available byte is buffer + offset */
    size_t size;        /* Data length, the last data byte is buffer + offset + size - 1 */
    char *buffer;
} FlexBuffer;

#define flex_buffer_alloca(__buf, __capacity, __offset) \
    do { \
        __buf = (FlexBuffer *)alloca(sizeof(FlexBuffer) + (__capacity)); \
        __buf->capacity = (__capacity); \
        __buf->offset = (__offset); \
        __buf->size = 0; \
        __buf->buffer = (char *)(__buf + 1); \
    } while (0)

#define flex_buffer_from(__buf, __offset, __src, __len) \
    do { \
        size_t __capacity = (__len) + (__offset); \
        __buf = (FlexBuffer *)alloca(sizeof(FlexBuffer) + __capacity); \
        __buf->capacity = __capacity; \
        __buf->offset = (__offset); \
        __buf->size = (__len); \
        __buf->buffer = (char *)(__buf + 1); \
        memcpy(__buf->buffer + (__offset), (__src), (__len)); \
    } while (0)

static inline
FlexBuffer *flex_buffer_init(FlexBuffer *buf, const void *buffer,
                             size_t capacity, size_t offset)
{
    buf->capacity = capacity;
    buf->offset = offset;
    buf->size = 0;
    buf->buffer = (char *)buffer;

    return buf;
}

static inline
FlexBuffer *flex_buffer_reset(FlexBuffer *buf, size_t offset)
{
    buf->offset = offset;
    buf->size = 0;

    return buf;
}

static inline
size_t flex_buffer_offset(FlexBuffer *buf)
{
    return buf->offset;
}

static inline
size_t flex_buffer_available(FlexBuffer *buf)
{
    return buf->capacity - buf->offset - buf->size;
}

static inline
size_t flex_buffer_size(FlexBuffer *buf)
{
    return buf->size;
}

static inline
size_t flex_buffer_set_size(FlexBuffer *buf, size_t size)
{
    buf->size = size;
    return size;
}

static inline
const void *flex_buffer_ptr(FlexBuffer *buf)
{
    return buf->buffer + buf->offset;
}

static inline
void *flex_buffer_mutable_ptr(FlexBuffer *buf)
{
    return buf->buffer + buf->offset;
}

static inline
size_t flex_buffer_forward_offset(FlexBuffer *buf, size_t bytes)
{
    assert(buf->offset + bytes <= buf->capacity);

    bytes = buf->offset + bytes <= buf->capacity ?
                bytes : buf->capacity - buf->offset - 1;

    buf->offset += bytes;
    buf->size -= bytes;

    return bytes;
}

static inline
size_t flex_buffer_backward_offset(FlexBuffer *buf, size_t bytes)
{
    assert(buf->offset >= bytes);

    bytes = buf->offset >= bytes ? bytes : buf->offset;
    buf->offset -= bytes;
    buf->size += bytes;

    return bytes;
}

static inline
FlexBuffer *flex_buffer_copy(FlexBuffer *dest, FlexBuffer *src)
{
    size_t len = dest->capacity - dest->offset;
    len = len > src->size ? src->size : len;

    memcpy(dest->buffer, src->buffer, len);
    dest->size = len;
    return dest;
}

static inline
FlexBuffer *flex_buffer_append(FlexBuffer *dest, FlexBuffer *src)
{
    size_t available = dest->capacity - dest->offset - dest->size;
    size_t len = src->size < available ? src->size : available;

    memcpy(dest->buffer + dest->offset + dest->size, src->buffer + src->offset, len);
    dest->size += len;
    return dest;
}

static inline
FlexBuffer *flex_buffer_append2(FlexBuffer *dest, FlexBuffer *src, size_t len)
{
    size_t available = dest->capacity - dest->offset - dest->size;
    len = len < available ? len : available;

    memcpy(dest->buffer + dest->offset + dest->size, src->buffer + src->offset, len);
    dest->size += len;
    return dest;
}

#ifdef __cplusplus
}
#endif

#endif /* __FLEX_BUFFER_H__ */
