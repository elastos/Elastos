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
#include "CRCProposal.h"

#include <Common/hash.h>
#include <Common/Log.h>
#include <WalletCore/Base58.h>
#include <WalletCore/Key.h>
#include <Common/Base64.h>
#include <Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {

#define DRAFT_DATA_MAX_SIZE (1024 * 1024)

		Budget::Budget() {

		}

		Budget::Budget(Budget::Type type, uint8_t stage, const BigInt &amount) :
			_type(type), _stage(stage), _amount(amount) {

		}

		Budget::~Budget() {

		}

		Budget::Type Budget::GetType() const {
			return _type;
		}

		uint8_t Budget::GetStage() const {
			return _stage;
		}

		BigInt Budget::GetAmount() const {
			return _amount;
		}

		void Budget::Serialize(ByteStream &ostream) const {
			ostream.WriteUint8(_type);
			ostream.WriteUint8(_stage);
			ostream.WriteUint64(_amount.getUint64());
		}

		bool Budget::Deserialize(const ByteStream &istream) {
			uint8_t type;
			if (!istream.ReadUint8(type)) {
				SPVLOG_ERROR("Budget::Deserialize: read type key");
				return false;
			}
			_type = Budget::Type(type);

			if (!istream.ReadUint8(_stage)) {
				SPVLOG_ERROR("Budget::Deserialize: read stage key");
				return false;
			}

			uint64_t amount;
			if (!istream.ReadUint64(amount)) {
				SPVLOG_ERROR("Budget::Deserialize: read amount key");
				return false;
			}
			_amount.setUint64(amount);

			return true;
		}

		bool Budget::IsValid() const {
			if (_type >= Budget::Type::maxType) {
				SPVLOG_ERROR("invalid budget type: {}", _type);
				return false;
			}

			if (_stage > 127) {
				SPVLOG_ERROR("invalid budget stage", _stage);
				return false;
			}

			return true;
		}

		nlohmann::json Budget::ToJson() const {
			nlohmann::json j;
			j[JsonKeyType] = _type;
			j[JsonKeyStage] = _stage;
			j[JsonKeyAmount] = _amount.getDec();
			return j;
		}

		void Budget::FromJson(const nlohmann::json &j) {
			_type = Budget::Type(j[JsonKeyType].get<uint8_t>());
			_stage = j[JsonKeyStage].get<uint8_t>();
			_amount.setDec(j[JsonKeyAmount].get<std::string>());
		}

		bool Budget::operator==(const Budget &budget) const {
			return _type == budget._type && _stage == budget._stage && _amount == budget._amount;
		}

		CRCProposal::CRCProposal() {

		}

		CRCProposal::~CRCProposal() {

		}

		void CRCProposal::SetTpye(CRCProposal::Type type) {
			_type = type;
		}

		CRCProposal::Type CRCProposal::GetType() const {
			return _type;
		}

		void CRCProposal::SetCategoryData(const std::string &categoryData) {
			_categoryData = categoryData;
		}

		const std::string &CRCProposal::GetCategoryData() const {
			return _categoryData;
		}

		void CRCProposal::SetOwnerPublicKey(const bytes_t &publicKey) {
			_ownerPublicKey = publicKey;
		}

		const bytes_t &CRCProposal::GetOwnerPublicKey() const {
			return _ownerPublicKey;
		}

		void CRCProposal::SetCRCouncilMemberDID(const Address &crSponsorDID) {
			_crCouncilMemberDID = crSponsorDID;
		}

		const Address &CRCProposal::GetCRCouncilMemberDID() const {
			return _crCouncilMemberDID;
		}

		void CRCProposal::SetDraftHash(const uint256 &draftHash) {
			_draftHash = draftHash;
		}

		const uint256 &CRCProposal::GetDraftHash() const {
			return _draftHash;
		}

		void CRCProposal::SetDraftData(const bytes_t &draftData) {
			_draftData = draftData;
		}

		const bytes_t &CRCProposal::GetDraftData() const {
			return _draftData;
		}

		void CRCProposal::SetBudgets(const std::vector<Budget> &budgets) {
			_budgets = budgets;
		}

		const std::vector<Budget> &CRCProposal::GetBudgets() const {
			return _budgets;
		}

		void CRCProposal::SetRecipient(const Address &recipient) {
			_recipient = recipient;
		}

		const Address &CRCProposal::GetRecipient() const {
			return _recipient;
		}

		void CRCProposal::SetTargetProposalHash(const uint256 &hash) {
			_targetProposalHash = hash;
		}

		const uint256 &CRCProposal::GetTargetProposalHash() const {
			return _targetProposalHash;
		}

		void CRCProposal::SetNewRecipient(const Address &recipient) {
			_newRecipient = recipient;
		}

		const Address &CRCProposal::GetNewRecipient() const {
			return _newRecipient;
		}

		void CRCProposal::SetNewOwnerPublicKey(const bytes_t &pubkey) {
			_newOwnerPublicKey = pubkey;
		}

		const bytes_t &CRCProposal::GetNewOwnerPublicKey() const {
			return _newOwnerPublicKey;
		}

		void CRCProposal::SetSecretaryPublicKey(const bytes_t &pubkey) {
			_secretaryPublicKey = pubkey;
		}

		const bytes_t CRCProposal::GetSecretaryPublicKey() const {
			return _secretaryPublicKey;
		}

		void CRCProposal::SetSecretaryDID(const Address &did) {
			_secretaryDID = did;
		}

		const Address &CRCProposal::GetSecretaryDID() const {
			return _secretaryDID;
		}

		void CRCProposal::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		const bytes_t &CRCProposal::GetSignature() const {
			return _signature;
		}

		void CRCProposal::SetNewOwnerSignature(const bytes_t &sign) {
			_newOwnerSignature = sign;
		}

		const bytes_t &CRCProposal::GetNewOwnerSignature() const {
			return _newOwnerSignature;
		}

		void CRCProposal::SetSecretarySignature(const bytes_t &sign) {
			_secretarySignature = sign;
		}

		const bytes_t &CRCProposal::GetSecretarySignature() const {
			return _secretarySignature;
		}

		void CRCProposal::SetCRCouncilMemberSignature(const bytes_t &signature) {
			_crCouncilMemberSignature = signature;
		}

		const bytes_t &CRCProposal::GetCRCouncilMemberSignature() const {
			return _crCouncilMemberSignature;
		}

		const uint256 &CRCProposal::DigestNormalOwnerUnsigned(uint8_t version) const {
			if (_digestOwnerUnsigned == 0) {
				ByteStream stream;
				SerializeOwnerUnsigned(stream, version);
				_digestOwnerUnsigned = sha256(stream.GetBytes());
			}

			return _digestOwnerUnsigned;
		}

		const uint256 &CRCProposal::DigestNormalCRCouncilMemberUnsigned(uint8_t version) const {
			if (_digestCRCouncilMemberUnsigned == 0) {
				ByteStream stream;
				SerializeCRCouncilMemberUnsigned(stream, version);
				_digestCRCouncilMemberUnsigned = sha256(stream.GetBytes());
			}

			return _digestCRCouncilMemberUnsigned;
		}

		size_t CRCProposal::EstimateSize(uint8_t version) const {
			ByteStream stream, byteStream;
			size_t size = 0;

			size += sizeof(uint16_t);
			size += stream.WriteVarUint(_categoryData.size());
			size += _categoryData.size();
			size += stream.WriteVarUint(_ownerPublicKey.size());
			size += _ownerPublicKey.size();
			if (version >= CRCProposalVersion01) {
				size += stream.WriteVarUint(_draftData.size());
				size += _draftData.size();
			}
			size += _draftHash.size();

			switch (_type) {
				case elip:
				case normal:
					size += stream.WriteVarUint(_budgets.size());

					for (size_t i = 0; i < _budgets.size(); ++i) {
						_budgets[i].Serialize(byteStream);
					}
					size += byteStream.GetBytes().size();
					size += _recipient.ProgramHash().size();
					size += stream.WriteVarUint(_signature.size());
					size += _signature.size();
					break;

				case secretaryGeneralElection:
					size += stream.WriteVarUint(_secretaryPublicKey.size());
					size += _secretaryPublicKey.size();
					size += _secretaryDID.ProgramHash().size();
					size += stream.WriteVarUint(_secretarySignature.size());
					size += _secretarySignature.size();
					size += stream.WriteVarUint(_signature.size());
					size += _signature.size();
					break;

				case changeProposalOwner:
					size += _targetProposalHash.size();
					size += _newRecipient.ProgramHash().size();
					size += stream.WriteVarUint(_newOwnerPublicKey.size());
					size += _newOwnerPublicKey.size();
					size += stream.WriteVarUint(_signature.size());
					size += _signature.size();
					size += stream.WriteVarUint(_newOwnerSignature.size());
					size += _newOwnerSignature.size();
					break;

				case terminateProposal:
					size += stream.WriteVarUint(_signature.size());
					size += _signature.size();
					size += _targetProposalHash.size();
					break;

				default:
					break;
			}

			size += _crCouncilMemberDID.ProgramHash().size();
			size += stream.WriteVarUint(_crCouncilMemberSignature.size());
			size += _crCouncilMemberSignature.size();

			return size;
		}

		// normal or elip
		void CRCProposal::SerializeOwnerUnsigned(ByteStream &stream, uint8_t version) const {
			stream.WriteUint16(_type);
			stream.WriteVarString(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			if (version >= CRCProposalVersion01)
				stream.WriteVarBytes(_draftData);
			stream.WriteVarUint(_budgets.size());
			for (size_t i = 0; i < _budgets.size(); ++i)
				_budgets[i].Serialize(stream);
			stream.WriteBytes(_recipient.ProgramHash());
		}

		bool CRCProposal::DeserializeOwnerUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize categoryData");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize owner PublicKey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize draftHash");
				return false;
			}

			if (version >= CRCProposalVersion01) {
				if (!stream.ReadVarBytes(_draftData)) {
					SPVLOG_ERROR("deserialize draftdata");
					return false;
				}
			}

			uint64_t count = 0;
			if (!stream.ReadVarUint(count)) {
				SPVLOG_ERROR("deserialize budgets size");
				return false;
			}
			_budgets.resize(count);
			for (size_t i = 0; i < count; ++i) {
				if (!_budgets[i].Deserialize(stream)) {
					SPVLOG_ERROR("deserialize bugets");
					return false;
				}
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize recipient");
				return false;
			}
			_recipient = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeCRCouncilMemberUnsigned(ByteStream &ostream, uint8_t version) const {
			SerializeOwnerUnsigned(ostream, version);

			ostream.WriteVarBytes(_signature);

			ostream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeCRCouncilMemberUnsigned(const ByteStream &istream, uint8_t version) {
			if (!DeserializeOwnerUnsigned(istream, version)) {
				SPVLOG_ERROR("deserialize unsigned");
				return false;
			}

			if (!istream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize signature");
				return false;
			}

			uint168 programHash;
			if (!istream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeNormalOrELIP(ByteStream &stream, uint8_t version) const {
			SerializeCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeNormalOrELIP(const ByteStream &stream, uint8_t version) {
			if (!DeserializeCRCouncilMemberUnsigned(stream, version)) {
				SPVLOG_ERROR("CRCProposal deserialize crc unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("CRCProposal deserialize crc signature");
				return false;
			}

			return true;
		}

		// change owner
		void CRCProposal::SerializeChangeOwnerUnsigned(ByteStream &stream, uint8_t version) const {
			uint16_t type = _type;
			stream.WriteUint16(type);

			stream.WriteVarString(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			if (version >= CRCProposalVersion01)
				stream.WriteVarBytes(_draftData);
			stream.WriteBytes(_targetProposalHash);
			stream.WriteBytes(_newRecipient.ProgramHash());
			stream.WriteVarBytes(_newOwnerPublicKey);
		}

		bool CRCProposal::DeserializeChangeOwnerUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize categoryData");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize owner PublicKey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize draftHash");
				return false;
			}

			if (version >= CRCProposalVersion01) {
				if (!stream.ReadVarBytes(_draftData)) {
					SPVLOG_ERROR("deserialize draftData");
					return false;
				}
			}

			if (!stream.ReadBytes(_targetProposalHash)) {
				SPVLOG_ERROR("deserialize target proposal hash");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize new recipient");
				return false;
			}
			_newRecipient = Address(programHash);

			if (!stream.ReadVarBytes(_newOwnerPublicKey)) {
				SPVLOG_ERROR("deserialize new owner PublicKey");
				return false;
			}

			return true;
		}

		void CRCProposal::SerializeChangeOwnerCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeChangeOwnerUnsigned(stream, version);

			stream.WriteVarBytes(_signature);
			stream.WriteVarBytes(_newOwnerSignature);
			stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeChangeOwnerCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeChangeOwnerUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize change owner unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize change owner signature");
				return false;
			}

			if (!stream.ReadVarBytes(_newOwnerSignature)) {
				SPVLOG_ERROR("deserialize change owner new owner signature");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeChangeOwner(ByteStream &stream, uint8_t version) const {
			SerializeChangeOwnerCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeChangeOwner(const ByteStream &stream, uint8_t version) {
			if (!DeserializeChangeOwnerCRCouncilMemberUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize change owner cr council member unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("deserialize change owner cr council member signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJsonChangeOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;

			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			if (version >= CRCProposalVersion01)
				j[JsonKeyDraftData] = Base64::Encode(_draftData);
			j[JsonKeyTargetProposalHash] = _targetProposalHash.GetHex();
			j[JsonKeyNewRecipient] = _newRecipient.String();
			j[JsonKeyNewOwnerPublicKey] = _newOwnerPublicKey.getHex();

			return j;
		}

		void CRCProposal::FromJsonChangeOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			if (version >= CRCProposalVersion01) {
				std::string draftData = j[JsonKeyDraftData].get<std::string>();
				_draftData = Base64::Decode(draftData);
				ErrorChecker::CheckParam(_draftData.size() > DRAFT_DATA_MAX_SIZE, Error::ProposalContentTooLarge, "proposal origin content too large");
				uint256 draftHash(sha256_2(_draftData));
				ErrorChecker::CheckParam(draftHash != _draftHash, Error::ProposalHashNotMatch, "proposal hash not match");
			}
			_targetProposalHash.SetHex(j[JsonKeyTargetProposalHash].get<std::string>());
			_newRecipient = Address(j[JsonKeyNewRecipient].get<std::string>());
			_newOwnerPublicKey.setHex(j[JsonKeyNewOwnerPublicKey].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonChangeOwnerUnsigned(version);

			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeyNewOwnerSignature] = _newOwnerSignature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();

			return j;
		}

		void CRCProposal::FromJsonChangeOwnerCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonChangeOwnerUnsigned(j, version);

			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_newOwnerSignature.setHex(j[JsonKeyNewOwnerSignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		bool CRCProposal::IsValidChangeOwnerUnsigned(uint8_t version) const {
			if (_type != changeProposalOwner) {
				SPVLOG_ERROR("invalid type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(CTElastos, _ownerPublicKey);
				Key key1(CTElastos, _newOwnerPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid pubkey");
				return false;
			}

			if (_draftHash == 0 || _targetProposalHash == 0) {
				SPVLOG_ERROR("invalid hash");
				return false;
			}

			if (!_newRecipient.Valid()) {
				SPVLOG_ERROR("invalid new recipient");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidChangeOwnerUnsigned(version)) {
				return false;
			}

			try {
				if (!Key(CTElastos, _ownerPublicKey).Verify(DigestChangeOwnerUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify signature fail");
					return false;
				}

				if (!Key(CTElastos, _newOwnerPublicKey).Verify(DigestChangeOwnerUnsigned(version), _newOwnerSignature)) {
					SPVLOG_ERROR("verify new owner signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr council member did");
				return false;
			}

			return true;
		}

		const uint256 &CRCProposal::DigestChangeOwnerUnsigned(uint8_t version) const {
			if (_digestChangeOwnerUnsigned == 0) {
				ByteStream stream;
				SerializeChangeOwnerUnsigned(stream, version);
				_digestChangeOwnerUnsigned = sha256(stream.GetBytes());
			}

			return _digestChangeOwnerUnsigned;
		}

		const uint256 &CRCProposal::DigestChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const {
			if (_digestChangeOwnerCRCouncilMemberUnsigned == 0) {
				ByteStream stream;
				SerializeChangeOwnerCRCouncilMemberUnsigned(stream, version);
				_digestChangeOwnerCRCouncilMemberUnsigned = sha256(stream.GetBytes());
			}

			return _digestChangeOwnerCRCouncilMemberUnsigned;
		}

		// terminate proposal
		void CRCProposal::SerializeTerminateProposalUnsigned(ByteStream &stream, uint8_t version) const {
			uint16_t type = _type;
			stream.WriteUint16(type);
			stream.WriteVarString(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			if (version >= CRCProposalVersion01)
				stream.WriteVarBytes(_draftData);
			stream.WriteBytes(_targetProposalHash);
		}

		bool CRCProposal::DeserializeTerminateProposalUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize terminate proposal category data");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize terminate proposal owner pubkey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize terminate proposal draft hash");
				return false;
			}

			if (version >= CRCProposalVersion01) {
				if (!stream.ReadVarBytes(_draftData)) {
					SPVLOG_ERROR("deserialize terminate proposal draftData");
					return false;
				}
			}

			if (!stream.ReadBytes(_targetProposalHash)) {
				SPVLOG_ERROR("deserialize terminate proposal target proposal hash");
				return false;
			}

			return true;
		}

		void CRCProposal::SerializeTerminateProposalCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeTerminateProposalUnsigned(stream, version);

			stream.WriteVarBytes(_signature);
			stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeTerminateProposalCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeTerminateProposalUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize terminate proposal unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize terminate proposal signature");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeTerminateProposal(ByteStream &stream, uint8_t version) const {
			SerializeTerminateProposalCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeTerminateProposal(const ByteStream &stream, uint8_t version) {
			if (!DeserializeTerminateProposalCRCouncilMemberUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize terminate proposal cr council member unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("deserialize change owner cr council member signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJsonTerminateProposalOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;

			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			if (version >= CRCProposalVersion01)
				j[JsonKeyDraftData] = Base64::Encode(_draftData);
			j[JsonKeyTargetProposalHash] = _targetProposalHash.GetHex();

			return j;
		}

		void CRCProposal::FromJsonTerminateProposalOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			if (version >= CRCProposalVersion01) {
				std::string draftData = j[JsonKeyDraftData].get<std::string>();
				_draftData = Base64::Decode(draftData);
				ErrorChecker::CheckParam(_draftData.size() > DRAFT_DATA_MAX_SIZE, Error::ProposalContentTooLarge, "proposal origin content too large");
				uint256 draftHash(sha256_2(_draftData));
				ErrorChecker::CheckParam(draftHash != _draftHash, Error::ProposalHashNotMatch, "proposal hash not match");
			}
			_targetProposalHash.SetHex(j[JsonKeyTargetProposalHash].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonTerminateProposalOwnerUnsigned(version);

			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();

			return j;
		}

		void CRCProposal::FromJsonTerminateProposalCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonTerminateProposalOwnerUnsigned(j, version);

			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		bool CRCProposal::IsValidTerminateProposalOwnerUnsigned(uint8_t version) const {
			if (_type != terminateProposal) {
				SPVLOG_ERROR("invalid type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(CTElastos, _ownerPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid pubkey");
				return false;
			}

			if (_draftHash == 0 || _targetProposalHash == 0) {
				SPVLOG_ERROR("invalid hash");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidTerminateProposalOwnerUnsigned(version)) {
				SPVLOG_ERROR("terminate proposal unsigned is not valid");
				return false;
			}

			try {
				if (!Key(CTElastos, _ownerPublicKey).Verify(DigestTerminateProposalOwnerUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr council member did");
				return false;
			}

			return true;
		}

		const uint256 &CRCProposal::DigestTerminateProposalOwnerUnsigned(uint8_t version) const {
			if (_digestTerminateProposalOwnerUnsigned == 0) {
				ByteStream stream;
				SerializeTerminateProposalUnsigned(stream, version);
				_digestTerminateProposalOwnerUnsigned = sha256(stream.GetBytes());
			}

			return _digestTerminateProposalOwnerUnsigned;
		}

		const uint256 &CRCProposal::DigestTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const {
			if (_digestTerminateProposalCRCouncilMemberUnsigned == 0) {
				ByteStream stream;
				SerializeTerminateProposalCRCouncilMemberUnsigned(stream, version);
				_digestTerminateProposalCRCouncilMemberUnsigned = sha256(stream.GetBytes());
			}

			return _digestTerminateProposalCRCouncilMemberUnsigned;
		}

		// change secretary general
		void CRCProposal::SerializeSecretaryElectionUnsigned(ByteStream &stream, uint8_t version) const {
			uint16_t type = _type;
			stream.WriteUint16(type);
			stream.WriteVarString(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			if (version >= CRCProposalVersion01)
				stream.WriteVarBytes(_draftData);
			stream.WriteVarBytes(_secretaryPublicKey);
			stream.WriteBytes(_secretaryDID.ProgramHash());
		}

		bool CRCProposal::DeserializeSecretaryElectionUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize category data");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize owner pubkey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize draft hash");
				return false;
			}

			if (version >= CRCProposalVersion01) {
				if (!stream.ReadVarBytes(_draftData)) {
					SPVLOG_ERROR("deserialize draft data");
					return false;
				}
			}

			if (!stream.ReadVarBytes(_secretaryPublicKey)) {
				SPVLOG_ERROR("deserialize secretary pubkey");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_secretaryDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeSecretaryElectionCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeSecretaryElectionUnsigned(stream, version);

			stream.WriteVarBytes(_signature);
			stream.WriteVarBytes(_secretarySignature);
			stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeSecretaryElectionCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeSecretaryElectionUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize change secretary secretary unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize signature");
				return false;
			}

			if (!stream.ReadVarBytes(_secretarySignature)) {
				SPVLOG_ERROR("deserialize secretary signature");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize cr council mem did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeSecretaryElection(ByteStream &stream, uint8_t version) const {
			SerializeSecretaryElectionCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeSecretaryElection(const ByteStream &stream, uint8_t version) {
			if (!DeserializeSecretaryElectionCRCouncilMemberUnsigned(stream, version)) {
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("deserialize change secretary cr council member signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJsonSecretaryElectionUnsigned(uint8_t version) const {
			nlohmann::json j;

			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			if (version >= CRCProposalVersion01)
				j[JsonKeyDraftData] = Base64::Encode(_draftData);
			j[JsonKeySecretaryPublicKey] = _secretaryPublicKey.getHex();
			j[JsonKeySecretaryDID] = _secretaryDID.String();

			return j;
		}

		void CRCProposal::FromJsonSecretaryElectionUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			if (version >= CRCProposalVersion01) {
				std::string draftData = j[JsonKeyDraftData].get<std::string>();
				_draftData = Base64::Decode(draftData);
				ErrorChecker::CheckParam(_draftData.size() > DRAFT_DATA_MAX_SIZE, Error::ProposalContentTooLarge, "proposal origin content too large");
				uint256 draftHash(sha256_2(_draftData));
				ErrorChecker::CheckParam(draftHash != _draftHash, Error::ProposalHashNotMatch, "proposal hash not match");
			}
			_secretaryPublicKey.setHex(j[JsonKeySecretaryPublicKey].get<std::string>());
			_secretaryDID = Address(j[JsonKeySecretaryDID].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j;

			j = ToJsonSecretaryElectionUnsigned(version);
			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeySecretarySignature] = _secretarySignature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();

			return j;
		}

		void CRCProposal::FromJsonSecretaryElectionCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonSecretaryElectionUnsigned(j, version);
			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_secretarySignature.setHex(j[JsonKeySecretarySignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		bool CRCProposal::IsValidSecretaryElectionUnsigned(uint8_t version) const {
			if (_type != secretaryGeneralElection) {
				SPVLOG_ERROR("invalid type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(CTElastos, _ownerPublicKey);
				Key key1(CTElastos, _secretaryPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid ...");
				return false;
			}

			if (!_secretaryDID.Valid()) {
				SPVLOG_ERROR("invalid secretary did");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidSecretaryElectionUnsigned(version)) {
				SPVLOG_ERROR("secretary election secretary unsigned not valid");
				return false;
			}

			try {
				if (!Key(CTElastos, _ownerPublicKey).Verify(DigestSecretaryElectionUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify owner signature fail");
					return false;
				}
				if (!Key(CTElastos, _secretaryPublicKey).Verify(DigestSecretaryElectionUnsigned(version), _secretarySignature)) {
					SPVLOG_ERROR("verify secretary signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr committee did");
				return false;
			}

			return true;
		}

		const uint256 &CRCProposal::DigestSecretaryElectionUnsigned(uint8_t version) const {
			if (_digestSecretaryElectionUnsigned == 0) {
				ByteStream stream;
				SerializeSecretaryElectionUnsigned(stream, version);
				_digestSecretaryElectionUnsigned = sha256(stream.GetBytes());
			}

			return _digestSecretaryElectionUnsigned;
		}

		const uint256 &CRCProposal::DigestSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const {
			if (_digestSecretaryElectionCRCouncilMemberUnsigned == 0) {
				ByteStream stream;
				SerializeSecretaryElectionCRCouncilMemberUnsigned(stream, version);
				_digestSecretaryElectionCRCouncilMemberUnsigned = sha256(stream.GetBytes());
			}

			return _digestSecretaryElectionCRCouncilMemberUnsigned;
		}

        void CRCProposal::SerializeReserveCustomIDUnsigned(ByteStream &stream, uint8_t version) const {
            uint16_t type = _type;
            stream.WriteUint16(type);
            stream.WriteVarString(_categoryData);
            stream.WriteVarBytes(_ownerPublicKey);
            stream.WriteBytes(_draftHash);
            if (version >= CRCProposalVersion01)
                stream.WriteVarBytes(_draftData);
            stream.WriteVarUint((uint64_t)_reservedCustomIDList.size());
            for (const std::string &reservedCustomID : _reservedCustomIDList)
                stream.WriteVarString(reservedCustomID);
		}

        bool CRCProposal::DeserializeReserveCustomIDUnsigned(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadVarString(_categoryData)) {
                SPVLOG_ERROR("deserialize reserved custom id category data");
                return false;
            }

            if (!stream.ReadVarBytes(_ownerPublicKey)) {
                SPVLOG_ERROR("deserialize reserved custom id owner pubkey");
                return false;
            }

            if (!stream.ReadBytes(_draftHash)) {
                SPVLOG_ERROR("deserialize reserved custom id draft hash");
                return false;
            }

            if (version >= CRCProposalVersion01) {
                if (!stream.ReadVarBytes(_draftData)) {
                    SPVLOG_ERROR("deserialize reserved custom id draft data");
                    return false;
                }
            }

            uint64_t size = 0;
            if (!stream.ReadVarUint(size)) {
                SPVLOG_ERROR("deserialize reserved custom id list size");
                return false;
            }
            for (size_t i = 0; i < size; ++i) {
                std::string reservedCustomID;
                if (!stream.ReadVarString(reservedCustomID)) {
                    SPVLOG_ERROR("deserialize reserved custom id list[{}]", i);
                    return false;
                }
                _reservedCustomIDList.push_back(reservedCustomID);
            }

            return true;
		}

        void CRCProposal::SerializeReserveCustomIDCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
            SerializeReserveCustomIDUnsigned(stream, version);
            stream.WriteVarBytes(_signature);
            stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

        bool CRCProposal::DeserializeReserveCustomIDCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
            if (!DeserializeReserveCustomIDUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                SPVLOG_ERROR("deserialize reserved custom id signature");
                return false;
            }

            uint168 programHash;
            if (!stream.ReadBytes(programHash)) {
                SPVLOG_ERROR("deserialize cr council mem did");
                return false;
            }
            _crCouncilMemberDID = Address(programHash);

            return true;
		}

        void CRCProposal::SerializeReserveCustomID(ByteStream &stream, uint8_t version) const {
            SerializeReserveCustomIDCRCouncilMemberUnsigned(stream, version);
            stream.WriteVarBytes(_crCouncilMemberSignature);
		}

        bool CRCProposal::DeserializeReserveCustomID(const ByteStream &stream, uint8_t version) {
            if (!DeserializeReserveCustomIDCRCouncilMemberUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
                SPVLOG_ERROR("deserialize reserved custom id council member sign");
                return false;
            }

            return true;
		}

        nlohmann::json CRCProposal::ToJsonReserveCustomIDOwnerUnsigned(uint8_t version) const {
            nlohmann::json j;

            j[JsonKeyType] = _type;
            j[JsonKeyCategoryData] = _categoryData;
            j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
            j[JsonKeyDraftHash] = _draftHash.GetHex();
            if (version >= CRCProposalVersion01)
                j[JsonKeyDraftData] = Base64::Encode(_draftData);
            j[JsonKeyReservedCustomIDList] = _reservedCustomIDList;

            return j;
		}

        void CRCProposal::FromJsonReserveCustomIDOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
            _type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
            _categoryData = j[JsonKeyCategoryData].get<std::string>();
            _ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
            _draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
            if (version >= CRCProposalVersion01) {
                std::string draftData = j[JsonKeyDraftData].get<std::string>();
                _draftData = Base64::Decode(draftData);
                ErrorChecker::CheckParam(_draftData.size() > DRAFT_DATA_MAX_SIZE, Error::ProposalContentTooLarge, "proposal origin content too large");
                uint256 draftHash(sha256_2(_draftData));
                ErrorChecker::CheckParam(draftHash != _draftHash, Error::ProposalHashNotMatch, "proposal hash not match");
            }
            _reservedCustomIDList = j[JsonKeyReservedCustomIDList].get<std::vector<std::string>>();
		}

        nlohmann::json CRCProposal::ToJsonReserveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
            nlohmann::json j = ToJsonReserveCustomIDOwnerUnsigned(version);
            j[JsonKeySignature] = _signature.getHex();
            j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
            return j;
		}

        void CRCProposal::FromJsonReserveCustomIDCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
            FromJsonReserveCustomIDOwnerUnsigned(j, version);
            _signature.setHex(j[JsonKeySignature].get<std::string>());
            _crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

        bool CRCProposal::IsValidReserveCustomIDOwnerUnsigned(uint8_t version) const {
            if (_type != reserveCustomID) {
                SPVLOG_ERROR("invalid type: {}", _type);
                return false;
            }

            if (_categoryData.size() > 4096) {
                SPVLOG_ERROR("category data exceed 4096 bytes");
                return false;
            }

            try {
                Key key(CTElastos, _ownerPublicKey);
            } catch (const std::exception &e) {
                SPVLOG_ERROR("invalid reserve custom id pubkey");
                return false;
            }

            return true;
		}

        bool CRCProposal::IsValidReserveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
            if (!IsValidReserveCustomIDOwnerUnsigned(version))
                return false;

            try {
                if (!Key(CTElastos, _ownerPublicKey).Verify(DigestReserveCustomIDOwnerUnsigned(version), _signature)) {
                    SPVLOG_ERROR("reserve custom id verify owner signature fail");
                    return false;
                }
            } catch (const std::exception &e) {
                SPVLOG_ERROR("verify signature exception: {}", e.what());
                return false;
            }

            if (!_crCouncilMemberDID.Valid()) {
                SPVLOG_ERROR("invalid cr committee did");
                return false;
            }

            return true;
		}

        const uint256 &CRCProposal::DigestReserveCustomIDOwnerUnsigned(uint8_t version) const {
            if (_digestReserveCustomIDOwnerUnsigned == 0) {
                ByteStream stream;
                SerializeReserveCustomIDUnsigned(stream, version);
                _digestReserveCustomIDOwnerUnsigned = sha256(stream.GetBytes());
            }

            return _digestReserveCustomIDOwnerUnsigned;
		}

        const uint256 &CRCProposal::DigestReserveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
            if (_digestReserveCustomIDCRCouncilMemberUnsigned == 0) {
                ByteStream stream;
                SerializeReserveCustomIDCRCouncilMemberUnsigned(stream, version);
                _digestReserveCustomIDCRCouncilMemberUnsigned = sha256(stream.GetBytes());
            }

            return _digestReserveCustomIDCRCouncilMemberUnsigned;
		}

        // ReceiveCustomID
        void CRCProposal::SerializeReceiveCustomIDUnsigned(ByteStream &stream, uint8_t version) const {
            uint16_t type = _type;
            stream.WriteUint16(type);
            stream.WriteVarString(_categoryData);
            stream.WriteVarBytes(_ownerPublicKey);
            stream.WriteBytes(_draftHash);
            if (version >= CRCProposalVersion01)
                stream.WriteVarBytes(_draftData);
            stream.WriteVarUint((uint64_t)_receivedCustomIDList.size());
            for (const std::string &receivedCustomID : _receivedCustomIDList)
                stream.WriteVarString(receivedCustomID);
            stream.WriteBytes(_receiverDID.ProgramHash());
		}

        bool CRCProposal::DeserializeReceiveCustomIDUnsigned(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadVarString(_categoryData)) {
                SPVLOG_ERROR("deserialize receive custom category data");
                return false;
            }

            if (!stream.ReadVarBytes(_ownerPublicKey)) {
                SPVLOG_ERROR("deserialize receive custom owner pubkey");
                return false;
            }

            if (!stream.ReadBytes(_draftHash)) {
                SPVLOG_ERROR("deserialize receive custom draft hash");
                return false;
            }

            if (version >= CRCProposalVersion01) {
                if (!stream.ReadVarBytes(_draftData)) {
                    SPVLOG_ERROR("deserialize receive custom draft data");
                    return false;
                }
            }

            uint64_t size = 0;
            if (!stream.ReadVarUint(size)) {
                SPVLOG_ERROR("deserialize receive custom id list size");
                return false;
            }
            for (size_t i = 0; i < size; ++i) {
                std::string receivedCustomID;
                if (!stream.ReadVarString(receivedCustomID)) {
                    SPVLOG_ERROR("deserialize receive custom id list[{}]", i);
                    return false;
                }
                _receivedCustomIDList.push_back(receivedCustomID);
            }

            uint168 programHash;
            if (!stream.ReadBytes(programHash)) {
                SPVLOG_ERROR("deserialize receiver did");
                return false;
            }
            _receiverDID = Address(programHash);

            return true;
		}

        void CRCProposal::SerializeReceiveCustomIDCRCCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
            SerializeReceiveCustomIDUnsigned(stream, version);
            stream.WriteVarBytes(_signature);
            stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

        bool CRCProposal::DeserializeReceiveCustomIDCRCCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
            if (!DeserializeReceiveCustomIDUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                SPVLOG_ERROR("deserialize reserved custom id signature");
                return false;
            }

            uint168 programHash;
            if (!stream.ReadBytes(programHash)) {
                SPVLOG_ERROR("deserialize cr council mem did");
                return false;
            }
            _crCouncilMemberDID = Address(programHash);

            return true;
		}

        void CRCProposal::SerializeReceiveCustomID(ByteStream &stream, uint8_t version) const {
            SerializeReceiveCustomIDCRCCouncilMemberUnsigned(stream, version);
            stream.WriteVarBytes(_crCouncilMemberSignature);
		}

        bool CRCProposal::DeserializeReceiveCustomID(const ByteStream &stream, uint8_t version) {
            if (!DeserializeReceiveCustomIDCRCCouncilMemberUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
                SPVLOG_ERROR("deserialize receive custom id council member sign");
                return false;
            }

            return true;
		}

        nlohmann::json CRCProposal::ToJsonReceiveCustomIDOwnerUnsigned(uint8_t version) const {
            nlohmann::json j;

            j[JsonKeyType] = _type;
            j[JsonKeyCategoryData] = _categoryData;
            j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
            j[JsonKeyDraftHash] = _draftHash.GetHex();
            if (version >= CRCProposalVersion01)
                j[JsonKeyDraftData] = Base64::Encode(_draftData);
            j[JsonKeyReceivedCustomIDList] = _receivedCustomIDList;
            j[JsonKeyReceiverDID] = _receiverDID.String();

            return j;
		}

        void CRCProposal::FromJsonReceiveCustomIDOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
            _type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
            _categoryData = j[JsonKeyCategoryData].get<std::string>();
            _ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
            _draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
            if (version >= CRCProposalVersion01) {
                std::string draftData = j[JsonKeyDraftData].get<std::string>();
                _draftData = Base64::Decode(draftData);
                ErrorChecker::CheckParam(_draftData.size() > DRAFT_DATA_MAX_SIZE, Error::ProposalContentTooLarge, "proposal origin content too large");
                uint256 draftHash(sha256_2(_draftData));
                ErrorChecker::CheckParam(draftHash != _draftHash, Error::ProposalHashNotMatch, "proposal hash not match");
            }
            _receivedCustomIDList = j[JsonKeyReservedCustomIDList].get<std::vector<std::string>>();
            _receiverDID = Address(j[JsonKeyReceiverDID].get<std::string>());
		}

        nlohmann::json CRCProposal::ToJsonReceiveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
            nlohmann::json j = ToJsonReceiveCustomIDOwnerUnsigned(version);
            j[JsonKeySignature] = _signature.getHex();
            j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
            return j;
		}

        void CRCProposal::FromJsonReceiveCustomIDCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
            FromJsonReceiveCustomIDOwnerUnsigned(j, version);
            _signature.setHex(j[JsonKeySignature].get<std::string>());
            _crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

        bool CRCProposal::IsValidReceiveCustomIDOwnerUnsigned(uint8_t version) const {
            if (_type != receiveCustomID) {
                SPVLOG_ERROR("invalid type: {}", _type);
                return false;
            }

            if (_categoryData.size() > 4096) {
                SPVLOG_ERROR("category data exceed 4096 bytes");
                return false;
            }

            try {
                Key key(CTElastos, _ownerPublicKey);
            } catch (const std::exception &e) {
                SPVLOG_ERROR("invalid reserve custom id pubkey");
                return false;
            }

            return true;
		}

        bool CRCProposal::IsValidReceiveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
		    if (!IsValidReceiveCustomIDOwnerUnsigned(version))
		        return false;

            try {
                if (!Key(CTElastos, _ownerPublicKey).Verify(DigestReceiveCustomIDOwnerUnsigned(version), _signature)) {
                    SPVLOG_ERROR("receive custom id verify owner signature fail");
                    return false;
                }
            } catch (const std::exception &e) {
                SPVLOG_ERROR("verify signature exception: {}", e.what());
                return false;
            }

            if (!_crCouncilMemberDID.Valid()) {
                SPVLOG_ERROR("invalid cr committee did");
                return false;
            }

            return true;
		}

        const uint256 &CRCProposal::DigestReceiveCustomIDOwnerUnsigned(uint8_t version) const {
            if (_digestReceiveCustomIDOwnerUnsigned == 0) {
                ByteStream stream;
                SerializeReceiveCustomIDUnsigned(stream, version);
                _digestReceiveCustomIDOwnerUnsigned = sha256(stream.GetBytes());
            }

            return _digestReceiveCustomIDOwnerUnsigned;
		}

        const uint256 &CRCProposal::DigestReceiveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
            if (_digestReceiveCustomIDCRCouncilMemberUnsigned == 0) {
                ByteStream stream;
                SerializeReceiveCustomIDCRCCouncilMemberUnsigned(stream, version);
                _digestReceiveCustomIDCRCouncilMemberUnsigned = sha256(stream.GetBytes());
            }

            return _digestReceiveCustomIDCRCouncilMemberUnsigned;
		}

        // ChangeCustomIDFee
        void CRCProposal::SerializeChangeCustomIDFeeUnsigned(ByteStream &stream, uint8_t version) const {
            uint16_t type = _type;
            stream.WriteUint16(type);
            stream.WriteVarString(_categoryData);
            stream.WriteVarBytes(_ownerPublicKey);
            stream.WriteBytes(_draftHash);
            if (version >= CRCProposalVersion01)
                stream.WriteVarBytes(_draftData);
            stream.WriteUint64(_rateOfCustomIDFee);
		}

        bool CRCProposal::DeserializeChangeCustomIDFeeUnsigned(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadVarString(_categoryData)) {
                SPVLOG_ERROR("deserialize change custom id category data");
                return false;
            }

            if (!stream.ReadVarBytes(_ownerPublicKey)) {
                SPVLOG_ERROR("deserialize change custom id owner pubkey");
                return false;
            }

            if (!stream.ReadBytes(_draftHash)) {
                SPVLOG_ERROR("deserialize change custom id draft hash");
                return false;
            }

            if (version >= CRCProposalVersion01) {
                if (!stream.ReadVarBytes(_draftData)) {
                    SPVLOG_ERROR("deserialize change custom id draft data");
                    return false;
                }
            }

            if (!stream.ReadUint64(_rateOfCustomIDFee)) {
                SPVLOG_ERROR("deserialize change custom id fee");
                return false;
            }

            return true;
		}

        void CRCProposal::SerializeChangeCustomIDFeeCRCCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
            SerializeChangeCustomIDFeeUnsigned(stream, version);
            stream.WriteVarBytes(_signature);
            stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

        bool CRCProposal::DeserializeChangeCustomIDFeeCRCCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
            if (!DeserializeChangeCustomIDFeeUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                SPVLOG_ERROR("deserialize change custom id fee signature");
                return false;
            }

            uint168 programHash;
            if (!stream.ReadBytes(programHash)) {
                SPVLOG_ERROR("deserialize change custom id fee cr council mem did");
                return false;
            }
            _crCouncilMemberDID = Address(programHash);

            return true;
		}

        void CRCProposal::SerializeChangeCustomIDFee(ByteStream &stream, uint8_t version) const {
            SerializeChangeCustomIDFeeCRCCouncilMemberUnsigned(stream, version);
            stream.WriteVarBytes(_crCouncilMemberSignature);
		}

        bool CRCProposal::DeserializeChangeCustomIDFee(const ByteStream &stream, uint8_t version) {
            if (!DeserializeChangeCustomIDFeeCRCCouncilMemberUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
                SPVLOG_ERROR("deserialize change custom id fee council mem sign");
                return false;
            }

            return true;
		}

        nlohmann::json CRCProposal::ToJsonChangeCustomIDFeeOwnerUnsigned(uint8_t version) const {
            nlohmann::json j;

            j[JsonKeyType] = _type;
            j[JsonKeyCategoryData] = _categoryData;
            j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
            j[JsonKeyDraftHash] = _draftHash.GetHex();
            if (version >= CRCProposalVersion01)
                j[JsonKeyDraftData] = Base64::Encode(_draftData);
            j[JsonKeyRateOfCustomIDFee] = _rateOfCustomIDFee;

            return j;
		}

        void CRCProposal::FromJsonChangeCustomIDFeeOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
            _type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
            _categoryData = j[JsonKeyCategoryData].get<std::string>();
            _ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
            _draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
            if (version >= CRCProposalVersion01) {
                std::string draftData = j[JsonKeyDraftData].get<std::string>();
                _draftData = Base64::Decode(draftData);
                ErrorChecker::CheckParam(_draftData.size() > DRAFT_DATA_MAX_SIZE, Error::ProposalContentTooLarge, "proposal origin content too large");
                uint256 draftHash(sha256_2(_draftData));
                ErrorChecker::CheckParam(draftHash != _draftHash, Error::ProposalHashNotMatch, "proposal hash not match");
            }
            _rateOfCustomIDFee = j[JsonKeyRateOfCustomIDFee];
		}

        nlohmann::json CRCProposal::ToJsonChangeCustomIDFeeCRCouncilMemberUnsigned(uint8_t version) const {
		    nlohmann::json j = ToJsonChangeCustomIDFeeOwnerUnsigned(version);
            j[JsonKeySignature] = _signature.getHex();
            j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
		    return j;
		}

        void CRCProposal::FromJsonChangeCustomIDFeeCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
            FromJsonChangeCustomIDFeeOwnerUnsigned(j, version);
            _signature.setHex(j[JsonKeySignature].get<std::string>());
            _crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

        bool CRCProposal::IsValidChangeCustomIDFeeOwnerUnsigned(uint8_t version) const {
            if (_type != changeCustomIDFee) {
                SPVLOG_ERROR("invalid type: {}", _type);
                return false;
            }

            if (_categoryData.size() > 4096) {
                SPVLOG_ERROR("category data exceed 4096 bytes");
                return false;
            }

            try {
                Key key(CTElastos, _ownerPublicKey);
            } catch (const std::exception &e) {
                SPVLOG_ERROR("invalid reserve custom id pubkey");
                return false;
            }

            return true;
		}

        bool CRCProposal::IsValidChangeCustomIDFeeCRCouncilMemberUnsigned(uint8_t version) const {
            if (!IsValidChangeCustomIDFeeOwnerUnsigned(version))
                return false;

            try {
                if (!Key(CTElastos, _ownerPublicKey).Verify(DigestChangeCustomIDFeeOwnerUnsigned(version), _signature)) {
                    SPVLOG_ERROR("change custom id fee verify owner signature fail");
                    return false;
                }
            } catch (const std::exception &e) {
                SPVLOG_ERROR("verify signature exception: {}", e.what());
                return false;
            }

            if (!_crCouncilMemberDID.Valid()) {
                SPVLOG_ERROR("invalid cr committee did");
                return false;
            }

            return true;
		}

        const uint256 &CRCProposal::DigestChangeCustomIDFeeOwnerUnsigned(uint8_t version) const {
            if (_digestChangeCustomIDFeeOwnerUnsigned == 0) {
                ByteStream stream;
                SerializeChangeCustomIDFeeUnsigned(stream, version);
                _digestChangeCustomIDFeeOwnerUnsigned = sha256(stream.GetBytes());
            }

            return _digestChangeCustomIDFeeOwnerUnsigned;
		}

        const uint256 &CRCProposal::DigestChangeCustomIDFeeCRCouncilMemberUnsigned(uint8_t version) const {
            if (_digestChangeCustomIDFeeCRCouncilMemberUnsigned == 0) {
                ByteStream stream;
                SerializeChangeCustomIDFeeCRCCouncilMemberUnsigned(stream, version);
                _digestChangeCustomIDFeeCRCouncilMemberUnsigned = sha256(stream.GetBytes());
            }

            return _digestChangeCustomIDFeeCRCouncilMemberUnsigned;
		}

		// top serialize or deserialize
		void CRCProposal::Serialize(ByteStream &stream, uint8_t version) const {
			switch (_type) {
				case changeProposalOwner:
					SerializeChangeOwner(stream, version);
					break;

				case terminateProposal:
					SerializeTerminateProposal(stream, version);
					break;

				case secretaryGeneralElection:
					SerializeSecretaryElection(stream, version);
					break;

			    case reserveCustomID:
                    SerializeReserveCustomID(stream, version);
			        break;

			    case receiveCustomID:
                    SerializeReceiveCustomID(stream, version);
			        break;

			    case changeCustomIDFee:
                    SerializeChangeCustomIDFee(stream, version);
			        break;

			    case normal:
			    case elip:
					SerializeNormalOrELIP(stream, version);
					break;

				default:
					SPVLOG_ERROR("serialize cr proposal unknown type");
					break;
			}
		}

		bool CRCProposal::Deserialize(const ByteStream &stream, uint8_t version) {
			uint16_t type = 0;
			if (!stream.ReadUint16(type)) {
				SPVLOG_ERROR("deserialize type");
				return false;
			}
			_type = CRCProposal::Type(type);

			bool r = false;
			switch (_type) {
				case changeProposalOwner:
					r = DeserializeChangeOwner(stream, version);
					break;

				case terminateProposal:
					r = DeserializeTerminateProposal(stream, version);
					break;

				case secretaryGeneralElection:
					r = DeserializeSecretaryElection(stream, version);
					break;

                case reserveCustomID:
                    r = DeserializeReserveCustomID(stream, version);
                    break;

                case receiveCustomID:
                    r = DeserializeReceiveCustomID(stream, version);
                    break;

                case changeCustomIDFee:
                    r = DeserializeChangeCustomIDFee(stream, version);
                    break;

                case normal:
                case elip:
					r = DeserializeNormalOrELIP(stream, version);
					break;

				default:
					SPVLOG_ERROR("unknow type: {}", _type);
					r = false;
					break;
			}

			return r;
		}

		nlohmann::json CRCProposal::ToJsonNormalOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;
			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			if (version >= CRCProposalVersion01)
				j[JsonKeyDraftData] = Base64::Encode(_draftData);
			j[JsonKeyBudgets] = _budgets;
			j[JsonKeyRecipient] = _recipient.String();
			return j;
		}

		void CRCProposal::FromJsonNormalOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			if (version >= CRCProposalVersion01) {
				std::string draftData = j[JsonKeyDraftData].get<std::string>();
				_draftData = Base64::Decode(draftData);
				ErrorChecker::CheckParam(_draftData.size() > DRAFT_DATA_MAX_SIZE, Error::ProposalContentTooLarge, "proposal origin content too large");
				uint256 draftHash(sha256_2(_draftData));
				ErrorChecker::CheckParam(draftHash != _draftHash, Error::ProposalHashNotMatch, "proposal hash not match");
			}
			_budgets = j[JsonKeyBudgets].get<std::vector<Budget>>();
			_recipient = Address(j[JsonKeyRecipient].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonNormalCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonNormalOwnerUnsigned(version);
			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
			return j;
		}

		void CRCProposal::FromJsonNormalCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonNormalOwnerUnsigned(j, version);
			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJson(uint8_t version) const {
			nlohmann::json j;

			switch (_type) {
				case normal:
				case elip:
					j = ToJsonNormalCRCouncilMemberUnsigned(version);
					j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
					break;

				case secretaryGeneralElection:
					j = ToJsonSecretaryElectionCRCouncilMemberUnsigned(version);
					j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
					break;

				case changeProposalOwner:
					j = ToJsonChangeOwnerCRCouncilMemberUnsigned(version);
					j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
					break;

				case terminateProposal:
					j = ToJsonTerminateProposalCRCouncilMemberUnsigned(version);
					j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
					break;

			    case reserveCustomID:
			        j = ToJsonReserveCustomIDCRCouncilMemberUnsigned(version);
			        j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
			        break;

			    case receiveCustomID:
			        j = ToJsonReceiveCustomIDCRCouncilMemberUnsigned(version);
			        j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
			        break;

			    case changeCustomIDFee:
                    j = ToJsonChangeCustomIDFeeCRCouncilMemberUnsigned(version);
                    j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
			        break;

				default:
					SPVLOG_ERROR("unknow type: {}", _type);
					break;
			}
			return j;
		}

		void CRCProposal::FromJson(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());

			switch (_type) {
				case normal:
				case elip:
					FromJsonNormalCRCouncilMemberUnsigned(j, version);
					_crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
					break;

				case secretaryGeneralElection:
					FromJsonSecretaryElectionCRCouncilMemberUnsigned(j, version);
					_crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
					break;

				case changeProposalOwner:
					FromJsonChangeOwnerCRCouncilMemberUnsigned(j, version);
					_crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
					break;

				case terminateProposal:
					FromJsonTerminateProposalCRCouncilMemberUnsigned(j, version);
					_crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
					break;

			    case reserveCustomID:
			        FromJsonReserveCustomIDCRCouncilMemberUnsigned(j, version);
                    _crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
			        break;

			    case receiveCustomID:
			        FromJsonReceiveCustomIDCRCouncilMemberUnsigned(j, version);
                    _crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
			        break;

			    case changeCustomIDFee:
			        FromJsonChangeCustomIDFeeCRCouncilMemberUnsigned(j, version);
                    _crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
			        break;

				default:
					SPVLOG_ERROR("unknow type: {}", _type);
					break;
			}
		}

		bool CRCProposal::IsValidNormalOwnerUnsigned(uint8_t version) const {
			if (_type >= CRCProposal::maxType) {
				SPVLOG_ERROR("invalid proposal type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(CTElastos, _ownerPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid proposal owner pubkey");
				return false;
			}

			for (const Budget &budget : _budgets) {
				if (!budget.IsValid()) {
					SPVLOG_ERROR("invalid budget");
					return false;
				}
			}

			if (!_recipient.Valid()) {
				SPVLOG_ERROR("invalid recipient");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidNormalCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidNormalOwnerUnsigned(version))
				return false;

			try {
				if (!Key(CTElastos, _ownerPublicKey).Verify(DigestNormalOwnerUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify owner signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr committee did");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValid(uint8_t version) const {
			bool isValid = false;
			switch (_type) {
				case normal:
				case elip:
					isValid = IsValidNormalCRCouncilMemberUnsigned(version);
					break;

				case secretaryGeneralElection:
					isValid = IsValidSecretaryElectionCRCouncilMemberUnsigned(version);
					break;

				case changeProposalOwner:
					isValid = IsValidChangeOwnerCRCouncilMemberUnsigned(version);
					break;

				case terminateProposal:
					isValid = IsValidTerminateProposalCRCouncilMemberUnsigned(version);
					break;

			    case reserveCustomID:
			        isValid = IsValidReserveCustomIDCRCouncilMemberUnsigned(version);
			        break;

			    case receiveCustomID:
			        isValid = IsValidReceiveCustomIDCRCouncilMemberUnsigned(version);
			        break;

			    case changeCustomIDFee:
			        isValid = IsValidChangeCustomIDFeeCRCouncilMemberUnsigned(version);
			        break;

				default:
					break;
			}

			if (_crCouncilMemberSignature.empty()) {
				SPVLOG_ERROR("cr committee not signed");
				isValid = false;
			}

			return isValid;
		}

		CRCProposal &CRCProposal::operator=(const CRCProposal &payload) {
			_type = payload._type;
			_categoryData = payload._categoryData;
			_ownerPublicKey = payload._ownerPublicKey;
			_draftHash = payload._draftHash;
			_draftData = payload._draftData;
			_budgets = payload._budgets;
			_recipient = payload._recipient;
			_targetProposalHash = payload._targetProposalHash;
            _reservedCustomIDList = payload._reservedCustomIDList;
            _receivedCustomIDList = payload._receivedCustomIDList;
            _receiverDID = payload._receiverDID;
            _rateOfCustomIDFee = payload._rateOfCustomIDFee;
			_newRecipient = payload._newRecipient;
			_newOwnerPublicKey = payload._newOwnerPublicKey;
			_secretaryPublicKey = payload._secretaryPublicKey;
			_secretaryDID = payload._secretaryDID;
			_signature = payload._signature;
			_newOwnerSignature = payload._newOwnerSignature;
			_secretarySignature = payload._secretarySignature;

			_crCouncilMemberDID = payload._crCouncilMemberDID;
			_crCouncilMemberSignature = payload._crCouncilMemberSignature;
			return *this;
		}

		bool CRCProposal::Equal(const IPayload &payload, uint8_t version) const {
			bool equal = false;
			try {
				const CRCProposal &realPayload = dynamic_cast<const CRCProposal &>(payload);

				switch (_type) {
					case normal:
					case elip:
						equal = _type == realPayload._type &&
								_categoryData == realPayload._categoryData &&
								_ownerPublicKey == realPayload._ownerPublicKey &&
								_draftHash == realPayload._draftHash &&
								_budgets == realPayload._budgets &&
								_recipient == realPayload._recipient &&
								_signature == realPayload._signature &&
								_crCouncilMemberDID == realPayload._crCouncilMemberDID &&
								_crCouncilMemberSignature == realPayload._crCouncilMemberSignature;
						if (version >= CRCProposalVersion01)
							equal = equal && _draftData == realPayload._draftData;
						break;

					case secretaryGeneralElection:
						equal = _type == realPayload._type &&
								_categoryData == realPayload._categoryData &&
								_ownerPublicKey == realPayload._ownerPublicKey &&
								_draftHash == realPayload._draftHash &&
								_secretaryPublicKey == realPayload._secretaryPublicKey &&
								_secretaryDID == realPayload._secretaryDID &&
								_signature == realPayload._signature &&
								_secretarySignature == realPayload._secretarySignature &&
								_crCouncilMemberDID == realPayload._crCouncilMemberDID &&
								_crCouncilMemberSignature == realPayload._crCouncilMemberSignature;
						if (version >= CRCProposalVersion01)
							equal = equal && _draftData == realPayload._draftData;
						break;

					case changeProposalOwner:
						equal = _type == realPayload._type &&
								_categoryData == realPayload._categoryData &&
								_ownerPublicKey == realPayload._ownerPublicKey &&
								_draftHash == realPayload._draftHash &&
								_targetProposalHash == realPayload._targetProposalHash &&
								_newRecipient == realPayload._newRecipient &&
								_newOwnerPublicKey == realPayload._newOwnerPublicKey &&
								_signature == realPayload._signature &&
								_newOwnerSignature == realPayload._newOwnerSignature &&
								_crCouncilMemberDID == realPayload._crCouncilMemberDID &&
								_crCouncilMemberSignature == realPayload._crCouncilMemberSignature;
						if (version >= CRCProposalVersion01)
							equal = equal && _draftData == realPayload._draftData;
						break;

					case terminateProposal:
						equal = _type == realPayload._type &&
								_categoryData == realPayload._categoryData &&
								_ownerPublicKey == realPayload._ownerPublicKey &&
								_draftHash == realPayload._draftHash &&
								_targetProposalHash == realPayload._targetProposalHash &&
								_signature == realPayload._signature &&
								_crCouncilMemberDID == realPayload._crCouncilMemberDID &&
								_crCouncilMemberSignature == realPayload._crCouncilMemberSignature;
						if (version >= CRCProposalVersion01)
							equal = equal && _draftData == realPayload._draftData;
						break;

				    case reserveCustomID:
				        equal = _type == realPayload._type &&
				                _categoryData == realPayload._categoryData &&
                                _ownerPublicKey == realPayload._ownerPublicKey &&
                                _draftHash == realPayload._draftHash &&
                                _reservedCustomIDList == realPayload._reservedCustomIDList &&
                                _signature == realPayload._signature &&
                                _crCouncilMemberDID == realPayload._crCouncilMemberDID &&
                                _crCouncilMemberSignature == realPayload._crCouncilMemberSignature;

                        if (version >= CRCProposalVersion01)
                            equal = equal && _draftData == realPayload._draftData;
				        break;

				    case receiveCustomID:
                        equal = _type == realPayload._type &&
                                _categoryData == realPayload._categoryData &&
                                _ownerPublicKey == realPayload._ownerPublicKey &&
                                _draftHash == realPayload._draftHash &&
                                _receivedCustomIDList == realPayload._receivedCustomIDList &&
                                _receiverDID == realPayload._receiverDID &&
                                _signature == realPayload._signature &&
                                _crCouncilMemberDID == realPayload._crCouncilMemberDID &&
                                _crCouncilMemberSignature == realPayload._crCouncilMemberSignature;

                        if (version >= CRCProposalVersion01)
                            equal = equal && _draftData == realPayload._draftData;
				        break;

				    case changeCustomIDFee:
                        equal = _type == realPayload._type &&
                                _categoryData == realPayload._categoryData &&
                                _ownerPublicKey == realPayload._ownerPublicKey &&
                                _draftHash == realPayload._draftHash &&
                                _rateOfCustomIDFee == realPayload._rateOfCustomIDFee &&
                                _signature == realPayload._signature &&
                                _crCouncilMemberDID == realPayload._crCouncilMemberDID &&
                                _crCouncilMemberSignature == realPayload._crCouncilMemberSignature;

                        if (version >= CRCProposalVersion01)
                            equal = equal && _draftData == realPayload._draftData;
				        break;

					default:
						equal = false;
						break;
				}
			} catch (const std::bad_cast &e) {
				SPVLOG_ERROR("payload is not instance of CRCProposal");
				equal = false;
			}
			return equal;
		}

		IPayload &CRCProposal::operator=(const IPayload &payload) {
			try {
				const CRCProposal &crcProposal = dynamic_cast<const CRCProposal &>(payload);
				operator=(crcProposal);
			} catch (const std::bad_cast &e) {
				SPVLOG_ERROR("payload is not instance of CRCProposal");
			}
			return *this;
		}
	}
}