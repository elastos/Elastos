// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_DIDINFO_H__
#define __ELASTOS_SDK_DIDINFO_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

#define DID_DEFAULT_TYPE "ECDSAsecp256r1"
#define PREFIX_DID "did:elastos:"

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

		class CredentialSubject {
		public:
			CredentialSubject();

			~CredentialSubject();

			void SetID(const std::string &id);

			const std::string &ID() const;

			void SetDIDName(const std::string &didName);

			const std::string &GetDIDName() const;

			void SetName(const std::string &name);

			const std::string &GetName() const;

			void SetNickName(const std::string &nickName);

			const std::string &GetNickName() const;

			void SetGender(const std::string &gender);

			const std::string &GetGender() const;

			void SetBirthday(const std::string birthday);

			const std::string &GetBirthday() const;

			void SetAvatar(const std::string &avatar);

			const std::string &GetAvatar() const;

			void SetEmail(const std::string &email);

			const std::string &GetEmail() const;

			void SetPhone(const std::string &phone);

			const std::string GetPhone() const;

			void SetNation(const std::string &nation);

			const std::string &GetNation() const;

			void SetDescript(const std::string &descript);

			const std::string &GetDescript() const;

			void SetHomePage(const std::string &homePage);

			const std::string &GetHomePage() const;

			void SetGoogleAccount(const std::string &googleAccount);

			const std::string &GetGoogleAccount() const;

			void SetMicrosoftPassport(const std::string &microsoftPassport);

			const std::string &GetMicrosoftPassport() const;

			void SetFacebook(const std::string &facebook);

			const std::string &GetFacebook() const;

			void SetTwitter(const std::string &twitter);

			const std::string &GetTwitter() const;

			void SetWeibo(const std::string &weibo);

			const std::string &GetWeibo() const;

			void SetWechat(const std::string &wechat);

			const std::string &GetWechat() const;

			void SetAlipay(const std::string &alipay);

			const std::string &GetAlipay() const;

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &stream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &stream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);
		private:
			void init();

		private:
			std::string _id;
			std::string _didName;
			std::string _name;
			std::string _nickname;
			std::string _gender;
			std::string _birthday;
			std::string _avatar;
			std::string _email;
			std::string _phone;
			std::string _nation;

			std::string _descript;

			std::string _homePage;
			std::string _googleAccount;
			std::string _microsoftPassport;
			std::string _facebook;
			std::string _twitter;
			std::string _weibo;
			std::string _wechat;
			std::string _alipay;

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
		public:

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

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

		public:
			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);
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

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

		private:
			std::string _id;
			DIDPubKeyInfoArray _publickey;
			DIDPubKeyInfoArray _authentication; // contain 0 or 1
			DIDPubKeyInfoArray _authorization; // contain 0 or 1
			VerifiableCredentialArray _verifiableCredential; // contain 0 or 1
			ServiceEndpoints _services; // contain 0 or 1
			std::string _expires;
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
