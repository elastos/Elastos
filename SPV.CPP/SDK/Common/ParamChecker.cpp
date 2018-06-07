// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdexcept>

#include "ParamChecker.h"
#include "Config.h"

namespace Elastos {
	namespace SDK {

		void ParamChecker::checkPassword(const std::string &password, bool isParam) {
			if (password.size() < MIN_PASSWORD_LENGTH || password.size() > MAX_PASSWORD_LENGTH) {
				if (isParam)
					throw std::invalid_argument("Password should between 8 and 128.");
				else
					throw std::logic_error("Password should between 8 and 128.");
			}
		}

		void ParamChecker::checkPasswordWithNullLegal(const std::string &password, bool isParam) {
			if (password.empty())
				return;
			checkPassword(password, isParam);
		}

		void ParamChecker::checkNotEmpty(const std::string &message, bool isParam) {
			if (message.empty()) {
				if (isParam)
					throw std::invalid_argument("Message should not be empty.");
				else
					throw std::logic_error("Message should not be empty.");
			}
		}

		void ParamChecker::checkDataNotEmpty(const CMBlock &mem, bool isParam) {
			if (mem.GetSize() == 0) {
				if (isParam)
					throw std::invalid_argument("Data should not be empty.");
				else
					throw std::logic_error("Data should not be empty.");
			}
		}

		void ParamChecker::checkJsonArrayNotEmpty(nlohmann::json jsonData, bool isParam) {
			if (!jsonData.is_array() || jsonData.size() <= 0) {
				if (isParam)
					throw std::invalid_argument("json should not be empty.");
				else
					throw std::logic_error("json should not be empty.");
			}
		}
	}
}