// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <Common/Log.h>
#include <Common/Base64.h>
#include <Common/ErrorChecker.h>
#include <WalletCore/Base58.h>
#include <WalletCore/Key.h>

#include "DIDInfo.h"

namespace Elastos {
	namespace ElaWallet {

		DIDHeaderInfo::DIDHeaderInfo() {

		}

		DIDHeaderInfo::~DIDHeaderInfo() {

		}

		DIDHeaderInfo::DIDHeaderInfo(const std::string &specification, const std::string &operation) :
				_specification(specification), _operation(operation) {

		}

		const std::string &DIDHeaderInfo::Specification() const {
			return _specification;
		}

		void DIDHeaderInfo::SetSpecification(const std::string &specification) {
			_specification = specification;
		}

		const std::string &DIDHeaderInfo::Operation() const {
			return _operation;
		}

		void DIDHeaderInfo::SetOperation(const std::string &operation) {
			_operation = operation;
		}

		size_t DIDHeaderInfo::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			size = stream.WriteVarUint(_specification.size());
			size += _specification.size();
			size += stream.WriteVarUint(_operation.size());
			size += _operation.size();

			return size;
		}

		void DIDHeaderInfo::Serialize(ByteStream &stream, uint8_t version) const {
			stream.WriteVarString(_specification);
			stream.WriteVarString(_operation);
		}

