// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ERRORCODE_H__
#define __ELASTOS_SDK_ERRORCODE_H__

#include <string>

namespace Elastos {
	namespace ElaWallet {

		class ErrorCode {
		public:
			static void StandardLogicError(const std::string &errorCode, const std::string &extraMsg);

			static std::string PasswordError;
			static std::string WrongAccountType;
			static std::string WrongSubAccountType;
			static std::string NoCurrentMultiSinAccount;
			static std::string TransactionContentError;
		};

	}
}

#endif //__ELASTOS_SDK_ERRORCODE_H__
