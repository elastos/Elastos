// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoinInfo.h"

#include <Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		CoinInfo::CoinInfo() :
				_chainID("") {

		}

		const std::string &CoinInfo::GetChainID() const {
			return _chainID;
		}

		void CoinInfo::SetChainID(const std::string &id) {
			_chainID = id;
		}

		nlohmann::json CoinInfo::ToJson() const {
			nlohmann::json j;
			j["ChainID"] = _chainID;

			return j;
		}

		void CoinInfo::FromJson(const nlohmann::json &j) {
			_chainID = j["ChainID"].get<std::string>();
			if (_chainID == "IdChain")
				_chainID = "IDChain";
		}

	}
}
