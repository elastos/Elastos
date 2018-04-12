// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Log.h"

namespace Elastos {
	namespace SDK {

		std::shared_ptr<spdlog::logger> Log::_consoleLog = spdlog::stdout_color_mt("console");

		const std::shared_ptr<spdlog::logger> &Log::getLogger() const {
			return _consoleLog;
		}

	}
}