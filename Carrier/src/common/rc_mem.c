#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "rc_mem.h"

/** Defines a reference-counting memory object */
struct rc_mem {
    uint32_t nrefs;                 /**< Number of references  */
    rc_mem_destructor *destructor;  /**< destructor            */
#ifndef NDEBUG
    uint32_t magic;                 /**< Magic number          */
#endif
};

#ifndef NDEBUG
static const uint32_t mem_magic = 0xa9b8ada8;

#define MAGIC_CHECK(m) \
    if (mem_magic != (m)->magic) assert(0 && "magic check failed!");
#else
#define MAGIC_CHECK(m)
#endif

void *rc_alloc(size_t size, rc_mem_destructor *destructor)
{
    struct rc_mem *m;

    m = malloc(sizeof(struct rc_mem) + size);
    if (!m)
        return NULL;

    m->nrefs = 1;
    m->destructor = destructor;
#ifndef NDEBUG
    m->magic = mem_magic;
#endif

    return (void *)(m + 1);
}

void *rc_zalloc(size_t size, rc_mem_destructor *destructor)
{
    void *p;

    p = rc_alloc(size, destructor);
    if (!p)
        return NULL;

    memset(p, 0, size);
    
    return p;
}

void *rc_realloc(void *data, size_t size)
{
    struct rc_mem *m, *m2;

    if (!data)
        return NULL;

    m = ((struct rc_mem *)data) - 1;

    MAGIC_CHECK(m);

    m2 = realloc(m, sizeof(struct rc_mem) + size);
    if (!m2) {
        return NULL;
    }
    
    return (void *)(m2 + 1);
}

void *ref(void *data)
{
    struct rc_mem *m;

    if (!data)
        return NULL;

    m = ((struct rc_mem *)data) - 1;

    MAGIC_CHECK(m);

#if defined(_WIN32) || defined(_WIN64)
    InterlockedIncrement(&m->nrefs);
#else
    __sync_add_and_fetch(&m->nrefs, 1);
#endif
    
    return data;
}

void *deref(void *data)
{
    struct rc_mem *m;
    uint32_t nrefs;

    if (!data)
        return NULL;

    m = ((struct rc_mem *)data) - 1;

    MAGIC_CHECK(m);

    nrefs =
#if defined(_WIN32) || defined(_WIN64)
    InterlockedDecrement(&m->nrefs);
#else
    __sync_sub_and_fetch(&m->nrefs, 1);
#endif

    if (nrefs > 0)
        return NULL;

    if (m->destructor)
        m->destructor(data);

    /* NOTE: check if the destructor called ref() */
    if (m->nrefs > 0)
        return NULL;

    free(m);
    
    return NULL;
}

uint32_t nrefs(const void *data)
{
    struct rc_mem *m;

    if (!data)
        return 0;

    m = ((struct rc_mem *)data) - 1;

    MAGIC_CHECK(m);

    return m->nrefs;
}
