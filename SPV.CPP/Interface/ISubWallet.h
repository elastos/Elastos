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
			 * @return sum of balances.
			 */
			virtual uint64_t GetBalance() = 0;

			/**
			 * Create a new address or return existing unused address. Note that if create the sub wallet by setting the singleAddress to true, will always return the single address.
			 * @return a new address or existing unused address.
			 */
			virtual std::string CreateAddress() = 0;

			/**
			 * Get all created addresses in json format. The parameters of start and count are used for purpose of paging.
			 * @param start specify start index of all addresses list.
			 * @param count specify count of addresses we need.
			 * @return addresses in json format.
			 */
			virtual nlohmann::json GetAllAddress(
					uint32_t start,
					uint32_t count) = 0;

			/**
			 * Get balance of only the specified address.
			 * @param address is one of addresses created by current sub wallet.
			 * @return balance of specified address.
			 */
			virtual uint64_t GetBalanceWithAddress(const std::string &address) = 0;

			/**
			 * Add a sub wallet callback object listened to current sub wallet.
			 * @param subCallback is a pointer who want to listen events of current sub wallet.
			 */
			virtual void AddCallback(ISubWalletCallback *subCallback) = 0;

			/**
			 * Remove a sub wallet callback object listened to current sub wallet.
			 * @param subCallback is a pointer who want to listen events of current sub wallet.
			 */
			virtual void RemoveCallback(ISubWalletCallback *subCallback) = 0;

			/**
			 * Create a normal transaction and return the content of transaction in json format.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param toAddress specify which address we want to send.
			 * @param amount specify amount we want to send.
			 * @param fee [Obsoleted] specify fee for miners.
			 * @param memo input memo attribute for describing.
			 * @param remark is used to record message of local wallet.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					uint64_t fee,
					const std::string &memo,
					const std::string &remark) = 0;

			/**
			 * Create a multi-sign address used to create multi-sign transaction.
			 * @param multiPublicKeyJson is a list of public keys in json format.
			 * @param totalSignNum specify total sign number (n).
			 * @param requiredSignNum specify required sign number (m).
			 * @return multi-sign address.
			 */
			virtual std::string CreateMultiSignAddress(
					const nlohmann::json &multiPublicKeyJson,
					uint32_t totalSignNum,
					uint32_t requiredSignNum) = 0;

			/**
			 * Create a multi-sign transaction and return the content of transaction in json format.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param toAddress specify which address we want to send.
			 * @param amount specify amount we want to send.
			 * @param fee [Obsoleted] specify fee for miners.
			 * @param memo input memo attribute for describing.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateMultiSignTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					uint64_t fee,
					const std::string &memo) = 0;

			/**
			 * Calculate transaction fee by content of transaction.
			 * @param rawTransaction content of transaction in json format.
			 * @param feePerKb specify the factor to calculate fee (transaction size * feePerKb).
			 * @return Calculate result of final fee.
			 */
			virtual uint64_t CalculateTransactionFee(
					const nlohmann::json &rawTransaction,
					uint64_t feePerKb) = 0;

			/**
			 * Send raw transaction by p2p network.
			 * @param transactionJson content of transaction in json format.
			 * @param fee specify fee for miners, fee must greater or equal than 1000 (sela).
			 * @param payPassword use to decrypt the root private key temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return Sent result in json format.
			 */
			virtual nlohmann::json SendRawTransaction(
					const nlohmann::json &transactionJson,
					uint64_t fee,
					const std::string &payPassword) = 0;

			/**
			 * Get all qualified transactions sorted by descent (newest first).
			 * @param start specify start index of all transactions list.
			 * @param count specify count of transactions we need.
			 * @param addressOrTxid filter word which can be an address or a transaction id, if empty all transactions shall be qualified.
			 * @return All qualified transactions in json format.
			 */
			virtual nlohmann::json GetAllTransaction(
					uint32_t start,
					uint32_t count,
					const std::string &addressOrTxid) = 0;

			/**
			 * Sign message through root private key of the master wallet.
			 * @param message need to signed, it should not be empty.
			 * @param payPassword use to decrypt the root private key temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return signed data of the message.
			 */
			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword) = 0;

			/**
			 * Verify signature by public key and raw message. This method can check signatures signed by any private keys not just the root private key of the master wallet.
			 * @param publicKey belong to the private key signed the signature.
			 * @param message raw data.
			 * @param signature signed data by a private key that correspond to the public key.
			 * @return the result wrapper by a json.
			 */
			virtual nlohmann::json CheckSign(
					const std::string &address,
					const std::string &message,
					const std::string &signature) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLET_H__
