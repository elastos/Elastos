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
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>

#include <vlog.h>
#include "timerheap.h"

#define HEAP_PARENT(X)	(X == 0 ? 0 : (((X) - 1) / 2))
#define HEAP_LEFT(X)	(((X)+(X))+1)

#define DEFAULT_MAX_TIMED_OUT_PER_POLL  (64)

/*
static inline
int mutex_lock_init(mutex_lock_t *lock)
{
    return pthread_mutex_init((pthread_mutex_t *)lock, NULL);
}
*/
static inline
void mutex_lock_destroy(mutex_lock_t *lock)
{
    pthread_mutex_destroy((pthread_mutex_t *)lock);
}

static inline
int mutex_lock_acquire(mutex_lock_t *lock)
{
    return pthread_mutex_lock((pthread_mutex_t *)lock);
}

/*
static inline
int mutex_lock_try_acquire(mutex_lock_t *lock)
{
    return pthread_mutex_trylock((pthread_mutex_t *)lock);
}
*/
static inline
int mutex_lock_release(mutex_lock_t *lock)
{
    return pthread_mutex_unlock((pthread_mutex_t *)lock);
}

/******************************************************************************
 * Micro APIs of time_val_t
 */

/**
 * Get the total time value in miliseconds. This is the same as
 * multiplying the second part with 1000 and then add the miliseconds
 * part to the result.
 *
 * @param t     The time value.
 * @return      Total time in miliseconds.
 * @hideinitializer
 */
#define TIME_VAL_MSEC(t)	((t).sec * 1000 + (t).msec)

/**
 * This macro will check if \a t1 is equal to \a t2.
 *
 * @param t1    The first time value to compare.
 * @param t2    The second time value to compare.
 * @return      Non-zero if both time values are equal.
 * @hideinitializer
 */
#define TIME_VAL_EQ(t1, t2)	((t1).sec==(t2).sec && (t1).msec==(t2).msec)

/**
 * This macro will check if \a t1 is greater than \a t2
 *
 * @param t1    The first time value to compare.
 * @param t2    The second time value to compare.
 * @return      Non-zero if t1 is greater than t2.
 * @hideinitializer
 */
#define TIME_VAL_GT(t1, t2)	((t1).sec>(t2).sec || \
                            ((t1).sec==(t2).sec && (t1).msec>(t2).msec))

/**
 * This macro will check if \a t1 is greater than or equal to \a t2
 *
 * @param t1    The first time value to compare.
 * @param t2    The second time value to compare.
 * @return      Non-zero if t1 is greater than or equal to t2.
 * @hideinitializer
 */
#define TIME_VAL_GTE(t1, t2)    (TIME_VAL_GT(t1,t2) || \
                                 TIME_VAL_EQ(t1,t2))

/**
 * This macro will check if \a t1 is less than \a t2
 *
 * @param t1    The first time value to compare.
 * @param t2    The second time value to compare.
 * @return      Non-zero if t1 is less than t2.
 * @hideinitializer
 */
#define TIME_VAL_LT(t1, t2)     (!(TIME_VAL_GTE(t1,t2)))

/**
 * This macro will check if \a t1 is less than or equal to \a t2.
 *
 * @param t1    The first time value to compare.
 * @param t2    The second time value to compare.
 * @return      Non-zero if t1 is less than or equal to t2.
 * @hideinitializer
 */
#define TIME_VAL_LTE(t1, t2)	(!TIME_VAL_GT(t1, t2))

/**
 * Add \a t2 to \a t1 and store the result in \a t1. Effectively
 *
 * this macro will expand as: (\a t1 += \a t2).
 * @param t1    The time value to add.
 * @param t2    The time value to be added to \a t1.
 * @hideinitializer
 */
#define TIME_VAL_ADD(t1, t2)	    do {			\
                        (t1).sec += (t2).sec;	    \
                        (t1).msec += (t2).msec;	    \
                        time_val_normalize(&(t1));  \
				    } while (0)


/**
 * Substract \a t2 from \a t1 and store the result in \a t1. Effectively
 * this macro will expand as (\a t1 -= \a t2).
 *
 * @param t1    The time value to subsctract.
 * @param t2    The time value to be substracted from \a t1.
 * @hideinitializer
 */
