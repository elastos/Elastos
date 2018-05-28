// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IdChainSubWallet.h"

namespace Elastos {
	namespace SDK {

		IdChainSubWallet::IdChainSubWallet(const CoinInfo &info, const ChainParams &chainParams,
										   const std::string &payPassword, MasterWallet *parent) :
				SubWallet(info, chainParams, payPassword, parent) {

		}

		IdChainSubWallet::~IdChainSubWallet() {

		}

		nlohmann::json IdChainSubWallet::GenerateId(std::string &id, std::string &privateKey) {
			//todo generate random key
			Key key;
			return nlohmann::json();
		}

		std::string IdChainSubWallet::getIdValue(const std::string &path) {
			//todo complete me
			return "";
		}

		std::string
		IdChainSubWallet::SendDepositTransaction(const std::string &fromAddress, const std::string &toAddress,
												 const nlohmann::json &payloadJson, const nlohmann::json &programJson,
												 double fee, const std::string &payPassword, const std::string &memo) {
			//todo complete me
			return "";
		}
	}
}