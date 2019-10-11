// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
			 * @param payloadJson is payload for register id related information in json format, the content of payload should have Id, Path, DataHash, Proof, and Sign.
			 * @param memo input memo attribute for describing.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateIDTransaction(
					const nlohmann::json &payloadJson,
					const std::string &memo = "") = 0;

			/**
			 * Get all DID derived of current subwallet.
			 * @param start specify start index of all DID list.
			 * @param count specify count of DID we need.
			 * @return If success return all DID in JSON format.
			 *
			 * example:
			 * GetAllDID(0, 3) will return below
			 * {
			 *     "DID": ["iZDgaZZjRPGCE4x8id6YYJ158RxfTjTnCt", "iPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv", "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP"],
			 *     "MaxCount": 100
			 * }
			 */
			virtual nlohmann::json GetAllDID(uint32_t start, uint32_t count) const = 0;

			/**
			 * Sign message with public key of did.
			 * @param did will sign the message with public key of this did.
			 * @param message to be signed.
			 * @param payPassword pay password.
			 * @return If success, signature will be returned.
			 */
			virtual std::string Sign(
				const std::string &did,
				const std::string &message,
				const std::string &payPassword) = 0;

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

		};

	}
}

#endif //__ELASTOS_SDK_IIDCHAINSUBWALLET_H__
