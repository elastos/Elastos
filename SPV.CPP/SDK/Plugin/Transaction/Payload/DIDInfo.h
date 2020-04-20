// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_DIDINFO_H__
#define __ELASTOS_SDK_DIDINFO_H__

#include "IPayload.h"

#include <Common/JsonGenerator.h>

  namespace Elastos {
	namespace ElaWallet {

#define DID_DEFAULT_TYPE "ECDSAsecp256r1"
#define PREFIX_DID "did:elastos:"
#define UPDATE_DID "update"
#define PRIMARY_KEY "#primary"

		class DIDHeaderInfo {
		public:
			DIDHeaderInfo();

			~DIDHeaderInfo();

			DIDHeaderInfo(const std::string &specification, const std::string &operation,
			              const std::string &preTxID = "");

			const std::string &Specification() const;

			void SetSpecification(const std::string &specification);

			const std::string &Operation() const;

			void SetOperation(const std::string &operation);

			void SetPreviousTxid(const std::string &txid);

			const std::string &PreviousTxid() const;

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &stream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &stream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

		private:
			std::string _specification;
			std::string _operation;
			std::string _previousTxid;
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

			void ToOrderedJson(JsonGenerator *generator) const;

			void AutoFill(const std::string &did);
		private:
			std::string _id;
			std::string _type;
			std::string _controller;
			std::string _publicKeyBase58;
		};

		typedef std::vector<DIDPubKeyInfo> DIDPubKeyInfoArray;

		class CredentialSubject {
		public:
			CredentialSubject();

			~CredentialSubject();

			void SetID(const std::string &id);

			const std::string &ID() const;

			void AutoFill(const std::string &did);

			const nlohmann::json &GetProperties() const;

			const nlohmann::json &GetValue(const std::string &key) const;

			bool HasProperties(const std::string &key) const;

			void AddProperties(const std::string &key, const std::string &value);

		public:
			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			void ToOrderedJson(JsonGenerator *generator) const;

		private:
			void Properties2OrderedJson(JsonGenerator *generator, const nlohmann::json &properties) const;

		private:
			std::string _id;
			nlohmann::json _properties;

		};

		typedef std::vector<CredentialSubject> CredentialSubjectArray;

		class DIDProofInfo {
		public:
			DIDProofInfo();

			~DIDProofInfo();

			DIDProofInfo(const std::string &method, const std::string &signature,
			             const std::string &type = DID_DEFAULT_TYPE);

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

			void AutoFill(const std::string &did);

			void ToOrderJson(JsonGenerator *generator) const;

		private:
			std::string _type;
			std::string _verificationMethod;
			std::string _signature;
		};

		class ServiceEndpoint {
		public:
			ServiceEndpoint();

			ServiceEndpoint(const std::string &id, const std::string &type, const std::string &serviceEndpoint);

			~ServiceEndpoint();

			void SetID(const std::string &id);

			const std::string &ID() const;

			void SetType(const std::string &type);

			const std::string &Type() const;

			void SetService(const std::string &service);

			const std::string &GetService() const;

			void AutoFill(const std::string &did);
		public:

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			void ToOrderedJson(JsonGenerator *generator) const;

		private:
			std::string _id;
			std::string _type;
			std::string _serviceEndpoint;
		};

		typedef std::vector<ServiceEndpoint> ServiceEndpoints;

		class VerifiableCredential {
		public:
			VerifiableCredential();

			~VerifiableCredential();

			void SetID(const std::string &id);

			const std::string &ID();

			void SetTypes(const std::vector<std::string> &types);

			const std::vector<std::string> &Types() const;

			void SetIssuer(const std::string &issuer);

			const std::string &GetIssuer() const;

			void SetIssuerDate(const std::string &issuerDate);

			const std::string &GetIssuerDate() const;

			void SetCredentialSubject(const CredentialSubject &credentialSubject);

			const CredentialSubject &GetCredentialSubject() const;

			void SetProof(const DIDProofInfo &proof);

			const DIDProofInfo &Proof() const;

			void AutoFill(const std::string &did);

		public:
			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			void ToOrderedJson(JsonGenerator *generator) const;
		private:
			std::string _id;
			std::vector<std::string> _types;
			std::string _issuer;
			std::string _issuanceDate;
			std::string _expirationDate;
			CredentialSubject _credentialSubject;
			DIDProofInfo _proof;
		};

		typedef std::vector<VerifiableCredential> VerifiableCredentialArray;

		class DIDPayloadProof {
		public:
			DIDPayloadProof();

			~DIDPayloadProof();

			void SetType(const std::string &type);

			const std::string &GetType() const;

			void SetCreateDate(const std::string &date);

			const std::string &GetCreatedDate() const;

			void SetCreator(const std::string &creator);

			const std::string &GetCreator() const;

			void SetSignature(const std::string &signature);

			const std::string &GetSignature() const;

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

		private:
			std::string _type;
			std::string _created;
			std::string _creator;
			std::string _signatureValue;
		};

		class DIDPayloadInfo {
		public:
			DIDPayloadInfo();

			~DIDPayloadInfo();

			const std::string &ID() const;

			void SetID(const std::string &id);

			const DIDPubKeyInfoArray &PublicKeyInfo() const;

			void SetPublickKey(const DIDPubKeyInfoArray &pubkey);

			const DIDPubKeyInfoArray &Authentication() const;

			void SetAuthentication(const DIDPubKeyInfoArray &authentication);

			const DIDPubKeyInfoArray &Authorization() const;

			void SetAuthorization(const DIDPubKeyInfoArray &authorization);

			const VerifiableCredentialArray &GetVerifiableCredential() const;

			void SetVerifiableCredential(const VerifiableCredentialArray &verifiableCredential);

			const ServiceEndpoints &GetServiceEndpoint() const;

			void SetServiceEndpoints(const ServiceEndpoints &serviceEndpoint);

			const std::string &Expires() const;

			void SetExpires(const std::string &expires);

			void SetProof(const DIDPayloadProof &proof);

			const DIDPayloadProof &GetProof() const;

		public:
			virtual bool IsValid() const;

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			std::string ToOrderedJson() const;

		private:
			std::string _id;
			DIDPubKeyInfoArray _publickey;
			DIDPubKeyInfoArray _authentication; // contain 0 or 1
			DIDPubKeyInfoArray _authorization; // contain 0 or 1
			VerifiableCredentialArray _verifiableCredential; // contain 0 or 1
			ServiceEndpoints _services; // contain 0 or 1
			std::string _expires;
			DIDPayloadProof _proof;
		};


		class DIDInfo : public IPayload {
		public:
			DIDInfo();

			~DIDInfo();

			void SetDIDHeader(const DIDHeaderInfo &headerInfo);

			const DIDHeaderInfo &DIDHeader() const;

			const std::string &DIDPayloadString() const;

			void SetDIDPlayloadInfo(const DIDPayloadInfo &didPayloadInfo);

			const DIDPayloadInfo &DIDPayload() const;

			void SetDIDProof(const DIDProofInfo &proofInfo);

			const DIDProofInfo &DIDProof() const;

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &stream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &stream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual bool IsValid(uint8_t version) const;

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
