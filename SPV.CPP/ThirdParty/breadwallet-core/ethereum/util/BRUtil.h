//
//  BBRUtil.h
//  Core Ethereum
//
//  Created by Ed Gamble on 3/16/2018.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Util_H
#define BR_Util_H

#include "BRUtilHex.h"
#include "BRUtilMath.h"

#if 0
#define eth_log(topic, formatter, ...)   _eth_log("ETH: %s: " formatter "\n", (topic), __VA_ARGS__)
#else

#ifdef __cplusplus
extern "C" void ElastosElaWalletLogDebug(const char *s);
extern "C" void ElastosElaWalletLogInfo(const char *s);
extern "C" void ElastosElaWalletLogWarn(const char *s);
extern "C" void ElastosElaWalletLogError(const char *s);
extern "C" void ElastosElaWalletLogCritical(const char *s);
#else
void ElastosElaWalletLogDebug(const char *s);
void ElastosElaWalletLogInfo(const char *s);
void ElastosElaWalletLogWarn(const char *s);
void ElastosElaWalletLogError(const char *s);
void ElastosElaWalletLogCritical(const char *s);
#endif

#define eth_log(topic, formatter, ...)   do { \
	char buf[2048]; \
	snprintf(buf, sizeof(buf), "ETH: %s: " formatter, (topic), __VA_ARGS__); \
	buf[sizeof(buf) - 1] = '\0'; \
	ElastosElaWalletLogInfo(buf); \
} while (0)

#define eth_log_dbg(topic, formatter, ...)   do { \
	char buf[2048]; \
	snprintf(buf, sizeof(buf), "ETH: %s: " formatter, (topic), __VA_ARGS__); \
	buf[sizeof(buf) - 1] = '\0'; \
	ElastosElaWalletLogDebug(buf); \
} while (0)

#define eth_log_info(topic, formatter, ...)   do { \
	char buf[2048]; \
	snprintf(buf, sizeof(buf), "ETH: %s: " formatter, (topic), __VA_ARGS__); \
	buf[sizeof(buf) - 1] = '\0'; \
	ElastosElaWalletLogInfo(buf); \
} while (0)

#define eth_log_warn(topic, formatter, ...)   do { \
	char buf[2048]; \
	snprintf(buf, sizeof(buf), "ETH: %s: " formatter, (topic), __VA_ARGS__); \
	buf[sizeof(buf) - 1] = '\0'; \
	ElastosElaWalletLogWarn(buf); \
} while (0)

#define eth_log_err(topic, formatter, ...)   do { \
	char buf[2048]; \
	snprintf(buf, sizeof(buf), "ETH: %s: " formatter, (topic), __VA_ARGS__); \
	buf[sizeof(buf) - 1] = '\0'; \
	ElastosElaWalletLogError(buf); \
} while (0)

#endif

#if defined(TARGET_OS_MAC)
//#  include <Foundation/Foundation.h>
//#  define _eth_log(...) NSLog(__VA_ARGS__)
#  include <stdio.h>
#  define _eth_log(...) printf(__VA_ARGS__)
#elif defined(__ANDROID__)
#  include <android/log.h>
#  define _eth_log(...) __android_log_print(ANDROID_LOG_INFO, "bread", __VA_ARGS__)
#else
#  include <stdio.h>
#  define _eth_log(...) printf(__VA_ARGS__)
#endif

#endif // BR_Util_H