#define TIME_VAL_SUB(t1, t2)	    do {			\
                        (t1).sec -= (t2).sec;	    \
                        (t1).msec -= (t2).msec;	    \
                        time_val_normalize(&(t1));  \
				    } while (0)


static void time_val_normalize(time_val_t *t)
{
    if (t->msec >= 1000) {
        t->sec += (t->msec / 1000);
        t->msec = (t->msec % 1000);
    }
    else if (t->msec <= -1000) {
        do {
            t->sec--;
            t->msec += 1000;
        } while (t->msec <= -1000);
    }

    if (t->sec >= 1 && t->msec < 0) {
        t->sec--;
        t->msec += 1000;

    } else if (t->sec < 0 && t->msec > 0) {
        t->sec++;
        t->msec -= 1000;
    }
}

static void time_gettickcount(time_val_t *t)
{
    struct timeval tv;

    assert(t);
    gettimeofday(&tv, NULL);

    t->sec = (long)tv.tv_sec;
    t->msec = (long)tv.tv_usec/1000;
}

/******************************************************************************
 * Implementation of timer heap
 */

enum
{
    F_DONT_CALL = 1,
    F_DONT_ASSERT = 2,
    F_SET_ID = 4
};

/**
 * The implementation of timer heap.
 */
typedef struct timer_heap
{
    /** Maximum size of the heap. */
    size_t max_size;

    /** Current size of the heap. */
    size_t cur_size;

    /** Max timed out entries to process per poll. */
    unsigned max_entries_per_poll;

    /** Lock object. */
    mutex_lock_t *lock;

    /** Autodelete lock. */
    bool auto_delete_lock;

    /**
     * Current contents of the Heap, which is organized as a "heap" of
     * timer_entry_t *'s.  In this context, a heap is a "partially
     * ordered, almost complete" binary tree, which is stored in an
     * array.
     */
    timer_entry_t **heap;

    /**
     * An array of "pointers" that allows each timer_entry_t in the
     * <heap_> to be located in O(1) time.  Basically, <timer_id_[i]>
     * contains the slot in the <heap_> array where an timer_entry_t
     * with timer id <i> resides.  Thus, the timer id passed back from
     * <schedule_entry> is really an slot into the <timer_ids> array.  The
     * <timer_ids_> array serves two purposes: negative values are
     * treated as "pointers" for the <freelist_>, whereas positive
     * values are treated as "pointers" into the <heap_> array.
     */
    timer_id_t *timer_ids;

    /**
     * "Pointer" to the first element in the freelist contained within
     * the <timer_ids_> array, which is organized as a stack.
     */
    timer_id_t timer_ids_freelist;

    /** Callback to be called when a timer expires. */
    timer_heap_callback *callback;

} timer_heap_t;

static inline void lock_timer_heap(timer_heap_t *ht)
{
    if (ht->lock)
        mutex_lock_acquire(ht->lock);
}

static inline void unlock_timer_heap(timer_heap_t *ht)
{
    if (ht->lock)
        mutex_lock_release(ht->lock);
}

static void copy_node(timer_heap_t *ht, size_t slot, timer_entry_t *moved_node)
{
    // Insert <moved_node> into its new location in the heap.
    ht->heap[slot] = moved_node;

    // Update the corresponding slot in the parallel <timer_ids_> array.
    ht->timer_ids[moved_node->_timer_id] = (int)slot;
}

static timer_id_t pop_freelist(timer_heap_t *ht)
{
    // We need to truncate this to <int> for backwards compatibility.
    timer_id_t new_id = ht->timer_ids_freelist;

    // The freelist values in the <timer_ids_> are negative, so we need
    // to negate them to get the next freelist "pointer."
    ht->timer_ids_freelist = -ht->timer_ids[ht->timer_ids_freelist];
    
    return new_id;
    
}

static void push_freelist(timer_heap_t *ht, timer_id_t old_id)
{
    // The freelist values in the <timer_ids_> are negative, so we need
    // to negate them to get the next freelist "pointer."
    ht->timer_ids[old_id] = -ht->timer_ids_freelist;
    ht->timer_ids_freelist = old_id;
}

