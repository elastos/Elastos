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

#ifndef __ELASTOS_SDK_IIDCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IIDCHAINSUBWALLET_H__

#include "ISidechainSubWallet.h"

namespace Elastos {
	namespace ElaWallet {

		class IIDChainSubWallet : public virtual ISidechainSubWallet {
		public:

			/**
			 * Virtual destructor.
			 */
			virtual ~IIDChainSubWallet() noexcept {}

			/**
			 * Create a id transaction and return the content of transaction in json format, this is a special transaction to register id related information on id chain.
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
			 * @param payloadJson is payload for register id related information in json format, the content of payload should have Id, Path, DataHash, Proof, and Sign.
			 * @param memo input memo attribute for describing.
			 * @param fee transaction fee set by user.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateIDTransaction(
			        const nlohmann::json &inputs,
					const nlohmann::json &payloadJson,
					const std::string &memo = "",
                    const std::string &fee = "10000") = 0;

			/**
			 * Get all DID derived of current subwallet.
			 * @param index specify start index of all DID list.
			 * @param count specify count of DID we need.
			 * @param internal change address for true or normal external address for false.
			 * @return If success return all DID in JSON format.
			 *
			 * example:
			 * GetAllDID(0, 3) will return below
			 * {
			 *     "DID": ["iZDgaZZjRPGCE4x8id6YYJ158RxfTjTnCt", "iPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv", "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP"],
			 * }
			 */
			virtual nlohmann::json GetDID(uint32_t index, uint32_t count, bool internal = false) const = 0;

            /**
             * Get CID derived of current subwallet.
             * @param index specify start index of all CID list.
             * @param count specify count of CID we need.
             * @param internal change address for true or normal external address for false.
             * @return If success return CID in JSON format.
             *
             * example:
             * GetAllDID(0, 3) will return below
             * {
             *     "CID": ["iZDgaZZjRPGCE4x8id6YYJ158RxfTjTnCt", "iPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv", "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP"],
             * }
             */
			virtual nlohmann::json GetCID(uint32_t index, uint32_t count, bool internal = false) const = 0;

			/**
			 * Sign message with private key of did.
			 * @param DIDOrCID will sign the message with public key of this did/cid.
			 * @param message to be signed.
			 * @param passwd pay password.
			 * @return If success, signature will be returned.
			 */
			virtual std::string Sign(
				const std::string &DIDOrCID,
				const std::string &message,
				const std::string &passwd) const = 0;

			/**
			 * Verify signature with specify public key
			 * @param publicKey public key.
			 * @param message message to be verified.
			 * @param signature signature to be verified.
			 * @return true or false.
			 */
			virtual bool VerifySignature(
				const std::string &publicKey,
				const std::string &message,
				const std::string &signature) = 0;

			/**
			 * Get DID by public key
			 * @param pubkey public key
			 * @return did string
			 */
			virtual std::string GetPublicKeyDID(const std::string &pubkey) const = 0;

			/**
			 * Get CID by public key
			 * @param pubkey
			 * @return cid string
			 */
			virtual std::string GetPublicKeyCID(const std::string &pubkey) const = 0;

		};

	}
}

#endif //__ELASTOS_SDK_IIDCHAINSUBWALLET_H__
