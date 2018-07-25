// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <stdexcept>

#include "ParamChecker.h"
#include "CommonConfig.h"
#include "BRBIP39Mnemonic.h"

namespace Elastos {
	namespace ElaWallet {

		void ParamChecker::checkNullPointer(void *pointer, bool isParam) {
			if (pointer == nullptr) {
				if (isParam)
					throw std::invalid_argument("Pointer should not be null.");
				else
					throw std::logic_error("Pointer should not be null.");
			}
		}

		void ParamChecker::checkPassword(const std::string &password, const std::string &passName, bool isParam) {
			if (password.size() < MIN_PASSWORD_LENGTH || password.size() > MAX_PASSWORD_LENGTH) {
				std::stringstream ss;
				ss << passName << " " << "Password should between 8 and 128.";
				if (isParam)
					throw std::invalid_argument(ss.str());
				else
					throw std::logic_error(ss.str());
			}
		}

		void ParamChecker::checkPasswordWithNullLegal(const std::string &password, const std::string &passName, bool isParam) {
			if (password.empty())
				return;
			checkPassword(password, passName, isParam);
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
					throw std::invalid_argument("Json should not be empty.");
				else
					throw std::logic_error("Json should not be empty.");
			}
		}

		void ParamChecker::checkPathExists(const boost::filesystem::path &path, bool isParam) {
			if (!boost::filesystem::exists(path)) {
				if (isParam)
					throw std::invalid_argument("Path should valid.");
				else
					throw std::logic_error("Path should valid.");
			}
		}

		//check language words count is BIP39_WORDLIST_COUNT
		void ParamChecker::checkLangWordsCnt(const uint32_t cnt) {
			if (cnt != BIP39_WORDLIST_COUNT)
				throw std::invalid_argument("Language words count invalid.");
		}
	}
}