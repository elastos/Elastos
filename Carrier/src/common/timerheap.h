#ifndef __TIMERHEAP_H__
#define __TIMERHEAP_H__

#include <pthread.h>

#include <common_export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef pthread_mutex_t    mutex_lock_t;

/**
 * Representation of time value in this library.
 * This type can be used to represent either an interval or a specific time
 * or date.
 */
typedef struct time_val
{
    /** The seconds part of the time. */
    long    sec;

    /** The miliseconds fraction of the time. */
    long    msec;

} time_val_t;

/**
 * The type for internal timer ID.
 */
typedef int timer_id_t;

/**
 * The type of timer heap.
 */
typedef struct timer_heap  timer_heap_t;
typedef struct timer_entry timer_entry_t;

/**
 * The type of callback function to be called by timer scheduler when a timer
 * has expired.
 *
 * @param timer_heap    The timer heap.
 * @param entry         Timer entry which timer's has expired.
 */
typedef void timer_heap_callback(timer_heap_t *timer_heap,
                                 timer_entry_t *entry);

/**
 * This structure represents an entry to the timer.
 */
typedef struct timer_entry
{
    /** 
     * User data to be associated with this entry. 
     * Applications normally will put the instance of object that
     * owns the timer entry in this field.
     */
    void *user_data;

    /** 
     * Arbitrary ID assigned by the user/owner of this entry. 
     * Applications can use this ID to distinguish multiple
     * timer entries that share the same callback and user_data.
     */
    int id;

    /** 
     * Callback to be called when the timer expires. 
     */
    timer_heap_callback *cb;

    /** 
     * Internal unique timer ID, which is assigned by the timer heap. 
     * Application should not touch this ID.
     */
    timer_id_t _timer_id;

    /** 
     * The future time when the timer expires, which the value is updated
     * by timer heap when the timer is scheduled.
     */
    time_val_t _timer_value;
} timer_entry_t;

/**
 * Calculate memory size required to create a timer heap.
 *
 * @param count     Number of timer entries to be supported.
 * @return          Memory size requirement in bytes.
 */
COMMON_API
size_t timer_heap_mem_size(size_t count);

/**
 * Create a timer heap.
 *
 * @param count     The maximum number of timer entries to be supported
 *                  initially. If the application registers more entries 
 *                  during runtime, then the timer heap will resize.
 * @return          Pointer to created timer heap on success, otherwise NULL.
 */
COMMON_API
timer_heap_t *timer_heap_create(size_t count);

/**
 * Destroy the timer heap.
 *
 * @param ht        The timer heap.
 */
COMMON_API
void timer_heap_destroy(timer_heap_t *ht);

/**
 * Set lock object to be used by the timer heap. By default, the timer heap
 * uses dummy synchronization.
 *
 * @param ht        The timer heap.
 * @param lock      The lock object to be used for synchronization.
 * @param auto_del  If nonzero, the lock object will be destroyed when
 *                  the timer heap is destroyed.
 */
COMMON_API
void timer_heap_set_lock(timer_heap_t *ht, mutex_lock_t *lock, bool auto_del);

/**
 * Set maximum number of timed out entries to process in a single poll.
 *
 * @param ht        The timer heap.
 * @param count     Number of entries.
 *
 * @return          The old number.
 */
COMMON_API
unsigned timer_heap_set_max_timed_out_per_poll(timer_heap_t *ht, unsigned count);

/**
 * Initialize a timer entry. Application should call this function at least
 * once before scheduling the entry to the timer heap, to properly initialize
 * the timer entry.
 *
 * @param entry     The timer entry to be initialized.
 * @param id        Arbitrary ID assigned by the user/owner of this entry.
 *                  Applications can use this ID to distinguish multiple
 *                  timer entries that share the same callback and user_data.
 * @param user_data User data to be associated with this entry. 
 *                  Applications normally will put the instance of object that
 *                  owns the timer entry in this field.
 * @param cb        Callback function to be called when the timer elapses.
 *
 * @return          The timer entry itself.
 */
COMMON_API
timer_entry_t *timer_entry_init(timer_entry_t *entry, int id, void *user_data,
                                timer_heap_callback *cb );

/**
 * Queries whether a timer entry is currently running.
 *
 * @param entry     The timer entry to query.
 *
 * @return          true if the timer is running.  false if not.
 */
COMMON_API
bool timer_entry_running(timer_entry_t *entry );

/**
 * Schedule a timer entry which will expire AFTER the specified delay.
 *
 * @param ht        The timer heap.
 * @param entry     The entry to be registered. 
 * @param delay     The interval to expire.
 * @return          0, or the appropriate error code.
 */
COMMON_API
int timer_heap_schedule(timer_heap_t *ht, timer_entry_t *entry,
                        const time_val_t *delay);

/**
 * Cancel a previously registered timer. This will also decrement the
 * reference counter of the group lock associated with the timer entry,
 * if the entry was scheduled with one.
 *
 * @param ht        The timer heap.
 * @param entry     The entry to be cancelled.
 * @return          The number of timer cancelled, which should be one if the
 *                  entry has really been registered, or zero if no timer was
 *                  cancelled.
 */
COMMON_API
int timer_heap_cancel(timer_heap_t *ht, timer_entry_t *entry);

/**
 * Cancel only if the previously registered timer is active. This will
 * also decrement the reference counter of the group lock associated
 * with the timer entry, if the entry was scheduled with one. In any
 * case, set the "id" to the specified value.
 *
 * @param ht        The timer heap.
 * @param entry     The entry to be cancelled.
 * @param id_val    Value to be set to "id"
 *
 * @return          The number of timer cancelled, which should be one if the
 *                  entry has really been registered, or zero if no timer was
 *                  cancelled.
 */
COMMON_API
int timer_heap_cancel_if_active(timer_heap_t *ht, timer_entry_t *entry,
                                int id_val);

/**
 * Get the number of timer entries.
 *
 * @param ht        The timer heap.
 * @return          The number of timer entries.
 */
COMMON_API
size_t timer_heap_count(timer_heap_t *ht );

/**
 * Get the earliest time registered in the timer heap. The timer heap
 * MUST have at least one timer being scheduled (application should use
 * #timer_heap_count() before calling this function).
 *
 * @param ht        The timer heap.
 * @param timeval   The time deadline of the earliest timer entry.
 *
 * @return          0, or -1 if no entry is scheduled.
 */
COMMON_API
int timer_heap_earliest_time(timer_heap_t *ht, time_val_t *timeval);

/**
 * Poll the timer heap, check for expired timers and call the callback for
 * each of the expired timers.
 *
 * @param ht         The timer heap.
 * @param next_delay If this parameter is not NULL, it will be filled up with
 *		     the time delay until the next timer elapsed, or 
 *		     MAXINT32 in the sec part if no entry exist.
 *
 * @return           The number of timers expired.
 */
COMMON_API
unsigned timer_heap_poll(timer_heap_t *ht, time_val_t *next_delay);

/**
 * Dump timer heap entries.
 *
 * @param ht	    The timer heap.
 */
COMMON_API
void timer_heap_dump(timer_heap_t *ht);

#ifdef __cplusplus
}
#endif

#endif	/* __TIMERHEAP_H__ */

