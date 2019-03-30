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

		void to_json(nlohmann::json &j, const ElaWebWalletJson &p, bool withPrivKey) {
			to_json(j, dynamic_cast<const BitcoreWalletClientJson &>(p), withPrivKey);
			if (withPrivKey && !p._mnemonic.empty())
				j["mnemonic"] = p._mnemonic;
		}

		void from_json(const nlohmann::json &j, ElaWebWalletJson &p) {
			from_json(j, dynamic_cast<BitcoreWalletClientJson &>(p));
			if (j.find("mnemonic") != j.end())
				p._mnemonic = j["mnemonic"];
		}
	}
}