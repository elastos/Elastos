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
			void log(spdlog::level::level_enum lvl, const T &);

			template<typename T>
			void trace(const T &msg);

			template<typename T>
			void debug(const T &msg);

			template<typename T>
			void info(const T &msg);

			template<typename T>
			void warn(const T &msg);

			template<typename T>
			void error(const T &msg);

			template<typename T>
			void critical(const T &msg);

			template<typename... Args>
			void log(spdlog::level::level_enum lvl, const char *msg);

			const std::shared_ptr<spdlog::logger>& getLogger() const;


		private:
			static std::shared_ptr<spdlog::logger> _consoleLog;
		};

	}
}

#endif //__ELASTOS_SDK_LOG_H__
