/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ELASTOS_SDK_ISUBWALLET_H__
#define __ELASTOS_SDK_ISUBWALLET_H__

#include <string>

#include "nlohmann/json.hpp"

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
			virtual std::string GetChainID() const = 0;

			/**
			 * basic info of sub wallet
			 * @return basic information of current master wallet.
			 *
			 * Such as:
			 * {
			 *   "Info":{
			 *     "Account":{"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard", "HasPassPhrase": false},
			 *     "CoinIndex":0
			 *   },
			 *   "ChainID":"ELA"
			 * }
			 *
			 * {
			 *   "Info":{
			 *     "Account":{"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard", "HasPassPhrase": false},
			 *     "CoinIndex":1
			 *   },
			 *   "ChainID":"IDChain"
			 * }
			 *
			 * {
			 *   "Info":{
			 *     "Account":{"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard", "HasPassPhrase": false},
			 *     "CoinIndex":2
			 *   },
			 *   "ChainID":"TokenChain"
			 * }
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
			virtual std::string GetBalance() const = 0;

			/**
			 * Get balance of only the specified address.
			 * @param address is one of addresses created by current sub wallet.
			 * @return balance of specified address.
			 */
			virtual std::string GetBalanceWithAddress(const std::string &address) const = 0;

			/**
			 * Create a new address or return existing unused address. Note that if create the sub wallet by setting the singleAddress to true, will always return the single address.
			 * @return a new address or existing unused address.
			 */
			virtual std::string CreateAddress() = 0;

			/**
			 * Get all created addresses in json format. The parameters of start and count are used for purpose of paging.
			 * @param start specify start index of all addresses list.
			 * @param count specify count of addresses we need.
			 * @param internal indicate if addresses are change(internal) address or not.
			 * @return addresses in JSON format.
			 *
			 * example:
			 * {
			 *     "Addresses": ["EYMVuGs1FscpgmghSzg243R6PzPiszrgj7", "EJuHg2CdT9a9bqdKUAtbrAn6DGwXtKA6uh"],
			 *     "MaxCount": 100
			 * }
			 */
			virtual nlohmann::json GetAllAddress(
					uint32_t start,
					uint32_t count,
					bool internal = false) const = 0;

			/**
			 * Get all created public key list in JSON format. The parameters of start and count are used for the purpose of paging.
			 * @param start to specify start index of all public key list.
			 * @param count specifies the count of public keys we need.
			 * @return public keys in json format.
			 */
			virtual nlohmann::json GetAllPublicKeys(
					uint32_t start,
					uint32_t count) const = 0;

			/**
			 * Add a sub wallet callback object listened to current sub wallet.
			 * @param subCallback is a pointer who want to listen events of current sub wallet.
			 */
			virtual void AddCallback(ISubWalletCallback *subCallback) = 0;

			/**
			 * Remove a sub wallet callback object listened to current sub wallet.
			 */
			virtual void RemoveCallback() = 0;

			/**
			 * Create a normal transaction and return the content of transaction in json format.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param targetAddress specify which address we want to send.
			 * @param amount specify amount we want to send. "-1" means max.
			 * @param memo input memo attribute for describing.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &targetAddress,
					const std::string &amount,
					const std::string &memo) = 0;

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
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateConsolidateTransaction(
					const std::string &memo) = 0;

			/**
			 * Sign a transaction or append sign to a multi-sign transaction and return the content of transaction in json format.
			 * @param tx transaction created by Create*Transaction().
			 * @param payPassword use to decrypt the root private key temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json SignTransaction(
					const nlohmann::json &tx,
					const std::string &payPassword) const = 0;

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
			 * @param tx signed transaction.
			 * @return Sent result in json format.
			 */
			virtual nlohmann::json PublishTransaction(
					const nlohmann::json &tx) = 0;

			/**
			 * Convert tx to raw transaction.
			 * @param tx transaction json
			 * @return  tx in hex string format.
			 */
			virtual std::string ConvertToRawTransaction(const nlohmann::json &tx)  = 0;

			/**
			 * Get all qualified normal transactions sorted by descent (newest first).
			 * @param start specify start index of all transactions list.
			 * @param count specify count of transactions we need.
			 * @param txid transaction ID to be filtered.
			 * @return All qualified transactions in json format.
			 * {"MaxCount":3,"Transactions":[{"Amount":"20000","ConfirmStatus":"6+","Direction":"Received","Height":172570,"Status":"Confirmed","Timestamp":1557910458,"TxHash":"ff454532e57837cbe04f56a7e43f4209b5eb61d5d2a43a016a769c60d21125b6","Type":6},{"Amount":"10000","ConfirmStatus":"6+","Direction":"Received","Height":172569,"Status":"Confirmed","Timestamp":1557909659,"TxHash":"7253b2cefbac794b621b0080f0f5a4c27d5c91f65c83da75aad615062c42ac5a","Type":6},{"Amount":"100000","ConfirmStatus":"6+","Direction":"Received","Height":172300,"Status":"Confirmed","Timestamp":1557809019,"TxHash":"7e53bb8fe1617bdb57f7346bcf7d2e9dfa6b5d3f3524d0695046389bea79dcd9","Type":6}]}
			 */
			virtual nlohmann::json GetAllTransaction(
					uint32_t start,
					uint32_t count,
					const std::string &txid) const = 0;

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
			 * Get an asset details by specified asset ID
			 * @param assetID asset hex code from asset hash.
			 * @return asset info in json format.
			 */
			virtual nlohmann::json GetAssetInfo(
					const std::string &assetID) const = 0;

			/**
			 * Use fixed peer to sync
			 * @param address IP or domain name.
			 * @param port p2p port.
			 * @return return true if success, otherwise false.
			 */
			virtual bool SetFixedPeer(const std::string &address, uint16_t port) = 0;

			/**
			 * Start sync of P2P network
			 */
			virtual void SyncStart() = 0;

			/**
			 * Stop sync of P2P network
			 */
			virtual void SyncStop() = 0;

			/**
			 * Will delete all Merkle blocks and all transactions except the private key.
			 * And then resync from the beginning.
			 */
			virtual void Resync() = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLET_H__
