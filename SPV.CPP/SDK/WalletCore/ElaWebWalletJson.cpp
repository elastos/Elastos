// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ElaWebWalletJson.h"

namespace Elastos {
	namespace ElaWallet {

		ElaWebWalletJson::ElaWebWalletJson() {

		}

		ElaWebWalletJson::~ElaWebWalletJson() {
			_mnemonic.resize(_mnemonic.size(), 0);
		}

		nlohmann::json ElaWebWalletJson::ToJson(bool withPrivKey) const {
			nlohmann::json j = BitcoreWalletClientJson::ToJson(withPrivKey);

			if (withPrivKey)
				j["mnemonic"] = _mnemonic;

			return j;
		}

		void ElaWebWalletJson::FromJson(const nlohmann::json &j) {
			BitcoreWalletClientJson::FromJson(j);

			if (j.find("mnemonic") != j.end())
				_mnemonic = j["mnemonic"].get<std::string>();
		}

	}
}
