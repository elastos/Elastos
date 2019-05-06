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

		enum BalanceType {
			Default,
			Voted,
			Total,
		};

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
			 * Here is a example of hd account wallet basic info:
			 * {
			 * 	"Type": "Normal" //Type can be Normal, Mainchain, Sidechain and Idchain
			 * 	"Account":
			 * 		{
			 * 			"Type": "HD Account"
			 * 			"Details":
			 * 				{
			 * 					"CoinIndex": 1
			 * 				}
			 * 		}
			 * }
			 *
			 * and an example of multi-sign account wallet basic info:
			 * {
			 * 	"Type": "Mainchain" //Type can be Normal, Mainchain, Sidechain and Idchain
			 * 	"Account":
			 * 		{
			 * 			"Type": "Multi-Sign Account"
			 * 		}
			 * }
			 * @return basic information of current master wallet.
			 */
			virtual nlohmann::json GetBasicInfo() const = 0;

			/**
			 * Get balances of all addresses in json format.
			 * @return balances of all addresses in json format.
			 */
			virtual nlohmann::json GetBalanceInfo() const = 0;

			/**
			 * Get sum of balances of all addresses according to balance type.
			 * @return sum of balances.
			 */
			virtual uint64_t GetBalance(BalanceType type = Default) const = 0;

			/**
			 * Get balance of only the specified address.
			 * @param address is one of addresses created by current sub wallet.
			 * @return balance of specified address.
			 */
			virtual uint64_t GetBalanceWithAddress(const std::string &address, BalanceType type = Default) const = 0;

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
					uint32_t count) const = 0;

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
			 * @param memo input memo attribute for describing.
			 * @param remark is used to record message of local wallet.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					const std::string &memo,
					const std::string &remark,
					bool useVotedUTXO = false) = 0;

			/**
			 * Create a multi-sign transaction and return the content of transaction in json format.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param toAddress specify which address we want to send.
			 * @param amount specify amount we want to send.
			 * @param memo input memo attribute for describing.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateMultiSignTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					const std::string &memo,
					const std::string &remark,
					bool useVotedUTXO = false) = 0;

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
			 * Update a transaction by change fee
			 * @param transactionJson content of transaction in json format.
			 * @param fee specify fee for miners, fee must greater or equal than 1000 (sela).
			 * @return Sent result in json format.
			 */
			virtual nlohmann::json UpdateTransactionFee(
					const nlohmann::json &transactionJson,
					uint64_t fee,
					const std::string &fromAddress) = 0;

			/**
			 * Sign a transaction or append sign to a multi-sign transaction and return the content of transaction in json format.
			 * @param rawTransaction content of transaction in json format.
			 * @param payPassword use to decrypt the root private key temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json SignTransaction(
					const nlohmann::json &rawTransaction,
					const std::string &payPassword) = 0;

			/**
			 * Get signers already signed specified transaction.
			 * @param rawTransaction a multi-sign transaction to find signed signers.
			 * @return Signed signers in json format. An example of result will be displayed as follows:
			 *
			 * [{"M":3,"N":4,"SignType":"MultiSign","Signers":["02753416fc7c1fb43c91e29622e378cd16243b53577ec971c6c3624a775722491a","0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280","030f5bdbee5e62f035f19153c5c32966e0fc72e419c2b4867ba533c43340c86b78"]}]
			 * or
			 * [{"SignType":"Standard","Signers":["028e0ce09c7a5905f876f38473d4e1e0a85327122372e5db14fc72f88311c30e75"]}]
			 *
			 */
			virtual nlohmann::json GetTransactionSignedSigners(
					const nlohmann::json &rawTransaction) const = 0;

			/**
			 * Send a transaction by p2p network.
			 * @param rawTransaction content of transaction in json format.
			 * @param fee specify fee for miners, fee must greater or equal than 1000 (sela).
			 * @return Sent result in json format.
			 */
			virtual nlohmann::json PublishTransaction(
					const nlohmann::json &rawTransaction) = 0;

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
					const std::string &addressOrTxid) const = 0;

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
			 * @return true or false.
			 */
			virtual bool CheckSign(
					const std::string &publicKey,
					const std::string &message,
					const std::string &signature) = 0;

			/**
			 * Get an asset details by specified asset ID
			 * @param assetID asset hex code from asset hash.
			 * @return details about asset in json format.
			 */
			virtual nlohmann::json GetAssetDetails(
					const std::string &assetID) const = 0;

			/**
			 * Get root public key of current sub wallet.
			 * @return root public key with hex string format.
			 */
			virtual std::string GetPublicKey() const = 0;
		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLET_H__
