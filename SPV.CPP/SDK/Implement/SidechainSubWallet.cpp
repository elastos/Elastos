// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SidechainSubWallet.h"

namespace Elastos {
	namespace SDK {

		SidechainSubWallet::SidechainSubWallet(const CoinInfo &info, const ChainParams &chainParams,
											   const std::string &payPassword, MasterWallet *parent) :
				SubWallet(info, chainParams, payPassword, parent) {

		}

		SidechainSubWallet::~SidechainSubWallet() {

		}

		std::string SidechainSubWallet::SendWithdrawTransaction(const std::string &fromAddress,
																const nlohmann::json &mainchainAccounts,
																const nlohmann::json &mainchainAmounts, double fee,
																const std::string &payPassword,
																const std::string &memo) {
			//todo complete me
			return "";
		}
	}
}