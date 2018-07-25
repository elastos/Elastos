// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Log.h"

namespace Elastos {
	namespace ElaWallet {

#if defined(__ANDROID__)
		std::shared_ptr<spdlog::logger> Log::_consoleLog = spdlog::android_logger("Elastos", "spvsdk");
#else
		std::shared_ptr<spdlog::logger> Log::_consoleLog = spdlog::stdout_color_mt("console");
#endif
		Elastos::ElaWallet::SetLogLevel g_setLogLevel;

		const std::shared_ptr<spdlog::logger> &Log::getLogger() {
			return _consoleLog;
		}

	}
}
