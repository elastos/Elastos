// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_DIDINFO_H__
#define __ELASTOS_SDK_DIDINFO_H__

#include "IPayload.h"
namespace Elastos {
	namespace ElaWallet {

#define DID_DEFAULT_TYPE "ECDSAsecp256r1"

		class DIDHeaderInfo {
		public:
			DIDHeaderInfo();

			~DIDHeaderInfo();

			DIDHeaderInfo(const std::string &specification, const std::string &operation);

			const std::string &Specification() const;

			void SetSpecification(const std::string &specification);

			const std::string &Operation() const;

			void SetOperation(const std::string &operation);

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &stream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &stream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

		private:
			std::string _specification;
			std::string _operation;
		};

		class DIDPubKeyInfo {
		public:
			DIDPubKeyInfo();

			~DIDPubKeyInfo();

			DIDPubKeyInfo(const std::string &id, const std::string &pubkeyBase58,
							 const std::string &controller = "", const std::string &type = DID_DEFAULT_TYPE);

			const std::string &ID() const;

			void SetID(const std::string &id);

			const std::string &Type() const;

			void SetType(const std::string &type);

			const std::string &Controller() const;

			void SetController(const std::string &controller);

			const std::string &PublicKeyBase58() const;

			void SetPublicKeyBase58(const std::string &pubkey);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

		private:
			std::string _id;
			std::string _type;
			std::string _controller;
			std::string _publicKeyBase58;
		};
		typedef std::vector<DIDPubKeyInfo> DIDPubKeyInfoArray;

		class DIDPayloadInfo {
		public:
			DIDPayloadInfo();

			~DIDPayloadInfo();

			const std::string &ID() const;

			void SetID(const std::string &id);

			const DIDPubKeyInfoArray &PublicKeyInfo() const;

			void SetPublickKey(const DIDPubKeyInfoArray &pubkey);

			const std::string &Expires() const;

			void SetExpires(const std::string &expires);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

		private:
			std::string _id;
			DIDPubKeyInfoArray _publickey;
			nlohmann::json _authentication; // contain 0 or 1
			nlohmann::json _authorization; // contain 0 or 1
			nlohmann::json _verifiableCredential; // contain 0 or 1
			nlohmann::json _service; // contain 0 or 1
			std::string _expires;
		};

		class DIDProofInfo {
		public:
			DIDProofInfo();

			~DIDProofInfo();

			DIDProofInfo(const std::string &method, const std::string &signature, const std::string &type = DID_DEFAULT_TYPE);

			const std::string &Type() const;

			void SetType(const std::string &type);

			const std::string &VerificationMethod() const;

			void SetVerificationMethod(const std::string &method);

			const std::string &Signature() const;

			void SetSignature(const std::string &sign);

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &stream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &stream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

		private:
			std::string _type;
			std::string _verificationMethod;
			std::string _signature;
		};

		class DIDInfo : public IPayload {
		public:
			DIDInfo();

			~DIDInfo();

			const DIDHeaderInfo &DIDHeader() const;

			const std::string &DIDPayloadString() const;

			const DIDPayloadInfo &DIDPayload() const;

			const DIDProofInfo &DIDProof() const;

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &stream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &stream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual bool IsValid() const;

			virtual IPayload &operator=(const IPayload &payload);

			DIDInfo &operator=(const DIDInfo &payload);

		private:
			DIDHeaderInfo _header;
			std::string _payload;
			DIDProofInfo _proof;

			DIDPayloadInfo _payloadInfo;
		};
	}
}


#endif //SPVSDK_DIDINFO_H
