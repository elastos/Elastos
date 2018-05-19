// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ElaWebWalletJson.h"

namespace Elastos {
	namespace SDK {

		ElaWebWalletJson::ElaWebWalletJson() {

		}

		ElaWebWalletJson::~ElaWebWalletJson() {

		}

		const std::string &ElaWebWalletJson::getMnemonic() const {
			return _mnemonic;
		}

		void ElaWebWalletJson::setMnemonic(const std::string &value) {
			_mnemonic = value;
		}

		void to_json(nlohmann::json &j, const ElaWebWalletJson &p) {

		}

		void from_json(const nlohmann::json &j, ElaWebWalletJson &p) {

		}
	}
}