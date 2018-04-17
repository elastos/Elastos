// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_LOG_H__
#define __ELASTOS_SDK_LOG_H__

#include <spdlog/spdlog.h>

namespace Elastos {
	namespace SDK {

		class Log {
		public:

			template<typename T>
			static void log(spdlog::level::level_enum lvl, const T &msg){
			_consoleLog->log(lvl, msg);
		}
			template<typename T>
			static void trace(const T &msg){
				_consoleLog->trace(msg);
			}

			template<typename T>
			static void debug(const T &msg){
				_consoleLog->debug(msg);
			}

			template<typename T>
			static void info(const T &msg) {
				_consoleLog->info(msg);
			}

			template<typename T>
			static void warn(const T &msg){
				_consoleLog->warn(msg);
			}

			template<typename T>
			static void error(const T &msg){
				_consoleLog->error(msg);
			}

			template<typename T>
			static void critical(const T &msg) {
				_consoleLog->critical(msg);
			}

			template<typename... Args>
			static void log(spdlog::level::level_enum lvl, const char *msg){
				_consoleLog->log(lvl, msg);
			}

			static const std::shared_ptr<spdlog::logger>& getLogger();


		private:
			static std::shared_ptr<spdlog::logger> _consoleLog;
		};

	}
}

#endif //__ELASTOS_SDK_LOG_H__