static void reheap_down(timer_heap_t *ht, timer_entry_t *moved_node,
                        size_t slot, size_t child)
{
    // Restore the heap property after a deletion.
    
    while (child < ht->cur_size) {
        // Choose the smaller of the two children.
        if (child + 1 < ht->cur_size
            && TIME_VAL_LT(ht->heap[child + 1]->_timer_value,
                           ht->heap[child]->_timer_value)) {
            child++;
        }
	
        // Perform a <copy> if the child has a larger timeout value than
        // the <moved_node>.
        if (TIME_VAL_LT(ht->heap[child]->_timer_value,
                        moved_node->_timer_value)) {

            copy_node( ht, slot, ht->heap[child]);
            slot = child;
            child = HEAP_LEFT(child);
        } else
            // We've found our location in the heap.
            break;
    }
    
    copy_node( ht, slot, moved_node);
}

static void reheap_up(timer_heap_t *ht, timer_entry_t *moved_node,
		              size_t slot, size_t parent)
{
    // Restore the heap property after an insertion.
    
    while (slot > 0) {
	    // If the parent node is greater than the <moved_node> we need
	    // to copy it down.
	    if (TIME_VAL_LT(moved_node->_timer_value,
                        ht->heap[parent]->_timer_value)) {

            copy_node(ht, slot, ht->heap[parent]);
            slot = parent;
            parent = HEAP_PARENT(slot);

        } else
            break;
    }
    
    // Insert the new node into its proper resting place in the heap and
    // update the corresponding slot in the parallel <timer_ids> array.
    copy_node(ht, slot, moved_node);
}


static timer_entry_t * remove_node(timer_heap_t *ht, size_t slot)
{
    timer_entry_t *removed_node = ht->heap[slot];
    
    // Return this timer id to the freelist.
    push_freelist( ht, removed_node->_timer_id );
    
    // Decrement the size of the heap by one since we're removing the
    // "slot"th node.
    ht->cur_size--;
    
    // Set the ID
    removed_node->_timer_id = -1;

    // Only try to reheapify if we're not deleting the last entry.
    
    if (slot < ht->cur_size) {
        size_t parent;
        timer_entry_t *moved_node = ht->heap[ht->cur_size];
	
        // Move the end node to the location being removed and update
        // the corresponding slot in the parallel <timer_ids> array.
        copy_node( ht, slot, moved_node);
	
        // If the <moved_node->time_value_> is great than or equal its
        // parent it needs be moved down the heap.
        parent = HEAP_PARENT (slot);
	
        if (TIME_VAL_GTE(moved_node->_timer_value, ht->heap[parent]->_timer_value))
            reheap_down( ht, moved_node, slot, HEAP_LEFT(slot));
        else
            reheap_up( ht, moved_node, slot, parent);
    }
    
    return removed_node;
}

static void grow_heap(timer_heap_t *ht)
{
    // All the containers will double in size from max_size_
    size_t new_size = ht->max_size * 2;
    timer_id_t *new_timer_ids;
    size_t i;
    
    // First grow the heap itself.
    
    timer_entry_t **new_heap = 0;
    
    new_heap = (timer_entry_t **)realloc(ht->heap, sizeof(timer_entry_t*) * new_size);
    // TODO: need check success or not.
    ht->heap = new_heap;
    
    // Grow the array of timer ids.
    
    new_timer_ids = 0;
    new_timer_ids = (timer_id_t*)realloc(ht->timer_ids, new_size * sizeof(timer_id_t));
    // TODOX: need check success or not.
    ht->timer_ids = new_timer_ids;
    
    // And add the new elements to the end of the "freelist".
    for (i = ht->max_size; i < new_size; i++)
        ht->timer_ids[i] = -((timer_id_t) (i + 1));
    
    ht->max_size = new_size;
}

static void insert_node(timer_heap_t *ht, timer_entry_t *new_node)
{
    if (ht->cur_size + 2 >= ht->max_size)
        grow_heap(ht);
    
    reheap_up(ht, new_node, ht->cur_size, HEAP_PARENT(ht->cur_size));
    ht->cur_size++;
}

static int schedule_entry(timer_heap_t *ht, timer_entry_t *entry,
                          const time_val_t *future_time)
{
    if (ht->cur_size < ht->max_size) {
        // Obtain the next unique sequence number.
        // Set the entry
        entry->_timer_id = pop_freelist(ht);
        entry->_timer_value = *future_time;
        insert_node( ht, entry);
        return 0;
    }
    else
        return -1;
}

