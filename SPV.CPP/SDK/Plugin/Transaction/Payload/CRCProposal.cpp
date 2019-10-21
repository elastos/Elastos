// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CRCProposal.h"

#include <SDK/Common/hash.h>
#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
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

		void CRCProposal::SetBudgets(const std::vector<uint64_t> &budgets) {
			_budgets = budgets;
		}

		const std::vector<uint64_t> &CRCProposal::GetBudgets() const {
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

			size += sizeof(uint8_t);

			size += stream.WriteVarUint(_sponsorPublicKey.size());
			size += _sponsorPublicKey.size();

			size += _crSponsorDID.size();

			size += _draftHash.size();

			size += stream.WriteVarUint(_budgets.size());

			size += sizeof(uint64_t) * _budgets.size();

			size += _recipient.size();

			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			size += stream.WriteVarUint(_crSignature.size());
			size += _crSignature.size();

			return size;
		}

		void CRCProposal::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteUint8(_type);
			ostream.WriteVarBytes(_sponsorPublicKey);
			ostream.WriteBytes(_crSponsorDID);
			ostream.WriteBytes(_draftHash);
			ostream.WriteVarUint(_budgets.size());
			for (size_t i = 0; i < _budgets.size(); ++i) {
				ostream.WriteUint64(_budgets[i]);
			}
			ostream.WriteBytes(_recipient);
		}

		bool CRCProposal::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
			uint8_t type = 0;
			if (!istream.ReadUint8(type)) {
				Log::error("CRCProposal DeserializeUnsigned: read type key");
				return false;
			}
			_type = CRCProposalType(type);

			if (!istream.ReadVarBytes(_sponsorPublicKey)) {
				Log::error("CRCProposal DeserializeUnsigned: read sponsorPublicKey key");
				return false;
			}

			if (!istream.ReadBytes(_crSponsorDID)) {
				Log::error("CRCProposal DeserializeUnsigned: read sponsorDID key");
				return false;
			}

			if (!istream.ReadBytes(_draftHash)) {
				Log::error("CRCProposal DeserializeUnsigned: read draftHash key");
				return false;
			}

			_budgets.clear();
			uint64_t count = 0;
			if (!istream.ReadVarUint(count)) {
				Log::error("CRCProposal DeserializeUnsigned: read _budgets size");
				return false;
			}
			for (size_t i = 0; i < count; ++i) {
				uint64_t budgets = 0;
				if (!istream.ReadUint64(budgets)) {
					Log::error("CRCProposal DeserializeUnsigned: read _budgets");
					return false;
				}
				_budgets.push_back(budgets);
			}

			if (!istream.ReadBytes(_recipient)) {
				Log::error("CRCProposal DeserializeUnsigned: read _recipient  key");
				return false;
			}

			return true;
		}

		void CRCProposal::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);

			ostream.WriteVarBytes(_signature);

			ostream.WriteVarBytes(_crSignature);
		}

		bool CRCProposal::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!DeserializeUnsigned(istream, version)) {
				return false;
			}

			if (!istream.ReadVarBytes(_signature)) {
				Log::error("CRCProposal DeserializeUnsigned: read _signature key");
				return false;
			}

			if (!istream.ReadVarBytes(_crSignature)) {
				Log::error("CRCProposal DeserializeUnsigned: read _crSignature key");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["Type"] = _type;
			j["SponsorPublicKey"] = _sponsorPublicKey.getHex();
			j["CRSponsorDID"] = _crSponsorDID.GetHex();
			j["DraftHash"] = _draftHash.GetHex();
			j["Budgets"] = _budgets;
			j["Recipient"] = _recipient.GetHex();
			j["Sgnature"] = _signature.getHex();
			j["CRSignature"] = _crSignature.getHex();
			return j;
		}

		void CRCProposal::FromJson(const nlohmann::json &j, uint8_t version) {
			uint8_t type = j["Type"].get<uint8_t>();
			_type = CRCProposalType(type);

			std::string publickey = j["SponsorPublicKey"].get<std::string>();
			_sponsorPublicKey.setHex(publickey);

			std::string did = j["CRSponsorDID"].get<std::string>();
			_crSponsorDID.SetHex(did);

			std::string draftHash = j["DraftHash"].get<std::string>();
			_draftHash.SetHex(draftHash);

			_budgets = j["Budgets"].get<std::vector<uint64_t>>();

			std::string recipient = j["Recipient"].get<std::string>();
			_recipient.SetHex(recipient);

			std::string signatue = j["Sgnature"].get<std::string>();
			_signature.setHex(signatue);

			std::string crSignatre = j["CRSignature"].get<std::string>();
			_crSignature.setHex(crSignatre);
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