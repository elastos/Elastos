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
			 * For Elastos-based or btc wallet: Derivate @count addresses from @index.  Note that if create the
			 * sub-wallet by setting the singleAddress to true, will always set @index to 0, set @count to 1,
			 * set @internal to false.
			 * For ETH-based sidechain: Only return a single address. Ignore all parameters.
			 *
			 * @index start from 0.
			 * @count count of addresses we need.
			 * @internal change address for true or normal receive address for false.
			 * @return a new address or addresses as required.
			 */
			virtual nlohmann::json GetAddresses(uint32_t index, uint32_t count, bool internal = false) const = 0;

			/**
			 * For Elastos-based or btc wallet: Get @count public keys from @index.  Note that if create the
			 * sub-wallet by setting the singleAddress to true, will always set @index to 0, set @count to 1,
			 * set @internal to false.
			 * For ETH-based sidechain: Only return a single public key. Ignore all parameters.
			 *
			 * @param index to specify start index of all public key list.
			 * @param count specifies the count of public keys we need.
			 * @param internal change address for true or normal receive address for false.
			 * @return public keys in json format.
			 */
			virtual nlohmann::json GetPublicKeys(uint32_t index, uint32_t count, bool internal = false) const = 0;

			/**
			 * Sign a transaction or append sign to a multi-sign transaction and return the content of transaction in json format.
			 * @param tx transaction created by Create*Transaction().
			 * @param passwd use to decrypt the root private key temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json SignTransaction(const nlohmann::json &tx, const std::string &passwd) const = 0;

            /**
             * Sign message with private key of did.
             * @address will sign the digest with private key of this address.
             * @digest hex string of sha256
             * @passwd pay password.
             * @return If success, signature will be returned.
             */
            virtual std::string SignDigest(const std::string &address,
                                           const std::string &digest,
                                           const std::string &passwd) const = 0;

            /**
             * Verify signature with specify public key
             * @pubkey public key hex string.
             * @digest hex string of sha256.
             * @signature signature to be verified.
             * @return true or false.
             */
            virtual bool VerifyDigest(const std::string &pubkey,
                                      const std::string &digest,
                                      const std::string &signature) const = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLET_H__
