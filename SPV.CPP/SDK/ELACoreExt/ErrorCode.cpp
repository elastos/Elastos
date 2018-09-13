// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>

#include "ErrorCode.h"

namespace Elastos {
	namespace ElaWallet {

		std::string ErrorCode::PasswordError = "[Password]";
		std::string ErrorCode::WrongAccountType = "[WrongAccountType]";
		std::string ErrorCode::WrongSubAccountType = "[WrongSubAccountType]";
		std::string ErrorCode::NoCurrentMultiSinAccount = "[NoCurrentMultiSinAccount]";
		std::string ErrorCode::TransactionContentError = "TransactionContentError";

		void ErrorCode::StandardLogicError(const std::string &errorCode, const std::string &extraMsg) {
			std::stringstream ss;
			ss << "[" << errorCode << "] " << extraMsg;
			throw std::logic_error(ss.str());
		}

	}
}