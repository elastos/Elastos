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
			 * Get last 10 external addresses or 5 internal addresses
			 * @internal indicate if addresses are change(internal) address or not
			 * @return
			 */
			virtual std::vector<std::string> GetLastAddresses(bool internal) const = 0;

			/**
			 * @param usedAddresses
			 * @return none
			 */
			virtual void UpdateUsedAddress(const std::vector<std::string> &usedAddresses) const = 0;

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
			 * Create a normal transaction and return the content of transaction in json format.
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param outputs Outputs which we want to send to. If there is change, a new output will be append. eg
			 * [
			 *   {
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo input memo attribute for describing.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateTransaction(
					const nlohmann::json &inputs,
					const nlohmann::json &outputs,
					const std::string &fee,
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
			 * Convert tx to raw transaction.
			 * @param tx transaction json
			 * @return  tx in hex string format.
			 */
			virtual std::string ConvertToRawTransaction(const nlohmann::json &tx)  = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLET_H__
