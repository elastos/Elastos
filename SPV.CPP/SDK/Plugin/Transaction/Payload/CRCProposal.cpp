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

namespace Elastos {
	namespace ElaWallet {

#define JsonKeyType "Type"
#define JsonKeyStage "Stage"
#define JsonKeyAmount "Amount"

#define JsonKeyType "Type"
#define JsonKeyCategoryData "CategoryData"
#define JsonKeyOwnerPublicKey "OwnerPublicKey"
#define JsonKeyDraftHash "DraftHash"
#define JsonKeyBudgets "Budgets"
#define JsonKeyRecipient "Recipient"
#define JsonKeySignature "Signature"
#define JsonKeyCRCouncilMemberDID "CRCouncilMemberDID"
#define JsonKeyCRCouncilMemberSignature "CRCouncilMemberSignature"

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

		void CRCProposal::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		const bytes_t &CRCProposal::GetSignature() const {
			return _signature;
		}

		void CRCProposal::SetCRCouncilMemberSignature(const bytes_t &signature) {
			_crCouncilMemberSignature = signature;
		}

		const bytes_t &CRCProposal::GetCRCouncilMemberSignature() const {
			return _crCouncilMemberSignature;
		}

		const uint256 &CRCProposal::DigestOwnerUnsigned(uint8_t version) const {
			if (_digestOwnerUnsigned == 0) {
				ByteStream stream;
				SerializeOwnerUnsigned(stream, version);
				_digestOwnerUnsigned = sha256(stream.GetBytes());
			}

			return _digestOwnerUnsigned;
		}

		const uint256 &CRCProposal::DigestCRCouncilMemberUnsigned(uint8_t version) const {
			if (_digestCRCouncilMemberUnsigned == 0) {
				ByteStream stream;
				SerializeCRCouncilMemberUnsigned(stream, version);
				_digestCRCouncilMemberUnsigned = sha256(stream.GetBytes());
			}

			return _digestCRCouncilMemberUnsigned;
		}

		size_t CRCProposal::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			size += sizeof(uint16_t);

			size += stream.WriteVarUint(_categoryData.size());
			size += _categoryData.size();

			size += stream.WriteVarUint(_ownerPublicKey.size());
			size += _ownerPublicKey.size();

			size += _draftHash.size();

			size += stream.WriteVarUint(_budgets.size());

			ByteStream byteStream;
			for (size_t i = 0; i < _budgets.size(); ++i) {
				_budgets[i].Serialize(byteStream);
			}
			size += byteStream.GetBytes().size();

			size += _recipient.ProgramHash().size();

			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			size += _crCouncilMemberDID.ProgramHash().size();

			size += stream.WriteVarUint(_crCouncilMemberSignature.size());
			size += _crCouncilMemberSignature.size();

			return size;
		}

		void CRCProposal::SerializeOwnerUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteUint16(_type);
			ostream.WriteVarString(_categoryData);
			ostream.WriteVarBytes(_ownerPublicKey);
			ostream.WriteBytes(_draftHash);
			ostream.WriteVarUint(_budgets.size());
			for (size_t i = 0; i < _budgets.size(); ++i) {
				_budgets[i].Serialize(ostream);
			}
			ostream.WriteBytes(_recipient.ProgramHash());
		}

		bool CRCProposal::DeserializeOwnerUnsigned(const ByteStream &istream, uint8_t version) {
			uint16_t type = 0;
			if (!istream.ReadUint16(type)) {
				SPVLOG_ERROR("deserialize type");
				return false;
			}
			_type = CRCProposal::Type(type);

			if (!istream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize categoryData");
				return false;
			}

			if (!istream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize owner PublicKey");
				return false;
			}

			if (!istream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize draftHash");
				return false;
			}

			uint64_t count = 0;
			if (!istream.ReadVarUint(count)) {
				SPVLOG_ERROR("deserialize budgets size");
				return false;
			}
			_budgets.resize(count);
			for (size_t i = 0; i < count; ++i) {
				if (!_budgets[i].Deserialize(istream)) {
					SPVLOG_ERROR("deserialize bugets");
					return false;
				}
			}

			uint168 programHash;
			if (!istream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize recipient key");
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

		void CRCProposal::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeCRCouncilMemberUnsigned(ostream, version);

			ostream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!DeserializeCRCouncilMemberUnsigned(istream, version)) {
				SPVLOG_ERROR("CRCProposal deserialize crc unsigned");
				return false;
			}

			if (!istream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("CRCProposal deserialize crc signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJsonOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;
			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			j[JsonKeyBudgets] = _budgets;
			j[JsonKeyRecipient] = _recipient.String();
			return j;
		}

		void CRCProposal::FromJsonOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint8_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			_budgets = j[JsonKeyBudgets].get<std::vector<Budget>>();
			_recipient = Address(j[JsonKeyRecipient].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonOwnerUnsigned(version);
			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
			return j;
		}

		void CRCProposal::FromJsonCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonOwnerUnsigned(j, version);
			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJson(uint8_t version) const {
			nlohmann::json j = ToJsonCRCouncilMemberUnsigned(version);
			j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
			return j;
		}

		void CRCProposal::FromJson(const nlohmann::json &j, uint8_t version) {
			FromJsonCRCouncilMemberUnsigned(j, version);
			_crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
		}

		bool CRCProposal::IsValidOwnerUnsigned(uint8_t version) const {
			if (_type >= CRCProposal::maxType) {
				SPVLOG_ERROR("invalid proposal type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(_ownerPublicKey);
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

		bool CRCProposal::IsValidCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidOwnerUnsigned(version))
				return false;

			try {
				if (!Key(_ownerPublicKey).Verify(DigestOwnerUnsigned(version), _signature)) {
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
			if (!IsValidCRCouncilMemberUnsigned(version))
				return false;

			if (_crCouncilMemberSignature.empty()) {
				SPVLOG_ERROR("cr committee not signed");
				return false;
			}

			return true;
		}

		CRCProposal &CRCProposal::operator=(const CRCProposal &payload) {
			_type = payload._type;
			_categoryData = payload._categoryData;
			_ownerPublicKey = payload._ownerPublicKey;
			_draftHash = payload._draftHash;
			_budgets = payload._budgets;
			_recipient = payload._recipient;
			_signature = payload._signature;

			_crCouncilMemberDID = payload._crCouncilMemberDID;
			_crCouncilMemberSignature = payload._crCouncilMemberSignature;
			return *this;
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