// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "AccountFactory.h"
#include "StandardAccount.h"
#include "SimpleAccount.h"
#include "MultiSignAccount.h"

namespace Elastos {
	namespace ElaWallet {

		IAccount *AccountFactory::CreateFromJson(const std::string &rootPath, const nlohmann::json &j) {

			std::string accountType = j["AccountType"].get<std::string>();

			IAccount *result = nullptr;
			if (accountType == "Standard") {
				result = new StandardAccount(rootPath);
			} else if (accountType == "Simple") {
				result = new SimpleAccount();
			} else if (accountType == "MultiSign") {
				result = new MultiSignAccount(rootPath);
			} else {
				throw std::logic_error("Invalid account type");
			}

			result->FromJson(j["Account"]);
			return result;
		}
	}
}
