// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ACCOUNTFACTORY_H__
#define __ELASTOS_SDK_ACCOUNTFACTORY_H__

#include <nlohmann/json.hpp>

#include "IAccount.h"

namespace Elastos {
	namespace ElaWallet {

		class AccountFactory {
		public:
			static IAccount *CreateFromJson(const std::string &rootPath, const nlohmann::json &j);
		};

	}
}

#endif //__ELASTOS_SDK_ACCOUNTFACTORY_H__
