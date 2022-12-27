/*
 * Copyright (c) 2019 Elastos Foundation LTD.
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
#include "CRCProposalTracking.h"

#include <Common/Log.h>
#include <Common/hash.h>
#include <WalletCore/Key.h>
#include <Common/Base64.h>
#include <Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {

#define MESSAGE_DATA_MAX_SIZE (800 * 1024)
#define OPINION_DATA_MAX_SIZE (200 * 1024)

		CRCProposalTracking::CRCProposalTracking() {

		}

		CRCProposalTracking::~CRCProposalTracking() {

		}

		void CRCProposalTracking::SetProposalHash(const uint256 &proposalHash) {
			_proposalHash = proposalHash;
		}

		const uint256 &CRCProposalTracking::GetProposalHash() const {
			return _proposalHash;
		}

		void CRCProposalTracking::SetMessageHash(const uint256 &messageHash) {
			_messageHash = messageHash;
		}

		const uint256 &CRCProposalTracking::GetMessageHash() const {
			return _messageHash;
		}

		void CRCProposalTracking::SetMessageData(const bytes_t &data) {
			_messageData = data;
		}

		const bytes_t &CRCProposalTracking::GetMessageData() const {
			return _messageData;
		}

		void CRCProposalTracking::SetStage(uint8_t stage) {
			_stage = stage;
		}

		uint8_t CRCProposalTracking::GetStage() const {
			return _stage;
		}

		void CRCProposalTracking::SetOwnerPubKey(const bytes_t &ownerPubKey) {
			_ownerPubKey = ownerPubKey;
		}

		const bytes_t &CRCProposalTracking::GetOwnerPubKey() const {
			return _ownerPubKey;
		}

		void CRCProposalTracking::SetNewOwnerPubKey(const bytes_t &newOwnerPubKey) {
			_newOwnerPubKey = newOwnerPubKey;
		}

		const bytes_t &CRCProposalTracking::GetNewOwnerPubKey() const {
			return _newOwnerPubKey;
		}

		void CRCProposalTracking::SetOwnerSign(const bytes_t &signature) {
			_ownerSign = signature;
		}

		const bytes_t &CRCProposalTracking::GetOwnerSign() const {
			return _ownerSign;
		}

		void CRCProposalTracking::SetNewOwnerSign(const bytes_t &signature) {
			_newOwnerSign = signature;
		}

		const bytes_t &CRCProposalTracking::GetNewOwnerSign() const {
			return _newOwnerSign;
		}

		void CRCProposalTracking::SetType(CRCProposalTracking::Type type) {
			_type = type;
		}

		CRCProposalTracking::Type CRCProposalTracking::GetType() const {
			return _type;
		}

		void CRCProposalTracking::SetSecretaryGeneralOpinionHash(const uint256 &hash) {
			_secretaryGeneralOpinionHash = hash;
		}

		const uint256 &CRCProposalTracking::GetSecretaryGeneralOpinionHash()  const {
			return _secretaryGeneralOpinionHash;
		}

		void CRCProposalTracking::SetSecretaryGeneralOpinionData(const bytes_t &data) {
			_secretaryGeneralOpinionData = data;
		}

		const bytes_t &CRCProposalTracking::GetSecretaryGeneralOpinionData()  const {
			return _secretaryGeneralOpinionData;
		}

		void CRCProposalTracking::SetSecretaryGeneralSignature(const bytes_t &signature) {
			_secretaryGeneralSignature = signature;
		}

		const bytes_t &CRCProposalTracking::GetSecretaryGeneralSignature() const {
			return _secretaryGeneralSignature;
		}

		const uint256 &CRCProposalTracking::DigestOwnerUnsigned(uint8_t version) const {
			if (_digestOwnerUnsigned == 0) {
				ByteStream stream;
				SerializeOwnerUnsigned(stream, version);
				_digestOwnerUnsigned = sha256(stream.GetBytes());
			}

			return _digestOwnerUnsigned;
		}

		const uint256 &CRCProposalTracking::DigestNewOwnerUnsigned(uint8_t version) const {
			if (_digestNewOwnerUnsigned == 0) {
				ByteStream stream;
				SerializeNewOwnerUnsigned(stream, version);
				_digestNewOwnerUnsigned = sha256(stream.GetBytes());
			}

			return _digestNewOwnerUnsigned;
		}

		const uint256 &CRCProposalTracking::DigestSecretaryUnsigned(uint8_t version) const {
			if (_digestSecretaryUnsigned == 0) {
				ByteStream stream;
				SerializeSecretaryUnsigned(stream, version);
				_digestSecretaryUnsigned = sha256(stream.GetBytes());
			}

			return _digestSecretaryUnsigned;
		}

		size_t CRCProposalTracking::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			if (version >= CRCProposalTrackingVersion01) {
				size += stream.WriteVarUint(_messageData.size());
				size += _messageData.size();
				size += stream.WriteVarUint(_secretaryGeneralOpinionData.size());
				size += _secretaryGeneralOpinionData.size();
			}
			size += _proposalHash.size();
			size += _messageHash.size();

			size += sizeof(uint8_t); // stage

			size += stream.WriteVarUint(_ownerPubKey.size());
			size += _ownerPubKey.size();

			size += stream.WriteVarUint(_newOwnerPubKey.size());
			size += _newOwnerPubKey.size();

			size += stream.WriteVarUint(_ownerSign.size());
			size += _ownerSign.size();

			size += stream.WriteVarUint(_newOwnerSign.size());
			size += _newOwnerSign.size();

			size += sizeof(uint8_t); // type

			size += _secretaryGeneralOpinionHash.size();

			size += stream.WriteVarUint(_secretaryGeneralSignature.size());
			size += _secretaryGeneralSignature.size();

			return size;
		}

		void CRCProposalTracking::SerializeOwnerUnsigned(ByteStream &stream, uint8_t version) const {
			stream.WriteBytes(_proposalHash);
			stream.WriteBytes(_messageHash);
			if (version >= CRCProposalTrackingVersion01)
				stream.WriteVarBytes(_messageData);
			stream.WriteUint8(_stage);
			stream.WriteVarBytes(_ownerPubKey);
			stream.WriteVarBytes(_newOwnerPubKey);
		}

		bool CRCProposalTracking::DeserializeOwnerUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadBytes(_proposalHash)) {
				SPVLOG_ERROR("deserialize proposal hash");
				return false;
			}

			if (!stream.ReadBytes(_messageHash)) {
				SPVLOG_ERROR("deserialize document hash");
				return false;
			}

			if (version >= CRCProposalTrackingVersion01) {
				if (!stream.ReadVarBytes(_messageData)) {
					SPVLOG_ERROR("deserialize msg data");
					return false;
				}
			}

			if (!stream.ReadUint8(_stage)) {
				SPVLOG_ERROR("deserialize stage");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPubKey)) {
				SPVLOG_ERROR("deserialize owner public key");
				return false;
			}

			if (!stream.ReadVarBytes(_newOwnerPubKey)) {
				SPVLOG_ERROR("deserialize new owner public key");
				return false;
			}

			return true;
		}

		void CRCProposalTracking::SerializeNewOwnerUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeOwnerUnsigned(stream, version);

			stream.WriteVarBytes(_ownerSign);
		}

		bool CRCProposalTracking::DeserializeNewOwnerUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeOwnerUnsigned(stream, version))
				return false;

			if (!stream.ReadVarBytes(_ownerSign)) {
				SPVLOG_ERROR("deserialize owner sign");
				return false;
			}

			return true;
		}

		void CRCProposalTracking::SerializeSecretaryUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeNewOwnerUnsigned(stream, version);
			stream.WriteVarBytes(_newOwnerSign);
			stream.WriteUint8(_type);
			stream.WriteBytes(_secretaryGeneralOpinionHash);
			if (version >= CRCProposalTrackingVersion01)
				stream.WriteVarBytes(_secretaryGeneralOpinionData);
		}

		bool CRCProposalTracking::DeserializeSecretaryUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeNewOwnerUnsigned(stream, version))
				return false;

			if (!stream.ReadVarBytes(_newOwnerSign)) {
				SPVLOG_ERROR("deserialize new owner sign");
				return false;
			}

			uint8_t type;
			if (!stream.ReadUint8(type)) {
				SPVLOG_ERROR("deserialize type");
				return false;
			}
			_type = CRCProposalTracking::Type(type);

			if (!stream.ReadBytes(_secretaryGeneralOpinionHash)) {
				SPVLOG_ERROR("deserialize secretary opinion hash");
				return false;
			}

			if (version >= CRCProposalTrackingVersion01) {
				if (!stream.ReadVarBytes(_secretaryGeneralOpinionData)) {
					SPVLOG_ERROR("deserialize secretary opinion data");
					return false;
				}
			}

			return true;
		}

		void CRCProposalTracking::Serialize(ByteStream &stream, uint8_t version) const {
			SerializeSecretaryUnsigned(stream, version);

			stream.WriteVarBytes(_secretaryGeneralSignature);
		}

		bool CRCProposalTracking::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!DeserializeSecretaryUnsigned(istream, version)) {
				SPVLOG_ERROR("deserialize secretary unsigned");
				return false;
			}

			if (!istream.ReadVarBytes(_secretaryGeneralSignature)) {
				SPVLOG_ERROR("deserialize secretary signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposalTracking::ToJsonOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;
			j[JsonKeyProposalHash] = _proposalHash.GetHex();
			j[JsonKeyMessageHash] = _messageHash.GetHex();
			if (version >= CRCProposalTrackingVersion01)
				j[JsonKeyMessageData] = _messageData.getHex();
			j[JsonKeyStage] = _stage;
			j[JsonKeyOwnerPublicKey] = _ownerPubKey.getHex();
			j[JsonKeyNewOwnerPublicKey] = _newOwnerPubKey.getHex();
			return j;
		}

		void CRCProposalTracking::FromJsonOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_proposalHash.SetHex(j[JsonKeyProposalHash].get<std::string>());
			_messageHash.SetHex(j[JsonKeyMessageHash].get<std::string>());
			if (version >= CRCProposalTrackingVersion01) {
				std::string messageData = j[JsonKeyMessageData].get<std::string>();
				_messageData.setHex(messageData);
				ErrorChecker::CheckParam(_messageData.size() > MESSAGE_DATA_MAX_SIZE, Error::ProposalContentTooLarge, "message data size too large");
				uint256 messageHash(sha256_2(_messageData));
				ErrorChecker::CheckParam(messageHash != _messageHash, Error::ProposalHashNotMatch, "message hash not match");
			}
			_stage = j[JsonKeyStage].get<uint8_t>();
			_ownerPubKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_newOwnerPubKey.setHex(j[JsonKeyNewOwnerPublicKey].get<std::string>());
		}

		nlohmann::json CRCProposalTracking::ToJsonNewOwnerUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonOwnerUnsigned(version);
			j[JsonKeyOwnerSignature] = _ownerSign.getHex();
			return j;
		}

		void CRCProposalTracking::FromJsonNewOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonOwnerUnsigned(j, version);
			_ownerSign.setHex(j[JsonKeyOwnerSignature].get<std::string>());
		}

		nlohmann::json CRCProposalTracking::ToJsonSecretaryUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonNewOwnerUnsigned(version);
			j[JsonKeyNewOwnerSignature] = _newOwnerSign.getHex();
			j[JsonKeyType] = _type;
			j[JsonKeySecretaryGeneralOpinionHash] = _secretaryGeneralOpinionHash.GetHex();
			if (version >= CRCProposalTrackingVersion01)
				j[JsonKeySecretaryGeneralOpinionData] = _secretaryGeneralOpinionData.getHex();
			return j;
		}

		void CRCProposalTracking::FromJsonSecretaryUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonNewOwnerUnsigned(j, version);
			_newOwnerSign.setHex(j[JsonKeyNewOwnerSignature].get<std::string>());
			_type = CRCProposalTracking::Type(j[JsonKeyType].get<uint8_t>());
			_secretaryGeneralOpinionHash.SetHex(j[JsonKeySecretaryGeneralOpinionHash].get<std::string>());
			if (version >= CRCProposalTrackingVersion01) {
				std::string data = j[JsonKeySecretaryGeneralOpinionData].get<std::string>();
				_secretaryGeneralOpinionData.setHex(data);
				ErrorChecker::CheckParam(_secretaryGeneralOpinionData.size() > OPINION_DATA_MAX_SIZE, Error::ProposalContentTooLarge, "opinion data size too large");
				uint256 opinionHash(sha256_2(_secretaryGeneralOpinionData));
				ErrorChecker::CheckParam(opinionHash != _secretaryGeneralOpinionHash, Error::ProposalHashNotMatch, "opinion hash not match");
			}
		}

		nlohmann::json CRCProposalTracking::ToJson(uint8_t version) const {
			nlohmann::json j = ToJsonSecretaryUnsigned(version);

			j[JsonKeySecretaryGeneralSignature] = _secretaryGeneralSignature.getHex();
			return j;
		}

		void CRCProposalTracking::FromJson(const nlohmann::json &j, uint8_t version) {
			FromJsonSecretaryUnsigned(j, version);

			_secretaryGeneralSignature.setHex(j[JsonKeySecretaryGeneralSignature].get<std::string>());
		}

		bool CRCProposalTracking::IsValidOwnerUnsigned(uint8_t version) const {
			if (_stage > 127) {
				SPVLOG_ERROR("invalid stage");
				return false;
			}

			try {
				Key key(CTElastos, _ownerPubKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid owner pubkey");
				return false;
			}

			if (!_newOwnerPubKey.empty()) {
				try {
					Key key(CTElastos, _newOwnerPubKey);
				} catch (const std::exception &e) {
					SPVLOG_ERROR("invalid new owner pubkey");
					return false;
				}
			}

			return true;
		}

		bool CRCProposalTracking::IsValidNewOwnerUnsigned(uint8_t version) const {
			if (!IsValidOwnerUnsigned(version))
				return false;

			// verify signature of owner
			try {
				if (!Key(CTElastos, _ownerPubKey).Verify(DigestOwnerUnsigned(version), _ownerSign)) {
					SPVLOG_ERROR("verify owner sign fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("versify new owner sign exception: {}", e.what());
				return false;
			}

			return true;
		}

		bool CRCProposalTracking::IsValidSecretaryUnsigned(uint8_t version) const {
			if (!IsValidNewOwnerUnsigned(version))
				return false;

			// verify signature of new owner
			if (!_newOwnerPubKey.empty()) {
				try {
					if (!Key(CTElastos, _newOwnerPubKey).Verify(DigestNewOwnerUnsigned(version), _newOwnerSign)) {
						SPVLOG_ERROR("verify new owner sign fail");
						return false;
					}
				} catch (const std::exception &e) {
					SPVLOG_ERROR("verify new owner sign exception: {}", e.what());
					return false;
				}
			}

			if (_type >= CRCProposalTracking::Type::unknowTrackingType) {
				SPVLOG_ERROR("unknow type: {}", _type);
				return false;
			}

			return true;
		}

		bool CRCProposalTracking::IsValid(uint8_t version) const {
			if (!IsValidSecretaryUnsigned(version))
				return false;

			if (_secretaryGeneralSignature.empty()) {
				SPVLOG_ERROR("secretary signature is empty");
				return false;
			}

			return true;
		}

		IPayload &CRCProposalTracking::operator=(const IPayload &payload) {
			try {
				const CRCProposalTracking &tracking = dynamic_cast<const CRCProposalTracking &>(payload);
				operator=(tracking);
			} catch (const std::bad_cast &e) {
				SPVLOG_ERROR("payload is not instance of CRCProposalTracking");
			}
			return *this;
		}

		CRCProposalTracking &CRCProposalTracking::operator=(const CRCProposalTracking &payload) {
			_proposalHash = payload._proposalHash;
			_messageHash = payload._messageHash;
			_messageData = payload._messageData;
			_stage = payload._stage;
			_ownerPubKey = payload._ownerPubKey;
			_newOwnerPubKey = payload._newOwnerPubKey;
			_ownerSign = payload._ownerSign;
			_newOwnerSign = payload._newOwnerSign;
			_type = payload._type;
			_secretaryGeneralOpinionHash = payload._secretaryGeneralOpinionHash;
			_secretaryGeneralOpinionData = payload._secretaryGeneralOpinionData;
			_secretaryGeneralSignature = payload._secretaryGeneralSignature;
			return *this;
		}

		bool CRCProposalTracking::Equal(const IPayload &payload, uint8_t version) const {
			try {
				const CRCProposalTracking &p = dynamic_cast<const CRCProposalTracking &>(payload);
				bool equal = _proposalHash == p._proposalHash &&
							 _messageHash == p._messageHash &&
							 _stage == p._stage &&
							 _ownerPubKey == p._ownerPubKey &&
							 _newOwnerPubKey == p._newOwnerPubKey &&
							 _ownerSign == p._ownerSign &&
							 _newOwnerSign == p._newOwnerSign &&
							 _type == p._type &&
							 _secretaryGeneralOpinionHash == p._secretaryGeneralOpinionHash &&
							 _secretaryGeneralSignature == p._secretaryGeneralSignature;

				if (version >= CRCProposalTrackingVersion01)
					equal = equal && _messageData == p._messageData &&
							_secretaryGeneralOpinionData == p._secretaryGeneralOpinionData;

				return equal;
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CRCProposalTracking");
			}

			return false;
		}
	}
}