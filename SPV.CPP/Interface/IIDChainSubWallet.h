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
			 * Get all Resolved DID list of current subwallet.
			 * @param start specify start index of all did list.
			 * @param count specify count of did we need.
			 * @param did filter word, if empty all did list shall be qualified.
			 * @return all did list of resolved in json format.
			 * example:
			 * params did is not empty
			 * {"DID":[{"expires":1575104460,"id":"innnNZJLqmJ8uKfVHKFxhdqVtvipNHzmZs","issuanceDate":1572516335,"operation":"update","publicKey":[{"id":"#primary","publicKey":"031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4"}],"status":"Confirmed"}],"MaxCount":1}
			 * or params did is empty
			 * {"DID":[{"expires":"1575104460","id":"iZFrhZLetd6i6qPu2MsYvE2aKrgw7Af4Ww","didName":"testname","operation":"create","issuanceDate":1575104460,status:"Pending"},{"expires":"1575104460","id":"ifUQ59wFpHUKe5NZ6gjffx48sWEBt9YgQE","didName":"testname","operation":"create","issuanceDate":1575104460,status:"Confirmed"}],"MaxCount":2}
			 */
			virtual nlohmann::json GetResolveDIDInfo(uint32_t start, uint32_t count, const std::string &did) const = 0;

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

			virtual nlohmann::json GetAllCID(uint32_t start, uint32_t count) const = 0;

			/**
			 * Sign message with private key of did.
			 * @param DIDOrCID will sign the message with public key of this did/cid.
			 * @param message to be signed.
			 * @param payPassword pay password.
			 * @return If success, signature will be returned.
			 */
			virtual std::string Sign(
				const std::string &DIDOrCID,
				const std::string &message,
				const std::string &payPassword) const = 0;

			/**
			 * Sign message with private key of did.
			 * @param DIDOrCID will sign the message with public key of this did/cid.
			 * @param digest hex string of sha256
			 * @param payPassword pay password.
			 * @return If success, signature will be returned.
			 */
			virtual std::string SignDigest(
					const std::string &DIDOrCID,
					const std::string &digest,
					const std::string &payPassword) const = 0;

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

			/**
			 * Generate payload for operation the did.
			 * @param inputInfo to generate DIDInfoPayload json fomat,able used to CreateIDTransaction. Content such as
			 * {
				"id": "innnNZJLqmJ8uKfVHKFxhdqVtvipNHzmZs",
			 	"didName":"testName",
				"operation":"create",
				 "publicKey": [{
				  "id": "#primary",
				  "publicKey":
				 "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4"
				  }, {
				 "id": "#recovery",
				 "controller": "ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh",
				 "publicKey":
				 "03d25d582c485856520c501b2e2f92934eda0232ded70cad9e51cf13968cac22cc"
				 }],
				"expires":1575104460
			   }
			 * @return The payload in JSON format.
			 */
			virtual nlohmann::json GenerateDIDInfoPayload(
				const nlohmann::json &didInfo,
				const std::string &paypasswd) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IIDCHAINSUBWALLET_H__