static int cancel(timer_heap_t *ht, timer_entry_t *entry, unsigned flags)
{
    long timer_node_slot;

    // Check to see if the timer_id is out of range
    if (entry->_timer_id < 0 || (size_t)entry->_timer_id > ht->max_size) {
        entry->_timer_id = -1;
        return 0;
    }

    timer_node_slot = ht->timer_ids[entry->_timer_id];

    if (timer_node_slot < 0) { // Check to see if timer_id is still valid.
        entry->_timer_id = -1;
        return 0;
    }

    if (entry != ht->heap[timer_node_slot]) {
        if ((flags & F_DONT_ASSERT) == 0)
            assert(entry == ht->heap[timer_node_slot]);
        entry->_timer_id = -1;
        return 0;

    } else {
        remove_node( ht, timer_node_slot);

        if ((flags & F_DONT_CALL) == 0)
            // Call the close hook.
        (*ht->callback)(ht, entry);
        return 1;
    }
}

size_t timer_heap_mem_size(size_t count)
{
    return /* size of the timer heap itself: */
           sizeof(timer_heap_t) +
           /* size of each entry: */
           (count+2) * (sizeof(timer_entry_t*)+sizeof(timer_id_t)) +
           /* lock: */
           sizeof(mutex_lock_t); //CHECK ME!!!
}

/*
 * Create a new timer heap.
 */
timer_heap_t *timer_heap_create(size_t size)
{
    timer_heap_t *ht;
    size_t i;

    /* Magic? */
    size += 2;

    /* Allocate timer heap data structure from the pool */
    ht = (timer_heap_t *)calloc(1, sizeof(timer_heap_t));
    if (!ht)
        return NULL;

    /* Initialize timer heap sizes */
    ht->max_size = size;
    ht->cur_size = 0;
    ht->max_entries_per_poll = DEFAULT_MAX_TIMED_OUT_PER_POLL;
    ht->timer_ids_freelist = 1;

    /* Lock. */
    ht->lock = NULL;
    ht->auto_delete_lock = 0;

    // Create the heap array.
    ht->heap = (timer_entry_t**)calloc(1, sizeof(timer_entry_t *) * size);
    if (!ht->heap) {
        free(ht);
        return NULL;
    }

    // Create the parallel
    ht->timer_ids = (timer_id_t *)calloc(1, sizeof(timer_id_t *) * size);
    if (!ht->timer_ids) {
        free(ht->heap);
        free(ht);
        return NULL;
    }

    // Initialize the "freelist," which uses negative values to
    // distinguish freelist elements from "pointers" into the <heap_>
    // array.
    for (i=0; i<size; ++i)
        ht->timer_ids[i] = -((timer_id_t) (i + 1));

    return ht;
}

void timer_heap_destroy(timer_heap_t *ht)
{
    if (ht->lock && ht->auto_delete_lock) {
        mutex_lock_destroy(ht->lock);
        ht->lock = NULL;
    }

    if (ht->heap)
        free(ht->heap);

    if (ht->timer_ids)
        free(ht->timer_ids);

    free(ht);
}

void timer_heap_set_lock(timer_heap_t *ht,  mutex_lock_t *lock, bool auto_del)
{
    if (ht->lock && ht->auto_delete_lock)
        mutex_lock_destroy(ht->lock);

    ht->lock = lock;
    ht->auto_delete_lock = auto_del;
}

unsigned timer_heap_set_max_timed_out_per_poll(timer_heap_t *ht, unsigned count)
{
    unsigned old_count = ht->max_entries_per_poll;
    ht->max_entries_per_poll = count;
    return old_count;
}

timer_entry_t *timer_entry_init(timer_entry_t *entry, int id, void *user_data,
                                timer_heap_callback *cb)
{
    assert(entry && cb);

    entry->_timer_id = -1;
    entry->id = id;
    entry->user_data = user_data;
    entry->cb = cb;

    return entry;
}

bool timer_entry_running(timer_entry_t *entry)
{
    return (entry->_timer_id >= 1);
}

