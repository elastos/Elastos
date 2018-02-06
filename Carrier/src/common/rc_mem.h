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

#ifndef __RC_MEM_H__
#define __RC_MEM_H__

#include <stddef.h>

#include <common_export.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defines the memory destructor handler, which is called when the reference
 * of a memory object goes down to zero
 *
 * @param
 *      data    Pointer to memory object
 */
typedef void (rc_mem_destructor)(void *data);

/**
 * Allocate a new reference-counted memory object
 *
 * @param 
 *      size          Size of memory object
 * @param
 *      destructor    Optional destructor, called when destroyed
 *
 * @return Pointer to allocated object
 */
COMMON_API
void *rc_alloc(size_t size, rc_mem_destructor *destructor);

/**
 * Allocate a new reference-counted memory object. Memory is zeroed.
 *
 * @param
 *      size            Size of memory object
 * @param
 *      destructor      Optional destructor, called when destroyed
 *
 * @return Pointer to allocated object
 */
COMMON_API
void *rc_zalloc(size_t size, rc_mem_destructor *destructor);

/**
 * Re-allocate a reference-counted memory object
 *
 * @param
 *      data    Memory object
 * @param
 *      size    New size of memory object
 *
 * @return New pointer to allocated object
 *
 * @note Realloc NULL pointer is not supported
 */
COMMON_API
void *rc_realloc(void *data, size_t size);

/**
 * Reference a reference-counted memory object
 *
 * @param
 *      data    Memory object
 *
 * @return Memory object (same as data)
 */
COMMON_API
void *ref(void *data);

/**
 * Dereference a reference-counted memory object. When the reference count
 * is zero, the destroy handler will be called (if present) and the memory
 * will be freed
 *
 * @param
 *      data        Memory object
 *
 * @return Always NULL
 */
COMMON_API
void *deref(void *data);

/**
 * Get number of references to a reference-counted memory object
 *
 * @param
 *      data        Memory object
 *
 * @return Number of references
 */
COMMON_API
unsigned int nrefs(const void *data);

#ifdef __cplusplus
}
#endif

#endif /* __RC_MEM_H__ */
