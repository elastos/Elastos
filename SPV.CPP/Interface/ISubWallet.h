// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ISUBWALLET_H__
#define __ELASTOS_SDK_ISUBWALLET_H__

#include <string>

#include "nlohmann/json.hpp"

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
			virtual std::string GetChainID() const = 0;

			/**
			 * Here is a example of hd account wallet basic info:
			 * {
			 *   "Info":{
			 *     "Account":{"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard"},
			 *     "CoinIndex":0
			 *   },
			 *   "ChainID":"ELA"
			 * }
			 *
			 * {
			 *   "Info":{
			 *     "Account":{"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard"},
			 *     "CoinIndex":1
			 *   },
			 *   "ChainID":"IDChain"
			 * }
			 *
			 * {
			 *   "Info":{
			 *     "Account":{"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard"},
			 *     "CoinIndex":2
			 *   },
			 *   "ChainID":"TokenChain"
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
			virtual std::string GetBalance(BalanceType type = Default) const = 0;

			/**
			 * Get balance of only the specified address.
			 * @param address is one of addresses created by current sub wallet.
			 * @return balance of specified address.
			 */
			virtual std::string GetBalanceWithAddress(const std::string &address, BalanceType type = Default) const = 0;

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
			 * @param useVotedUTXO If true, all voted UTXO will be picked. Otherwise, any voted UTXO will not be picked.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					const std::string &amount,
					const std::string &memo,
					bool useVotedUTXO = false) = 0;

			/**
			 * Get all UTXO list. Include locked and pending and deposit utxos.
			 * @param start specify start index of all utxos list.
			 * @param count specify count of utxos we need.
			 * @param address to filter the specify address's utxos. If empty, all utxo of all addresses wil be returned.
			 * @return return all utxo in json format
			 */
			virtual nlohmann::json GetAllUTXOs(
				uint32_t start,
				uint32_t count,
				const std::string &address) const = 0;

			/**
			 * Create a transaction to combine as many UTXOs as possible until transaction size reaches the max size.
			 * @param memo input memo attribute for describing.
			 * @param useVotedUTXO If true, all voted UTXO will be picked. Otherwise, any voted UTXO will not be picked.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateConsolidateTransaction(
					const std::string &memo,
					bool useVotedUTXO = false) = 0;

			/**
			 * Sign a transaction or append sign to a multi-sign transaction and return the content of transaction in json format.
			 * @param createdTx content of transaction in json format.
			 * @param payPassword use to decrypt the root private key temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json SignTransaction(
					const nlohmann::json &createdTx,
					const std::string &payPassword) = 0;

			/**
			 * Get signers already signed specified transaction.
			 * @param tx a signed transaction to find signed signers.
			 * @return Signed signers in json format. An example of result will be displayed as follows:
			 *
			 * [{"M":3,"N":4,"SignType":"MultiSign","Signers":["02753416fc7c1fb43c91e29622e378cd16243b53577ec971c6c3624a775722491a","0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280","030f5bdbee5e62f035f19153c5c32966e0fc72e419c2b4867ba533c43340c86b78"]}]
			 * or
			 * [{"SignType":"Standard","Signers":["0207d8bc14c4bdd79ea4a30818455f705bcc9e17a4b843a5f8f4a95aa21fb03d77"]},{"SignType":"Standard","Signers":["02a58d1c4e4993572caf0133ece4486533261e0e44fb9054b1ea7a19842c35300e"]}]
			 *
			 */
			virtual nlohmann::json GetTransactionSignedInfo(
					const nlohmann::json &tx) const = 0;

			/**
			 * Publish a transaction to p2p network.
			 * @param signedTx content of transaction in json format.
			 * @return Sent result in json format.
			 */
			virtual nlohmann::json PublishTransaction(
					const nlohmann::json &signedTx) = 0;

			/**
			 * Get all qualified normal transactions sorted by descent (newest first).
			 * @param start specify start index of all transactions list.
			 * @param count specify count of transactions we need.
			 * @param addressOrTxid filter word which can be an address or a transaction id, if empty all transactions shall be qualified.
			 * @return All qualified transactions in json format.
			 * {"MaxCount":3,"Transactions":[{"Amount":"20000","ConfirmStatus":"6+","Direction":"Received","Height":172570,"Status":"Confirmed","Timestamp":1557910458,"TxHash":"ff454532e57837cbe04f56a7e43f4209b5eb61d5d2a43a016a769c60d21125b6","Type":6},{"Amount":"10000","ConfirmStatus":"6+","Direction":"Received","Height":172569,"Status":"Confirmed","Timestamp":1557909659,"TxHash":"7253b2cefbac794b621b0080f0f5a4c27d5c91f65c83da75aad615062c42ac5a","Type":6},{"Amount":"100000","ConfirmStatus":"6+","Direction":"Received","Height":172300,"Status":"Confirmed","Timestamp":1557809019,"TxHash":"7e53bb8fe1617bdb57f7346bcf7d2e9dfa6b5d3f3524d0695046389bea79dcd9","Type":6}]}
			 */
			virtual nlohmann::json GetAllTransaction(
					uint32_t start,
					uint32_t count,
					const std::string &addressOrTxid) const = 0;

			/**
			 * Get all coinbase transactions sorted by descent (newest first).
			 * @param start specify start index of all transactions list.
			 * @param count specify count of transactions we need.
			 * @param txID the filter can be transaction ID or empty. If empty, all transactions shall be qualified.
			 * @return transaction[s] in json format.
			 */
			virtual nlohmann::json GetAllCoinBaseTransaction(
				uint32_t start,
				uint32_t count,
				const std::string &txID) const = 0;

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
			 * @return asset info in json format.
			 */
			virtual nlohmann::json GetAssetInfo(
					const std::string &assetID) const = 0;

			/**
			 * Get public key ring of yourself publickey ring
			 * {"xPubKey":***,"requestPubKey":***"}
			 * @return PublicKeyRing of json format
			 */
			virtual nlohmann::json GetOwnerPublicKeyRing() const = 0;

			/**
			 * Start sync of P2P network
			 */
			virtual void SyncStart() = 0;

			/**
			 * Stop sync of P2P network
			 */
			virtual void SyncStop() = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLET_H__
