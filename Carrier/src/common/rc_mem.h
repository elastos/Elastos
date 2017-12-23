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
