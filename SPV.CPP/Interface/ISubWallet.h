// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ISUBWALLET_H__
#define __ELASTOS_SDK_ISUBWALLET_H__

#include <string>

#include <nlohmann/json.hpp>

#include "ISubWalletCallback.h"

namespace Elastos {
	namespace ElaWallet {

		class ISubWallet {
		public:
			/**
			 * Virtual destructor.
			 */
			virtual ~ISubWallet() noexcept {}

			/**
			 * Get the sub wallet chain id.
			 * @return sub wallet chain id.
			 */
			virtual std::string GetChainId() const = 0;

			/**
			 * Get balances of all addresses in json format.
			 * @return balances of all addresses in json format.
			 */
			virtual nlohmann::json GetBalanceInfo() = 0;

			/**
			 * Get sum of balances of all addresses.
			 * @return sum of balances。
			 */
			virtual uint64_t GetBalance() = 0;

			/**
			 * Create a new address or return existing unused address. Note that if create the sub wallet by setting the singleAddress to true, will always return the single address.
			 * @return a new address or existing unused address.
			 */
			virtual std::string CreateAddress() = 0;

			virtual nlohmann::json GetAllAddress(
					uint32_t start,
					uint32_t count) = 0;

			virtual uint64_t GetBalanceWithAddress(const std::string &address) = 0;

			virtual void AddCallback(ISubWalletCallback *subCallback) = 0;

			virtual void RemoveCallback(ISubWalletCallback *subCallback) = 0;

			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					uint64_t fee,
					const std::string &memo,
					const std::string &remark) = 0;

			virtual std::string CreateMultiSignAddress(
					const nlohmann::json &multiPublicKeyJson,
					uint32_t totalSignNum,
					uint32_t requiredSignNum) = 0;

			virtual nlohmann::json CreateMultiSignTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					uint64_t fee,
					const std::string &memo) = 0;

			virtual nlohmann::json SendRawTransaction(
					const nlohmann::json &transactionJson,
					uint64_t fee,
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

			virtual uint64_t CalculateTransactionFee(
					const nlohmann::json &rawTransaction,
					uint64_t feePerKb
					) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLET_H__
