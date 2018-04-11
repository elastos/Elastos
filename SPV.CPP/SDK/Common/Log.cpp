// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Log.h"

namespace Elastos {
	namespace SDK {

		std::shared_ptr<spdlog::logger> Log::_consoleLog = spdlog::stdout_color_mt("console");

		template<typename T>
		void Log::log(spdlog::level::level_enum lvl, const T &msg) {
			_consoleLog->log(lvl, msg);
		}

		template<typename T>
		void Log::trace(const T &msg) {
			_consoleLog->trace(msg);
		}

		template<typename T>
		void Log::debug(const T &msg) {
			_consoleLog->debug(msg);
		}

		template<typename T>
		void Log::info(const T &msg) {
			_consoleLog->info(msg);
		}

		template<typename T>
		void Log::warn(const T &msg) {
			_consoleLog->warn(msg);
		}

		template<typename T>
		void Log::error(const T &msg) {
			_consoleLog->error(msg);
		}

		template<typename T>
		void Log::critical(const T &msg) {
			_consoleLog->critical(msg);
		}

		template<typename... Args>
		void Log::log(spdlog::level::level_enum lvl, const char *msg) {
			_consoleLog->log(lvl, msg);
		}

		const std::shared_ptr<spdlog::logger> &Log::getLogger() const {
			return _consoleLog;
		}

	}
}