		bool DIDHeaderInfo::Deserialize(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_specification)) {
				Log::error("DIDHeaderInfo deserialize: specification");
				return false;
			}

			if (!stream.ReadVarString(_operation)) {
				Log::error("DIDHeaderInfo deserialize: operation");
				return false;
			}

			return true;
		}

		nlohmann::json DIDHeaderInfo::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["specification"] = _specification;
			j["operation"] = _operation;
			return j;
		}

		void DIDHeaderInfo::FromJson(const nlohmann::json &j, uint8_t version) {
			_specification = j["specification"].get<std::string>();
			_operation = j["operation"].get<std::string>();
		}

		DIDPubKeyInfo::DIDPubKeyInfo() {

		}

		DIDPubKeyInfo::~DIDPubKeyInfo() {

		}

		DIDPubKeyInfo::DIDPubKeyInfo(const std::string &id, const std::string &pubkeyBase58,
		                             const std::string &controller, const std::string &type) :
				_id(id), _publicKeyBase58(pubkeyBase58), _controller(controller), _type(type) {

		}

		const std::string &DIDPubKeyInfo::ID() const {
			return _id;
		}

		void DIDPubKeyInfo::SetID(const std::string &id) {
			_id = id;
		}

		const std::string &DIDPubKeyInfo::Type() const {
			return _type;
		}

		void DIDPubKeyInfo::SetType(const std::string &type) {
			_type = type;
		}

		const std::string &DIDPubKeyInfo::Controller() const {
			return _controller;
		}

		void DIDPubKeyInfo::SetController(const std::string &controller) {
			_controller = controller;
		}

		const std::string &DIDPubKeyInfo::PublicKeyBase58() const {
			return _publicKeyBase58;
		}

		void DIDPubKeyInfo::SetPublicKeyBase58(const std::string &pubkey) {
			_publicKeyBase58 = pubkey;
		}

		nlohmann::json DIDPubKeyInfo::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["id"] = _id;
			j["type"] = _type;

			if (!_controller.empty()){
				j["controller"] = _controller;
			}

			j["publicKeyBase58"] = _publicKeyBase58;

			return j;
		}

		void DIDPubKeyInfo::FromJson(const nlohmann::json &j, uint8_t version) {
			if (j.is_structured()) {
				_id = j["id"].get<std::string>();
				_publicKeyBase58 = j["publicKeyBase58"].get<std::string>();
			} else if (j.is_string()) {
				_id = j.get<std::string>();
			}

			if (j.find("type") != j.end()) {
				_type = j["type"].get<std::string>();
			} else {
				_type = DID_DEFAULT_TYPE;
			}

			if (j.find("controller") != j.end()) {
				_controller = j["controller"].get<std::string>();
				ErrorChecker::CheckParam(!_controller.empty() && _controller.find(PREFIX_DID) == std::string::npos, Error::InvalidArgument, "invalid controller");
			}
		}

		CredentialSubject::CredentialSubject() {
			init();
		}

		CredentialSubject::~CredentialSubject() {

		}

		void CredentialSubject::init() {
			_id = "";
			_didName = "";
			_name = "";
			_nickname = "";
			_gender = "";
			_birthday = "";
			_avatar = "";
			_email = "";
			_phone = "";
			_nation = "";
			_descript = "";
			_homePage = "";
			_googleAccount = "";
			_microsoftPassport = "";
			_facebook = "";
			_twitter = "";
			_weibo = "";
			_wechat = "";
			_alipay = "";
		}

		void CredentialSubject::SetID(const std::string &id) {
			ErrorChecker::CheckParam(id.find(PREFIX_DID) == std::string::npos, Error::InvalidArgument, "invalid id");
			_id = id;
		}

		const std::string &CredentialSubject::ID() const {
			return _id;
		}

		void CredentialSubject::SetDIDName(const std::string &didName) {
			_didName = didName;
		}

		const std::string &CredentialSubject::GetDIDName() const {
			return _didName;
		}

		void CredentialSubject::SetName(const std::string &name) {
			_name = name;
		}

		const std::string &CredentialSubject::GetName() const {
			return _name;
		}

		void CredentialSubject::SetNickName(const std::string &nickName) {
			_nickname = nickName;
		}

		const std::string &CredentialSubject::GetNickName() const {
			return _nickname;
		}

		void CredentialSubject::SetGender(const std::string &gender) {
			_gender = gender;
		}

		const std::string &CredentialSubject::GetGender() const {
			return _gender;
		}

		void CredentialSubject::SetBirthday(const std::string birthday) {
			_birthday = birthday;
		}

		const std::string &CredentialSubject::GetBirthday() const {
			return _birthday;
		}

		void CredentialSubject::SetAvatar(const std::string &avatar) {
			_avatar = avatar;
		}

		const std::string &CredentialSubject::GetAvatar() const {
			return _avatar;
		}

		void CredentialSubject::SetEmail(const std::string &email) {
			_email = email;
		}

		const std::string &CredentialSubject::GetEmail() const {
			return _email;
		}

		void CredentialSubject::SetPhone(const std::string &phone) {
			_phone = phone;
		}

		const std::string CredentialSubject::GetPhone() const {
			return _phone;
		}

		void CredentialSubject::SetNation(const std::string &nation) {
			_nation = nation;
		}

		const std::string &CredentialSubject::GetNation() const {
			return _nation;
		}

		void CredentialSubject::SetDescript(const std::string &descript) {
			_descript = descript;
		}

		const std::string &CredentialSubject::GetDescript() const {
			return _descript;
		}

		void CredentialSubject::SetHomePage(const std::string &homePage) {
			_homePage = homePage;
		}

		const std::string &CredentialSubject::GetHomePage() const {
			return _homePage;
		}

		void CredentialSubject::SetGoogleAccount(const std::string &googleAccount) {
			_googleAccount = googleAccount;
		}

		const std::string &CredentialSubject::GetGoogleAccount() const {
			return _googleAccount;
		}

		void CredentialSubject::SetMicrosoftPassport(const std::string &microsoftPassport) {
			_microsoftPassport = microsoftPassport;
		}

		const std::string &CredentialSubject::GetMicrosoftPassport() const {
			return _microsoftPassport;
		}

		void CredentialSubject::SetFacebook(const std::string &facebook) {
			_facebook = facebook;
		}

		const std::string &CredentialSubject::GetFacebook() const {
			return _facebook;
		}

		void CredentialSubject::SetTwitter(const std::string &twitter) {
			_twitter = twitter;
		}

		const std::string &CredentialSubject::GetTwitter() const {
			return _twitter;
		}

		void CredentialSubject::SetWeibo(const std::string &weibo) {
			_weibo = weibo;
		}

		const std::string &CredentialSubject::GetWeibo() const {
			return _weibo;
		}

		void CredentialSubject::SetWechat(const std::string &wechat) {
			_wechat = wechat;
		}

		const std::string &CredentialSubject::GetWechat() const {
			return _wechat;
		}

		void CredentialSubject::SetAlipay(const std::string &alipay) {
			_alipay = alipay;
		}

		const std::string &CredentialSubject::GetAlipay() const {
			return _alipay;
		}

		nlohmann::json CredentialSubject::ToJson(uint8_t version) const {
			nlohmann::json j;

			if (_id.size() > 0) {
				j["id"] = _id;
			}

			if (_didName.size() > 0) {
				j["didName"] = _didName;
			}

			if (_name.size() > 0) {
				j["name"] = _name;
			}

			if (_name.size() > 0) {
				j["name"] = _name;
			}

			if (_nickname.size() > 0) {
				j["nickname"] = _nickname;
			}

			if (_gender.size() > 0) {
				j["gender"] = _gender;
			}

			if (_birthday.size() > 0) {
				j["birthday"] = _birthday;
			}

			if (_avatar.size() > 0) {
				j["avatar"] = _avatar;
			}

			if (_email.size() > 0) {
				j["email"] = _email;
			}

			if (_phone.size() > 0) {
				j["phone"] = _phone;
			}

			if (_nation.size() > 0) {
				j["nation"] = _nation;
			}

			if (_descript.size() > 0) {
				j["descript"] = _descript;
			}

			if (_homePage.size() > 0) {
				j["homePage"] = _homePage;
			}

			if (_googleAccount.size() > 0) {
				j["googleAccount"] = _googleAccount;
			}

			if (_microsoftPassport.size() > 0) {
				j["microsoftPassport"] = _microsoftPassport;
			}

			if (_facebook.size() > 0) {
				j["facebook"] = _facebook;
			}

			if (_twitter.size() > 0) {
				j["twitter"] = _twitter;
			}

			if (_weibo.size() > 0) {
				j["weibo"] = _weibo;
			}

			if (_wechat.size() > 0) {
				j["wechat"] = _wechat;
			}

			if (_alipay.size() > 0) {
				j["alipay"] = _alipay;
			}

			return j;
		}

		void CredentialSubject::FromJson(const nlohmann::json &j, uint8_t version) {
			if (j.find("id") != j.end()) {
				_id = j["id"].get<std::string>();
				ErrorChecker::CheckParam(_id.find(PREFIX_DID) == std::string::npos, Error::InvalidArgument, "invalid id");
			}

			if (j.find("didName") != j.end()) {
				_didName = j["didName"].get<std::string>();
			}

			if (j.find("name") != j.end())
				_name = j["name"].get<std::string>();

			if (j.find("nickname") != j.end())
				_nickname = j["nickname"].get<std::string>();

			if (j.find("gender") != j.end())
				_gender = j["gender"].get<std::string>();

			if (j.find("birthday") != j.end())
				_birthday = j["birthday"].get<std::string>();

			if (j.find("avatar") != j.end())
				_avatar = j["avatar"].get<std::string>();

			if (j.find("email") != j.end())
				_email = j["email"].get<std::string>();

			if (j.find("phone") != j.end())
				_phone = j["phone"].get<std::string>();

			if (j.find("nation") != j.end())
				_nation = j["nation"].get<std::string>();

			if (j.find("descript") != j.end())
				_descript = j["descript"].get<std::string>();

			if (j.find("homePage") != j.end())
				_homePage = j["homePage"].get<std::string>();

			if (j.find("googleAccount") != j.end())
				_googleAccount = j["googleAccount"].get<std::string>();

			if (j.find("microsoftPassport") != j.end())
				_microsoftPassport = j["microsoftPassport"].get<std::string>();

			if (j.find("facebook") != j.end())
				_facebook = j["facebook"].get<std::string>();

			if (j.find("twitter") != j.end())
				_twitter = j["twitter"].get<std::string>();

			if (j.find("weibo") != j.end())
				_weibo = j["weibo"].get<std::string>();

			if (j.find("wechat") != j.end())
				_wechat = j["wechat"].get<std::string>();

			if (j.find("alipay") != j.end())
				_alipay = j["alipay"].get<std::string>();
		}

		ServiceEndpoint::ServiceEndpoint() : _id(""), _type(""), _serviceEndpoint("") {

		}

		ServiceEndpoint::ServiceEndpoint(const std::string &id, const std::string &type,
		                                 const std::string &serviceEndpoint) : _id(id), _type(type),
		                                                                       _serviceEndpoint(serviceEndpoint) {

		}

		ServiceEndpoint::~ServiceEndpoint() {

		}

		void ServiceEndpoint::SetID(const std::string &id) {
			_id = id;
		}

		const std::string &ServiceEndpoint::ID() const {
			return _id;
		}

		void ServiceEndpoint::SetType(const std::string &type) {
			_type = type;
		}

		const std::string &ServiceEndpoint::Type() const {
			return _type;
		}

		void ServiceEndpoint::SetService(const std::string &service) {
			_serviceEndpoint = service;
		}

		const std::string &ServiceEndpoint::GetService() const {
			return _serviceEndpoint;
		}

		nlohmann::json ServiceEndpoint::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["id"] = _id;
			j["type"] = _type;
			j["serviceEndpoint"] = _serviceEndpoint;

			return j;
		}

		void ServiceEndpoint::FromJson(const nlohmann::json &j, uint8_t version) {
			if (j.find("id") != j.end())
				_id = j["id"].get<std::string>();

			if (j.find("type") != j.end())
				_type = j["type"].get<std::string>();

			if (j.find("serviceEndpoint") != j.end())
				_serviceEndpoint = j["serviceEndpoint"].get<std::string>();
		}

		VerifiableCredential::VerifiableCredential() {

		}

		VerifiableCredential::~VerifiableCredential() {

		}

		void VerifiableCredential::SetID(const std::string &id) {
			_id = id;
		}

		const std::string &VerifiableCredential::ID() {
			return _id;
		}

		void VerifiableCredential::SetTypes(const std::vector<std::string> &types) {
			_types = types;
		}

		const std::vector<std::string> &VerifiableCredential::Types() const {
			return _types;
		}

		void VerifiableCredential::SetIssuer(const std::string &issuer) {
			_issuer = issuer;
		}

		const std::string &VerifiableCredential::GetIssuer() const {
			return _issuer;
		}

		void VerifiableCredential::SetIssuerDate(const std::string &issuerDate) {
			_issuanceDate = issuerDate;
		}

		const std::string &VerifiableCredential::GetIssuerDate() const {
			return _issuanceDate;
		}

		void VerifiableCredential::SetCredentialSubject(const CredentialSubject &credentialSubject) {
			_credentialSubject = credentialSubject;
		}

		const CredentialSubject &VerifiableCredential::GetCredentialSubject() const {
			return _credentialSubject;
		}

		void VerifiableCredential::SetProof(const DIDProofInfo &proof) {
			_proof = proof;
		}

		const DIDProofInfo &VerifiableCredential::Proof() const {
			return _proof;
		}

		nlohmann::json VerifiableCredential::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["id"] = _id;
			j["types"] = _types;
			j["issuer"] = _issuer;
			j["issuanceDate"] = _issuanceDate;
			j["expirationDate"] = _expirationDate;
			j["credentialSubject"] = _credentialSubject.ToJson(version);
			j["proof"] = _proof.ToJson(version);

			return j;
		}

		void VerifiableCredential::FromJson(const nlohmann::json &j, uint8_t version) {
			_id = j["id"].get<std::string>();

			if (j.find("types") != j.end()) {
				_types.clear();
				std::vector<std::string> types = j["types"];
				_types = types;

			}

			if (j.find("issuer") != j.end()) {
				_issuer = j["issuer"].get<std::string>();
			}

			if (j.find("issuanceDate") != j.end()) {
				_issuanceDate = j["issuanceDate"].get<std::string>();
			}

			if (j.find("expirationDate") != j.end()) {
				_expirationDate = j["expirationDate"].get<std::string>();
			}

			if (j.find("credentialSubject") != j.end()) {
				_credentialSubject.FromJson(j["credentialSubject"], version);
			}

			if (j.find("proof") != j.end()) {
				_proof.FromJson(j["proof"], version);
			}

		}

		DIDPayloadInfo::DIDPayloadInfo() {

		}

		DIDPayloadInfo::~DIDPayloadInfo() {

		}

		const std::string &DIDPayloadInfo::ID() const {
			return _id;
		}

		void DIDPayloadInfo::SetID(const std::string &id) {
			_id = id;
		}

		const DIDPubKeyInfoArray &DIDPayloadInfo::PublicKeyInfo() const {
			return _publickey;
		}

		void DIDPayloadInfo::SetPublickKey(const DIDPubKeyInfoArray &pubkey) {
			_publickey = pubkey;
		}

		const DIDPubKeyInfoArray &DIDPayloadInfo::Authentication() const {
			return _authentication;
		}

		void DIDPayloadInfo::SetAuthentication(const DIDPubKeyInfoArray &authentication) {
				_authentication = authentication;
		}

		const DIDPubKeyInfoArray &DIDPayloadInfo::Authorization() const {
			return _authorization;
		}

		void DIDPayloadInfo::SetAuthorization(const DIDPubKeyInfoArray &authorization) {
			_authorization = authorization;
		}

		const VerifiableCredentialArray &DIDPayloadInfo::GetVerifiableCredential() const {
			return _verifiableCredential;
		}

		void DIDPayloadInfo::SetVerifiableCredential(const VerifiableCredentialArray &verifiableCredential) {
			_verifiableCredential = verifiableCredential;
		}

		const ServiceEndpoints &DIDPayloadInfo::GetServiceEndpoint() const {
			return _services;
		}

		void DIDPayloadInfo::SetServiceEndpoints(const ServiceEndpoints &serviceEndpoint) {
			_services = serviceEndpoint;
		}

		const std::string &DIDPayloadInfo::Expires() const {
			return _expires;
		}

		void DIDPayloadInfo::SetExpires(const std::string &expires) {
			_expires = expires;
		}

		nlohmann::json DIDPayloadInfo::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["id"] = _id;

			nlohmann::json jPubKey;
			for (DIDPubKeyInfoArray::const_iterator it = _publickey.cbegin(); it != _publickey.cend(); ++it)
				jPubKey.push_back((*it).ToJson(version));
			j["publicKey"] = jPubKey;

			if (_authentication.size()) {
				nlohmann::json jAuthentication;
				for (DIDPubKeyInfoArray::const_iterator it = _authentication.cbegin(); it != _authentication.cend(); ++it)
					jAuthentication.push_back((*it).ToJson(version));
				j["authentication"] = jAuthentication;
			}

			if (_authorization.size()) {
				nlohmann::json jAuthorization;
				for (DIDPubKeyInfoArray::const_iterator it = _authorization.cbegin(); it != _authorization.cend(); ++it)
					jAuthorization.push_back((*it).ToJson(version));
				j["authorization"] = jAuthorization;
			}

			if (_verifiableCredential.size()) {
				nlohmann::json jVerifiableCredential;
				for (VerifiableCredentialArray::const_iterator it = _verifiableCredential.cbegin();
				     it != _verifiableCredential.cend(); ++it) {
					jVerifiableCredential.push_back((*it).ToJson(version));
				}
				j["verifiableCredential"] = jVerifiableCredential;
			}

			j["expires"] = _expires;

			if (_services.size()) {
				nlohmann::json jService;
				for (ServiceEndpoints::const_iterator it = _services.cbegin(); it != _services.cend(); ++it) {
					jService.push_back((*it).ToJson(version));
				}
				j["service"] = jService;
			}

			return j;
		}

		void DIDPayloadInfo::FromJson(const nlohmann::json &j, uint8_t version) {
			_id = j["id"].get<std::string>();

			nlohmann::json jPubKey = j["publicKey"];
			for (nlohmann::json::iterator it = jPubKey.begin(); it != jPubKey.end(); ++it) {
				DIDPubKeyInfo pubKeyInfo;
				pubKeyInfo.FromJson(*it, version);
				_publickey.push_back(pubKeyInfo);
			}

			if (j.find("authentication") != j.end()) {
				nlohmann::json jAuthentication = j["authentication"];
				for (nlohmann::json::iterator it = jAuthentication.begin(); it != jAuthentication.end(); ++it) {
					DIDPubKeyInfo pubKeyInfo;
					pubKeyInfo.FromJson(*it, version);
					_authentication.push_back(pubKeyInfo);
				}
			}

			if (j.find("authorization") != j.end()) {
				nlohmann::json jAuthorization = j["authorization"];
				for (nlohmann::json::iterator it = jAuthorization.begin(); it != jAuthorization.end(); ++it) {
					DIDPubKeyInfo pubKeyInfo;
					pubKeyInfo.FromJson(*it, version);
					_authorization.push_back(pubKeyInfo);
				}
			}

			_expires = j["expires"].get<std::string>();

			if (j.find("verifiableCredential") != j.end()) {
				nlohmann::json jVerifiableCredential = j["verifiableCredential"];
				for (nlohmann::json::iterator it = jVerifiableCredential.begin(); it != jVerifiableCredential.end(); ++it) {
					VerifiableCredential verifiableCredential;
					verifiableCredential.FromJson(*it, version);
					_verifiableCredential.push_back(verifiableCredential);
				}
			}

			if (j.find("service") != j.end()) {
				nlohmann::json jservices = j["service"];
				for (nlohmann::json::iterator it = jservices.begin(); it != jservices.end(); ++it) {
					ServiceEndpoint serviceEndpoint;
					serviceEndpoint.FromJson(*it, version);
					_services.push_back(serviceEndpoint);
				}
			}

		}

		DIDProofInfo::DIDProofInfo() {

		}

		DIDProofInfo::~DIDProofInfo() {

		}

		DIDProofInfo::DIDProofInfo(const std::string &method, const std::string &signature, const std::string &type) :
				_verificationMethod(method), _signature(signature), _type(type) {

		}

		const std::string &DIDProofInfo::Type() const {
			return _type;
		}

		void DIDProofInfo::SetType(const std::string &type) {
			_type = type;
		}

		const std::string &DIDProofInfo::VerificationMethod() const {
			return _verificationMethod;
		}

		void DIDProofInfo::SetVerificationMethod(const std::string &method) {
			_verificationMethod = method;
		}

		const std::string &DIDProofInfo::Signature() const {
			return _signature;
		}

		void DIDProofInfo::SetSignature(const std::string &sign) {
			_signature = sign;
		}

		size_t DIDProofInfo::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			size += stream.WriteVarUint(_type.size());
			size += _type.size();
			size += stream.WriteVarUint(_verificationMethod.size());
			size += _verificationMethod.size();
			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			return size;
		}

		void DIDProofInfo::Serialize(ByteStream &stream, uint8_t version) const {
			stream.WriteVarString(_type);
			stream.WriteVarString(_verificationMethod);
			stream.WriteVarString(_signature);
		}

		bool DIDProofInfo::Deserialize(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_type)) {
				Log::error("DIDProofInfo deserialize: type");
				return false;
			}

			if (!stream.ReadVarString(_verificationMethod)) {
				Log::error("DIDProofInfo deserialize verificationMethod");
				return false;
			}

			if (!stream.ReadVarString(_signature)) {
				Log::error("DIDProofInfo deserialize sign");
				return false;
			}

			return true;
		}

		nlohmann::json DIDProofInfo::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["type"] = _type;
			j["verificationMethod"] = _verificationMethod;
			j["signature"] = _signature;

			return j;
		}

		void DIDProofInfo::FromJson(const nlohmann::json &j, uint8_t version) {
			if (j.find("type") != j.end())
				_type = j["type"].get<std::string>();
			else
				_type = DID_DEFAULT_TYPE;
			_verificationMethod = j["verificationMethod"].get<std::string>();
			_signature = j["signature"].get<std::string>();
		}

		DIDInfo::DIDInfo() {

		}

		DIDInfo::~DIDInfo() {

		}

		void DIDInfo::SetDIDHeader(const DIDHeaderInfo &headerInfo) {
			_header = headerInfo;
		}

		const DIDHeaderInfo &DIDInfo::DIDHeader() const {
			return _header;
		}

		const std::string &DIDInfo::DIDPayloadString() const {
			return _payload;
		}

		void DIDInfo::SetDIDPlayloadInfo(const DIDPayloadInfo &didPayloadInfo) {
			_payloadInfo = didPayloadInfo;
			std::string str = _payloadInfo.ToJson(0).dump();
			bytes_t data(str.data(), str.size());
			_payload = Base64::EncodeURL(data) ;
		}

		const DIDPayloadInfo &DIDInfo::DIDPayload() const {
			return _payloadInfo;
		}

		void DIDInfo::SetDIDProof(const DIDProofInfo &proofInfo) {
			_proof = proofInfo;
		}

		const DIDProofInfo &DIDInfo::DIDProof() const {
			return _proof;
		}

		size_t DIDInfo::EstimateSize(uint8_t version) const {
			size_t size = 0;

			size += _header.EstimateSize(version);
			size += _payload.size();
			size += _proof.EstimateSize(version);

			return size;
		}

		void DIDInfo::Serialize(ByteStream &stream, uint8_t version) const {
			_header.Serialize(stream, version);
			stream.WriteVarString(_payload);
			_proof.Serialize(stream, version);
		}

		bool DIDInfo::Deserialize(const ByteStream &stream, uint8_t version) {
			if (!_header.Deserialize(stream, version)) {
				Log::error("DIDInfo deserialize header");
				return false;
			}

			if (!stream.ReadVarString(_payload)) {
				Log::error("DIDInfo deserialize payload");
				return false;
			}

			if (!_proof.Deserialize(stream, version)) {
				Log::error("DIDInfo deserialize proof");
				return false;
			}

			bytes_t bytes = Base64::DecodeURL(_payload);
			std::string payloadString((char *) bytes.data(), bytes.size());
			_payloadInfo.FromJson(nlohmann::json::parse(payloadString), version);

			return true;
		}

		nlohmann::json DIDInfo::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["header"] = _header.ToJson(version);
			j["payload"] = _payload;
			j["proof"] = _proof.ToJson(version);

			return j;
		}

		void DIDInfo::FromJson(const nlohmann::json &j, uint8_t version) {
			_header.FromJson(j["header"], version);
			_payload = j["payload"].get<std::string>();
			_proof.FromJson(j["proof"], version);

			bytes_t bytes = Base64::DecodeURL(_payload);
			std::string payloadString((char *) bytes.data(), bytes.size());
			_payloadInfo.FromJson(nlohmann::json::parse(payloadString), version);
		}

		bool DIDInfo::IsValid() const {
			bool verifiedSign = false;

			if (_proof.Type() != DID_DEFAULT_TYPE) {
				Log::error("unsupport did type");
				return false;
			}

			std::string proofID = _proof.VerificationMethod();
			if (proofID.empty()) {
				Log::error("VerificationMethod of proof is empty");
				return false;
			}

			if (proofID[0] == '#')
				proofID = _payloadInfo.ID() + proofID;

			std::string sourceData = _header.Specification() + _header.Operation() + _payload;
			const DIDPubKeyInfoArray &pubkeyInfoArray = _payloadInfo.PublicKeyInfo();
			for (DIDPubKeyInfoArray::const_iterator it = pubkeyInfoArray.cbegin(); it != pubkeyInfoArray.cend(); ++it) {
				std::string pubkeyID = (*it).ID();
				if (pubkeyID[0] == '#')
					pubkeyID = _payloadInfo.ID() + pubkeyID;

				if (proofID == pubkeyID) {
					bytes_t signature = Base64::Decode(_proof.Signature());
					bytes_t pubkey = Base58::Decode((*it).PublicKeyBase58());
					Key key;
					key.SetPubKey(pubkey);

					if (key.Verify(sourceData, signature)) {
						verifiedSign = true;
					}

					break;
				}
			}

			if (!verifiedSign) {
				Log::error("did payload verify signature fail");
			}

			return verifiedSign;
		}

		IPayload &DIDInfo::operator=(const IPayload &payload) {
			try {
				const DIDInfo &didInfo = dynamic_cast<const DIDInfo &>(payload);
				operator=(didInfo);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CRInfo");
			}

			return *this;
		}

		DIDInfo &DIDInfo::operator=(const DIDInfo &payload) {
			_header = payload._header;
			_payload = payload._payload;
			_proof = payload._proof;

			_payloadInfo = payload._payloadInfo;

			return *this;
		}

	}
}
