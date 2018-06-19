// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ElaWebWalletJson.h"

namespace Elastos {
	namespace ElaWallet {

		ElaWebWalletJson::ElaWebWalletJson() {

		}

		ElaWebWalletJson::~ElaWebWalletJson() {

		}

		nlohmann::json &operator<<(nlohmann::json &j, const ElaWebWalletJson &p) {
			j << *(BitcoreWalletClientJson *) &p;

			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, ElaWebWalletJson &p) {
			j >> *(BitcoreWalletClientJson *) &p;

			from_json(j, p);

			return j;
		}

		void to_json(nlohmann::json &j, const ElaWebWalletJson &p) {
		}

		void from_json(const nlohmann::json &j, ElaWebWalletJson &p) {
		}
	}
}