static int schedule(timer_heap_t *ht, timer_entry_t *entry,
                    const time_val_t *delay, bool set_id, int id_val)
{
    int status;
    time_val_t expires;

    if (!ht || !entry || !delay)
        return EINVAL;

    if (!entry->cb)
        return EINVAL;

    /* Prevent same entry from being scheduled more than once */
    if (entry->_timer_id >= 1)
        return EINVAL;

    time_gettickcount(&expires);
    TIME_VAL_ADD(expires, *delay);
    
    lock_timer_heap(ht);
    status = schedule_entry(ht, entry, &expires);
    if (status == 0) {
        if (set_id)
            entry->id = id_val;
    }
    unlock_timer_heap(ht);

    return status;
}

int timer_heap_schedule(timer_heap_t *ht, timer_entry_t *entry,
                        const time_val_t *delay)
{
    return schedule(ht, entry, delay, false, 1);
}

static int cancel_timer(timer_heap_t *ht, timer_entry_t *entry,
                        unsigned flags, int id_val)
{
    int count;

    if (!ht || !entry)
        return EINVAL;

    lock_timer_heap(ht);
    count = cancel(ht, entry, flags | F_DONT_CALL);
    if (flags & F_SET_ID) {
        entry->id = id_val;
    }
    unlock_timer_heap(ht);

    return count;
}

int timer_heap_cancel(timer_heap_t *ht, timer_entry_t *entry)
{
    return cancel_timer(ht, entry, 0, 0);
}

int timer_heap_cancel_if_active(timer_heap_t *ht, timer_entry_t *entry,
                                int id_val)
{
    return cancel_timer(ht, entry, F_SET_ID | F_DONT_ASSERT, id_val);
}

unsigned timer_heap_poll(timer_heap_t *ht, time_val_t *next_delay)
{
    time_val_t now;
    unsigned count;

    assert(ht);

    if (!ht)
        return 0;

    lock_timer_heap(ht);
    if (!ht->cur_size && next_delay) {
        next_delay->sec = next_delay->msec = INT32_MAX;
        unlock_timer_heap(ht);
        return 0;
    }

    count = 0;
    time_gettickcount(&now);

    while ( ht->cur_size && TIME_VAL_LTE(ht->heap[0]->_timer_value, now) &&
            count < ht->max_entries_per_poll ) 
    {
        timer_entry_t *node = remove_node(ht, 0);

        ++count;

        unlock_timer_heap(ht);

        if (node->cb)
            (*node->cb)(ht, node);

        lock_timer_heap(ht);
    }

    if (ht->cur_size && next_delay) {
        *next_delay = ht->heap[0]->_timer_value;
        TIME_VAL_SUB(*next_delay, now);

        if (next_delay->sec < 0 || next_delay->msec < 0)
            next_delay->sec = next_delay->msec = 0;
    } else if (next_delay) {
        next_delay->sec = next_delay->msec = INT32_MAX;
    }

    unlock_timer_heap(ht);

    return count;
}

size_t timer_heap_count(timer_heap_t *ht)
{
    assert(ht);
    return ht->cur_size;
}

int timer_heap_earlist_time(timer_heap_t *ht, time_val_t *timeval)
{
    assert(ht->cur_size != 0);

    if (ht->cur_size == 0) {
        timeval->sec = timeval->msec = INT32_MAX;
        return -1;
    }

    lock_timer_heap(ht);
    *timeval = ht->heap[0]->_timer_value;
    unlock_timer_heap(ht);

    return 0;
}

void timer_heap_dump(timer_heap_t *ht)
{
    lock_timer_heap(ht);

    vlogD("Dumping timer heap:");
    vlogD("Cur size: %d entries, max: %d",
          (int)ht->cur_size, (int)ht->max_size);

    if (ht->cur_size) {
        unsigned i;
        time_val_t now;

        vlogD("Entries: ");
        vlogD("_id\tId\tElapsed\tSource");
        vlogD("----------------------------------");

        time_gettickcount(&now);


        for (i=0; i<(unsigned)ht->cur_size; ++i) {
            timer_entry_t *e = ht->heap[i];
            time_val_t delta;

            if (TIME_VAL_LTE(e->_timer_value, now)) {
                delta.sec = delta.msec = 0;
            } else {
                delta = e->_timer_value;
                TIME_VAL_SUB(delta, now);
            }

            vlogD("   %d\t%d\t%d.%03d", e->_timer_id, e->id,
                (int)delta.sec, (int)delta.msec);
        }
    }

    unlock_timer_heap(ht);
}
