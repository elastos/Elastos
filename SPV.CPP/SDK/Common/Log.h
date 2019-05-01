// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_LOG_H__
#define __ELASTOS_SDK_LOG_H__

#include <Config.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/file_sinks.h>
#if defined(__ANDROID__)
#include <spdlog/sinks/android_sink.h>
#endif

namespace Elastos {
	namespace ElaWallet {

		class Log {
		public:

#define SPV_LOG_NAME "spvsdk_multi"
#define SPV_DEFAULT_LOG SPV_LOG_NAME

#define SPV_FILE_NAME "spvsdk.log"

			static inline spdlog::sink_ptr registerLogger() {
#if defined(__ANDROID__)
				auto console_sink = std::make_shared<spdlog::sinks::android_sink>("spvsdk");
#else
				auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif
				console_sink->set_level(spdlog::level::trace);
				return console_sink;
			}

			static inline void registerMultiLogger(const std::string &path) {
				auto console_sink = registerLogger();

				auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path + "/" + SPV_FILE_NAME, 1024*1024*1, 1);
				file_sink->set_level(spdlog::level::info);
				std::vector<spdlog::sink_ptr> sinks = {console_sink, file_sink};
				auto logger = std::make_shared<spdlog::logger>(SPV_DEFAULT_LOG, sinks.begin(), sinks.end());
				spdlog::register_logger(logger);

#if defined(__ANDROID__)
				spdlog::get(SPV_DEFAULT_LOG)->set_pattern("%v");
#else
				spdlog::get(SPV_DEFAULT_LOG)->set_pattern("%m-%d %T.%e %P %t %^%L%$ %n %v");
#endif
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

		};

#define SPVLOG_DEBUG(...) SPDLOG_DEBUG(spdlog::get(SPV_DEFAULT_LOG), __VA_ARGS__)

	}
}

#endif //__ELASTOS_SDK_LOG_H__
