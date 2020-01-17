// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CRCProposal.h"

#include <Common/hash.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {

		Budget::Budget() {

		}

		Budget::Budget(BudgetType type, uint8_t stage, BigInt amount) : _type(type), _stage(stage), _amount(amount) {

		}

		Budget::~Budget() {

		}

		Budget::BudgetType Budget::GetType() const {
			return _type;
		}

		uint8_t Budget::GetStage() const {
			return _stage;
		}

		BigInt Budget::GetAmount() const {
			return _amount;
		}

		void Budget::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteUint8(_type);
			ostream.WriteUint8(_stage);
			ostream.WriteUint64(_amount.getUint64());
		}

		bool Budget::Deserialize(const ByteStream &istream, uint8_t version) {
			uint8_t type;
			if (!istream.ReadUint8(type)) {
				Log::error("Budget::Deserialize: read type key");
				return false;
			}
			_type = (BudgetType) type;

			if (!istream.ReadUint8(_stage)) {
				Log::error("Budget::Deserialize: read stage key");
				return false;
			}

			uint64_t amount;
			if (!istream.ReadUint64(amount)) {
				Log::error("Budget::Deserialize: read amount key");
				return false;
			}
			_amount.setUint64(amount);

			return true;
		}

		nlohmann::json Budget::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["Type"] = _type;
			j["Stage"] = _stage;
			j["Amount"] = _amount.getDec();
			return j;
		}

		void Budget::FromJson(const nlohmann::json &j, uint8_t version) {
			uint8_t type = j["Type"].get<uint8_t>();
			_type = (BudgetType) type;
			_stage = j["Stage"].get<uint8_t>();
			std::string amount = j["Amount"].get<std::string>();
			_amount.setDec(amount);
		}

		CRCProposal::CRCProposal() {

		}

		CRCProposal::~CRCProposal() {

		}

		void CRCProposal::SetTpye(CRCProposalType type) {
			_type = type;
		}

		CRCProposal::CRCProposalType CRCProposal::GetType() const {
			return _type;
		}

		void CRCProposal::SetCategoryData(const std::string &categoryData) {
			_categoryData = categoryData;
		}

		const std::string &CRCProposal::GetCategoryData() const {
			return _categoryData;
		}

		void CRCProposal::SetSponsorPublicKey(const bytes_t &publicKey) {
			_sponsorPublicKey = publicKey;
		}

		const bytes_t &CRCProposal::GetSponsorPublicKey() const {
			return _sponsorPublicKey;
		}

		void CRCProposal::SetCRSponsorDID(const uint168 &crSponsorDID) {
			_crSponsorDID = crSponsorDID;
		}

		const uint168 &CRCProposal::GetCRSponsorDID() const {
			return _crSponsorDID;
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

		void CRCProposal::SetRecipient(const uint168 &recipient) {
			_recipient = recipient;
		}

		const uint168 &CRCProposal::GetRecipient() const {
			return _recipient;
		}

		void CRCProposal::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		const bytes_t &CRCProposal::GetSignature() const {
			return _signature;
		}

		void CRCProposal::SetCROpinionHash(const uint256 &hash) {
			_crOpinionHash = hash;
		}

		const uint256 &CRCProposal::GetCROpinionHash() const {
			return _crOpinionHash;
		}

		void CRCProposal::SetCRSignature(const bytes_t &signature) {
			_crSignature = signature;
		}

		const bytes_t &CRCProposal::GetCRSignature() const {
			return _crSignature;
		}

		uint256 CRCProposal::Hash() const {
			ByteStream stream;
			Serialize(stream, 0);
			return uint256(sha256_2(stream.GetBytes()));
		}

		size_t CRCProposal::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			size += sizeof(uint16_t);

			size += stream.WriteVarUint(_categoryData.size());
			size += _categoryData.size();

			size += stream.WriteVarUint(_sponsorPublicKey.size());
			size += _sponsorPublicKey.size();

			size += _draftHash.size();

			size += stream.WriteVarUint(_budgets.size());

			ByteStream byteStream;
			for (size_t i = 0; i < _budgets.size(); ++i) {
				_budgets[i].Serialize(byteStream, version);
			}
			size += byteStream.GetBytes().size();

			size += _recipient.size();

			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			size += _crSponsorDID.size();

			size += _crOpinionHash.size();

			size += stream.WriteVarUint(_crSignature.size());
			size += _crSignature.size();

			return size;
		}

		void CRCProposal::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteUint16(_type);
			ostream.WriteVarString(_categoryData);
			ostream.WriteVarBytes(_sponsorPublicKey);
			ostream.WriteBytes(_draftHash);
			ostream.WriteVarUint(_budgets.size());
			for (size_t i = 0; i < _budgets.size(); ++i) {
				_budgets[i].Serialize(ostream, version);
			}
			ostream.WriteBytes(_recipient);
		}

		bool CRCProposal::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
			uint16_t type = 0;
			if (!istream.ReadUint16(type)) {
				Log::error("CRCProposal DeserializeUnsigned: read type key");
				return false;
			}
			_type = CRCProposalType(type);

			if (!istream.ReadVarString(_categoryData)) {
				Log::error("CRCProposal DeserializeUnsigned: read categoryData key");
				return false;
			}

			if (!istream.ReadVarBytes(_sponsorPublicKey)) {
				Log::error("CRCProposal DeserializeUnsigned: read sponsorPublicKey key");
				return false;
			}

			if (!istream.ReadBytes(_draftHash)) {
				Log::error("CRCProposal DeserializeUnsigned: read draftHash key");
				return false;
			}

			uint64_t count = 0;
			if (!istream.ReadVarUint(count)) {
				Log::error("CRCProposal DeserializeUnsigned: read _budgets size");
				return false;
			}
			_budgets.resize(count);
			for (size_t i = 0; i < count; ++i) {
				_budgets[i].Deserialize(istream, version);
			}

			if (!istream.ReadBytes(_recipient)) {
				Log::error("CRCProposal DeserializeUnsigned: read _recipient  key");
				return false;
			}

			return true;
		}

		void CRCProposal::SerializeSponsorSigned(ByteStream &ostream, uint8_t version) {
			SerializeUnsigned(ostream, version);

			ErrorChecker::CheckParam(_signature.size() <= 0, Error::Sign, "sponsor unsigned");

			ostream.WriteVarBytes(_signature);
		}

		bool CRCProposal::DeserializeSponsorSigned(const ByteStream &istream, uint8_t version) {
			if (!DeserializeUnsigned(istream, version)) {
				return false;
			}

			if (!istream.ReadVarBytes(_signature)) {
				Log::error("CRCProposal DeserializeUnsigned: read signature key");
				return false;
			}
			return true;
		}

		void CRCProposal::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);

			ostream.WriteVarBytes(_signature);

			ostream.WriteBytes(_crSponsorDID);

			ostream.WriteBytes(_crOpinionHash);

			ostream.WriteVarBytes(_crSignature);
		}

		bool CRCProposal::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!DeserializeSponsorSigned(istream, version)) {
				return false;
			}

			if (!istream.ReadBytes(_crSponsorDID)) {
				Log::error("CRCProposal DeserializeUnsigned: read sponsorDID key");
				return false;
			}

			if (!istream.ReadBytes(_crOpinionHash)) {
				Log::error("CRCProposal DeserializeUnsigned: read crOpinionHash key");
				return false;
			}

			if (!istream.ReadVarBytes(_crSignature)) {
				Log::error("CRCProposal DeserializeUnsigned: read crSignature key");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJson(uint8_t version) const {
			nlohmann::json j, budgets;
			j["Type"] = _type;
			j["SponsorPublicKey"] = _sponsorPublicKey.getHex();
			j["CategoryData"] = _categoryData;
			j["CRSponsorDID"] = _crSponsorDID.GetHex();
			j["DraftHash"] = _draftHash.GetHex();
			for (const Budget &budget : _budgets) {
				budgets.push_back(budget.ToJson(version));
			}
			j["Budgets"] = budgets;
			j["Recipient"] = _recipient.GetHex();
			j["CROpinionHash"] = _crOpinionHash.GetHex();
			j["Signature"] = _signature.getHex();
			j["CRSignature"] = _crSignature.getHex();
			return j;
		}

		void CRCProposal::FromJson(const nlohmann::json &j, uint8_t version) {
			uint8_t type = j["Type"].get<uint8_t>();
			_type = CRCProposalType(type);

			std::string publickey = j["SponsorPublicKey"].get<std::string>();
			_sponsorPublicKey.setHex(publickey);

			_categoryData = j["CategoryData"].get<std::string>();

			std::string draftHash = j["DraftHash"].get<std::string>();
			_draftHash.SetHex(draftHash);

			nlohmann::json budgets = j["Budgets"];
			for (nlohmann::json::iterator it = budgets.begin(); it != budgets.end(); ++it) {
				Budget budget;
				budget.FromJson(*it, version);
				_budgets.push_back(budget);
			}

			std::string recipient = j["Recipient"].get<std::string>();
			_recipient.SetHex(recipient);

			std::string signatue = j["Signature"].get<std::string>();
			_signature.setHex(signatue);

			if (j.find("CROpinionHash") != j.end()) {
				std::string crOpinionHash = j["CROpinionHash"].get<std::string>();
				_crOpinionHash.SetHex(crOpinionHash);
			}

			if (j.find("CRSponsorDID") != j.end()) {
				std::string did = j["CRSponsorDID"].get<std::string>();
				_crSponsorDID.SetHex(did);
			}

			if (j.find("CRSignature") != j.end()) {
				std::string crSignatre = j["CRSignature"].get<std::string>();
				_crSignature.setHex(crSignatre);
			}
		}

		CRCProposal &CRCProposal::operator=(const CRCProposal &payload) {
			_type = payload._type;
			_sponsorPublicKey = payload._sponsorPublicKey;
			_crSponsorDID = payload._crSponsorDID;
			_draftHash = payload._draftHash;
			_budgets = payload._budgets;
			_recipient = payload._recipient;
			_signature = payload._signature;
			_crSignature = payload._crSignature;
			return *this;
		}

		IPayload &CRCProposal::operator=(const IPayload &payload) {
			try {
				const CRCProposal &crcProposal = dynamic_cast<const CRCProposal &>(payload);
				operator=(crcProposal);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CRCProposal");
			}
			return *this;
		}
	}
}