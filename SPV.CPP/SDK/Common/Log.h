// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_LOG_H__
#define __ELASTOS_SDK_LOG_H__

#include <Config.h>
#include <spdlog/spdlog.h>

namespace Elastos {
	namespace ElaWallet {

		class Log {
		public:

			template<typename Arg1, typename... Args>
			static void trace(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				_consoleLog->trace(fmt.c_str(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			static void debug(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				_consoleLog->debug(fmt.c_str(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			static void info(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				_consoleLog->info(fmt.c_str(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			static void warn(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				_consoleLog->warn(fmt.c_str(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			static void error(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				_consoleLog->error(fmt.c_str(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			static void critical(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				_consoleLog->critical(fmt.c_str(), arg1, args...);
			}

			template<typename T>
			static void trace(const T &msg) {
				_consoleLog->trace(msg);
			}

			template<typename T>
			static void debug(const T &msg) {
				_consoleLog->debug(msg);
			}

			template<typename T>
			static void info(const T &msg) {
				_consoleLog->info(msg);
			}

			template<typename T>
			static void warn(const T &msg) {
				_consoleLog->warn(msg);
			}

			template<typename T>
			static void error(const T &msg) {
				_consoleLog->error(msg);
			}

			template<typename T>
			static void critical(const T &msg) {
				_consoleLog->critical(msg);
			}

			static void setLevel(spdlog::level::level_enum level) {
				_consoleLog->set_level(level);
			}

			static std::shared_ptr<spdlog::logger> &getLogger() {
				return _consoleLog;
			}

		private:
			static std::shared_ptr<spdlog::logger> _consoleLog;
		};

#define SPVLOG_DEBUG(...) SPDLOG_DEBUG(Log::getLogger(), __VA_ARGS__)

	}
}

#endif //__ELASTOS_SDK_LOG_H__
