// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ISUBWALLET_H__
#define __ELASTOS_SDK_ISUBWALLET_H__

#include <nlohmann/json.hpp>

#include "ISubWalletCallback.h"

namespace Elastos {
	namespace SDK {

		class ISubWallet {
		public:
			virtual nlohmann::json GetBalanceInfo() = 0;

			virtual double GetBalance() = 0;

			virtual std::string CreateAddress() = 0;

			virtual std::string GetTheLastAddress() = 0;

			virtual nlohmann::json GetAllAddress() = 0;

			virtual double GetBalanceWithAddress(const std::string &address) = 0;

			virtual void AddCallback(ISubWalletCallback *subCallback) = 0;

			virtual void RemoveCallback(ISubWalletCallback *subCallback) = 0;

			virtual std::string SendTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					double amount,
					double fee,
					const std::string &payPassword,
					const std::string &memo,
					const std::string &txid) = 0;

			virtual std::string SendRawTransaction(
					const nlohmann::json &transactionJson,
					const std::string &payPassword) = 0;

			virtual nlohmann::json GetAllTransaction(
					uint32_t start,
					uint32_t count,
					const std::string &addressOrTxid) = 0;

			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword) = 0;

			virtual nlohmann::json CheckSign(
					const std::string &address,
					const std::string &message,
					const std::string &signature) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLET_H__
