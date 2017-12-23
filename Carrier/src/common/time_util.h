#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

#include <stdint.h>

#include <common_export.h>

#ifdef __cplusplus
extern "C" {
#endif

COMMON_API
uint64_t get_monotonic_time(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIME_UTIL_H__ */
