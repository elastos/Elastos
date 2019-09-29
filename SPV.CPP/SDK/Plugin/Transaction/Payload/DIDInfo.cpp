// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/Log.h>
#include <SDK/Common/Base64.h>
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
			j["controller"] = _controller;
			j["publicKeyBase58"] = _publicKeyBase58;

			return j;
		}

		void DIDPubKeyInfo::FromJson(const nlohmann::json &j, uint8_t version) {
			_id = j["id"].get<std::string>();
			_publicKeyBase58 = j["publicKeyBase58"].get<std::string>();

			if (j.find("type") != j.end()) {
				_type = j["type"].get<std::string>();
			} else {
				_type = DID_DEFAULT_TYPE;
			}

			if (j.find("controller") != j.end()) {
				_controller = j["controller"].get<std::string>();
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

			j["expires"] = _expires;

			if (!_authentication.is_null())
				j["authentication"] = _authentication;
			if (!_authorization.is_null())
				j["authorization"] = _authorization;
			if (!_verifiableCredential.is_null())
				j["verifiableCredential"] = _verifiableCredential;
			if (!_service.is_null())
				j["service"] = _service;

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

			if (j.find("expires") != j.end())
				_expires = j["expires"].get<std::string>();

			if (j.find("authentication") != j.end())
				_authentication = j["authentication"];
			if (j.find("authorization") != j.end())
				_authorization = j["authorization"];
			if (j.find("verifiableCredential") != j.end())
				_verifiableCredential = j["verifiableCredential"];
			if (j.find("service") != j.end())
				_service = j["service"];
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

		const DIDHeaderInfo &DIDInfo::DIDHeader() const {
			return _header;
		}

		const std::string &DIDInfo::DIDPayloadString() const {
			return _payload;
		}

		const DIDPayloadInfo &DIDInfo::DIDPayload() const {
			return _payloadInfo;
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
			std::string payloadString((char *)bytes.data(), bytes.size());
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
			std::string payloadString((char *)bytes.data(), bytes.size());
			_payloadInfo.FromJson(nlohmann::json::parse(payloadString), version);
		}

		bool DIDInfo::IsValid() const {
			return true;
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
