// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MainchainSubWallet.h"

namespace Elastos {
	namespace SDK {

		MainchainSubWallet::MainchainSubWallet(const CoinInfo &info, const ChainParams &chainParams,
											   const std::string &payPassword, MasterWallet *parent) :
				SubWallet(info, chainParams, payPassword, parent) {

		}

		MainchainSubWallet::~MainchainSubWallet() {

		}

		std::string MainchainSubWallet::SendDepositTransaction(const std::string &fromAddress,
															   const nlohmann::json &sidechainAccounts,
															   const nlohmann::json &sidechainAmounts, double fee,
															   const std::string &payPassword,
															   const std::string &memo) {
			//todo complete me
			return "";
		}
	}
}