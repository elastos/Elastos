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
#ifndef __ELASTOS_SDK_LOG_H__
#define __ELASTOS_SDK_LOG_H__

#include <CMakeConfig.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#if defined(__ANDROID__)
#include <spdlog/sinks/android_sink.h>
#endif

namespace Elastos {
	namespace ElaWallet {

		class Log {
		public:

#define SPV_LOG_NAME "spvsdk"
#define SPV_DEFAULT_LOG SPV_LOG_NAME

#define SPV_FILE_NAME "spvsdk.log"
#define GetFunName() (std::string("<<< ") + (__FUNCTION__) + " >>>")

			static inline void registerMultiLogger(const std::string &path = ".") {
				if (spdlog::get(SPV_DEFAULT_LOG) != nullptr)
					return ;

#ifdef SPV_CONSOLE_LOG
#if defined(__ANDROID__)
				auto console_sink = std::make_shared<spdlog::sinks::android_sink_mt>("spvsdk");
#else
				auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif
				console_sink->set_level(spdlog::level::trace);

				std::vector<spdlog::sink_ptr> sinks = {console_sink};
#else
				std::vector<spdlog::sink_ptr> sinks = {};
#endif

				std::string filepath = SPV_FILE_NAME;
				if (path != "") {
					filepath = path + "/" + SPV_FILE_NAME;
					auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filepath, 1024*1024*50, 1);
					file_sink->set_level(spdlog::level::debug);
					sinks.push_back(file_sink);
				}

				auto logger = std::make_shared<spdlog::logger>(SPV_DEFAULT_LOG, sinks.begin(), sinks.end());
				spdlog::register_logger(logger);

				spdlog::get(SPV_DEFAULT_LOG)->set_pattern("%m-%d %T.%e %P %t %^%L%$ %n %v");
				spdlog::get(SPV_DEFAULT_LOG)->flush_on(spdlog::level::debug);
			}

			template<typename Arg1, typename... Args>
			static inline void trace(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				spdlog::get(SPV_DEFAULT_LOG)->trace(fmt.c_str(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			static inline void debug(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				spdlog::get(SPV_DEFAULT_LOG)->debug(fmt.c_str(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			static inline void info(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				spdlog::get(SPV_DEFAULT_LOG)->info(fmt.c_str(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			static inline void warn(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				spdlog::get(SPV_DEFAULT_LOG)->warn(fmt.c_str(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			static inline void error(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				spdlog::get(SPV_DEFAULT_LOG)->error(fmt.c_str(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			static inline void critical(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				spdlog::get(SPV_DEFAULT_LOG)->critical(fmt.c_str(), arg1, args...);
			}

			template<typename T>
			static inline void trace(const T &msg) {
				spdlog::get(SPV_DEFAULT_LOG)->trace(msg);
			}

			template<typename T>
			static inline void debug(const T &msg) {
				spdlog::get(SPV_DEFAULT_LOG)->debug(msg);
			}

			template<typename T>
			static inline void info(const T &msg) {
				spdlog::get(SPV_DEFAULT_LOG)->info(msg);
			}

			template<typename T>
			static inline void warn(const T &msg) {
				spdlog::get(SPV_DEFAULT_LOG)->warn(msg);
			}

			template<typename T>
			static inline void error(const T &msg) {
				spdlog::get(SPV_DEFAULT_LOG)->error(msg);
			}

			template<typename T>
			static inline void critical(const T &msg) {
				spdlog::get(SPV_DEFAULT_LOG)->critical(msg);
			}

			static inline void setLevel(spdlog::level::level_enum level) {
				spdlog::get(SPV_DEFAULT_LOG)->set_level(level);
			}

			static inline void setPattern(const std::string &fmt) {
				spdlog::get(SPV_DEFAULT_LOG)->set_pattern(fmt);
			}

			static inline void flush() {
				spdlog::get(SPV_DEFAULT_LOG)->flush();
			}

		};

#define SPVLOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::get(SPV_DEFAULT_LOG), __VA_ARGS__)
#define SPVLOG_INFO(...)  SPDLOG_LOGGER_INFO(spdlog::get(SPV_DEFAULT_LOG), __VA_ARGS__)
#define SPVLOG_WARN(...)  SPDLOG_LOGGER_WARN(spdlog::get(SPV_DEFAULT_LOG), __VA_ARGS__)
#define SPVLOG_ERROR(...)  SPDLOG_LOGGER_ERROR(spdlog::get(SPV_DEFAULT_LOG), __VA_ARGS__)
#define SPVLOG_CRITICAL(...)  SPDLOG_LOGGER_CRITICAL(spdlog::get(SPV_DEFAULT_LOG), __VA_ARGS__)

#ifdef ARGUMENT_LOG_ENABLE
#define __va_first(first, ...) first
#define __va_rest(first, ...) __VA_ARGS__
#define ArgInfo(...) SPVLOG_INFO(__va_first(__VA_ARGS__, NULL), __va_rest(__VA_ARGS__, NULL))
#else
#define ArgInfo(...)
#endif

	}
}

#endif //__ELASTOS_SDK_LOG_H__
