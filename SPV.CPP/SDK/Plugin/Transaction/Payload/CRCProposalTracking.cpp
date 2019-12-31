// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CRCProposalTracking.h"

#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		CRCProposalTracking::CRCProposalTracking() {

		}

		CRCProposalTracking::~CRCProposalTracking() {

		}

		void CRCProposalTracking::SetType(CRCProposalTrackingType type) {
			_type = type;
		}

		CRCProposalTracking::CRCProposalTrackingType CRCProposalTracking::GetType() const {
			return _type;
		}

		void CRCProposalTracking::SetProposalHash(const uint256 &proposalHash) {
			_proposalHash = proposalHash;
		}

		const uint256 &CRCProposalTracking::GetProposalHash() const {
			return _proposalHash;
		}

		void CRCProposalTracking::SetDocumentHash(const uint256 &documentHash) {
			_documentHash = documentHash;
		}

		const uint256 &CRCProposalTracking::GetDocumentHash() const {
			return _documentHash;
		}

		void CRCProposalTracking::SetStage(uint8_t stage) {
			_stage = stage;
		}

		uint8_t CRCProposalTracking::GetStage() const {
			return _stage;
		}

		void CRCProposalTracking::SetAppropriation(uint64_t appropriation) {
			_appropriation = appropriation;
		}

		uint64_t CRCProposalTracking::GetAppropriation() const {
			return _appropriation;
		}

		void CRCProposalTracking::SetLeaderPubKey(const bytes_t &leaderPubKey) {
			_leaderPubKey = leaderPubKey;
		}

		const bytes_t &CRCProposalTracking::GetLeaderPubKey() const {
			return _leaderPubKey;
		}

		void CRCProposalTracking::SetNewLeaderPubKey(const bytes_t &newLeaderPubKey) {
			_newLeaderPubKey = newLeaderPubKey;
		}

		const bytes_t &CRCProposalTracking::GetNewLeaderPubKey() const {
			return _newLeaderPubKey;
		}

		void CRCProposalTracking::SetLeaderSign(const bytes_t &signature) {
			_leaderSign = signature;
		}

		const bytes_t &CRCProposalTracking::GetLeaderSign() const {
			return _leaderSign;
		}

		void CRCProposalTracking::SetNewLeaderSign(const bytes_t &signature) {
			_newLeaderSign = signature;
		}

		const bytes_t &CRCProposalTracking::GetNewLeaderSign() const {
			return _newLeaderSign;
		}

		void CRCProposalTracking::SetSecretaryGeneralSign(const bytes_t &signature) {
			_secretaryGeneralSign = signature;
		}

		const bytes_t &CRCProposalTracking::GetSecretaryGeneralSign() const {
			return _secretaryGeneralSign;
		}

		size_t CRCProposalTracking::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			size += sizeof(uint8_t);

			size += _proposalHash.size();
			size += _documentHash.size();

			size += sizeof(uint8_t);

			size += sizeof(uint64_t);

			size += stream.WriteVarUint(_leaderPubKey.size());
			size += _leaderPubKey.size();

			size += stream.WriteVarUint(_newLeaderPubKey.size());
			size += _newLeaderPubKey.size();

			size += stream.WriteVarUint(_leaderSign.size());
			size += _leaderSign.size();

			size += stream.WriteVarUint(_newLeaderSign.size());
			size += _newLeaderSign.size();

			size += stream.WriteVarUint(_secretaryGeneralSign.size());
			size += _secretaryGeneralSign.size();

			return size;
		}

		void CRCProposalTracking::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteUint8(_type);
			ostream.WriteBytes(_proposalHash);
			ostream.WriteBytes(_documentHash);
			ostream.WriteUint8(_stage);
			ostream.WriteUint64(_appropriation);
			ostream.WriteVarBytes(_leaderPubKey);
			ostream.WriteVarBytes(_newLeaderPubKey);
		}

		bool CRCProposalTracking::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
			uint8_t type = 0;
			if (!istream.ReadUint8(type)) {
				Log::error("CRCProposalTracking DeserializeUnsigned: read type key");
				return false;
			}
			_type = CRCProposalTrackingType(type);

			if (!istream.ReadBytes(_proposalHash)) {
				Log::error("CRCProposalTracking DeserializeUnsigned: read proposalHash key");
				return false;
			}

			if (!istream.ReadBytes(_documentHash)) {
				Log::error("CRCProposalTracking DeserializeUnsigned: read documentHash key");
				return false;
			}

			if (!istream.ReadUint8(_stage)) {
				Log::error("CRCProposalTracking DeserializeUnsigned: read stage key");
				return false;
			}

			if (!istream.ReadUint64(_appropriation)) {
				Log::error("CRCProposalTracking DeserializeUnsigned: read appropriation key");
				return false;
			}

			if (!istream.ReadVarBytes(_leaderPubKey)) {
				Log::error("CRCProposalTracking DeserializeUnsigned: read leaderPubKey key");
				return false;
			}

			if (!istream.ReadVarBytes(_newLeaderPubKey)) {
				Log::error("CRCProposalTracking DeserializeUnsigned: read leaderPubKey key");
				return false;
			}

			return true;
		}

		void CRCProposalTracking::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);

			ostream.WriteVarBytes(_leaderSign);
			ostream.WriteVarBytes(_newLeaderSign);
			ostream.WriteVarBytes(_secretaryGeneralSign);
		}

		bool CRCProposalTracking::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!DeserializeUnsigned(istream, version)) {
				return false;
			}

			if (!istream.ReadVarBytes(_leaderSign)) {
				Log::error("CRCProposalTracking Deserialize: read leaderSign key");
				return false;
			}

			if (!istream.ReadVarBytes(_newLeaderSign)) {
				Log::error("CRCProposalTracking Deserialize: read newLeaderSign key");
				return false;
			}

			if (!istream.ReadVarBytes(_secretaryGeneralSign)) {
				Log::error("CRCProposalTracking Deserialize: read secretaryGeneralSign key");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposalTracking::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["Type"] = _type;
			j["ProposalHash"] = _proposalHash.GetHex();
			j["DocumentHash"] = _documentHash.GetHex();
			j["Stage"] = _stage;
			j["Appropriation"] = _appropriation;
			j["LeaderPubKey"] = _leaderPubKey.getHex();
			j["NewLeaderPubKey"] = _newLeaderPubKey.getHex();
			j["LeaderSign"] = _leaderSign.getHex();
			j["NewLeaderSign"] = _newLeaderSign.getHex();
			j["SecretaryGeneralSign"] = _secretaryGeneralSign.getHex();

			return j;
		}

		void CRCProposalTracking::FromJson(const nlohmann::json &j, uint8_t version) {
			uint8_t type = j["Type"].get<uint8_t>();
			_type = CRCProposalTrackingType(type);

			std::string proposalHash = j["ProposalHash"].get<std::string>();
			_proposalHash.SetHex(proposalHash);

			std::string documentHash = j["DocumentHash"].get<std::string>();
			_documentHash.SetHex(documentHash);

			_stage = j["Stage"].get<uint8_t>();

			_appropriation = j["Appropriation"].get<uint64_t>();

			std::string leaderPubKey = j["LeaderPubKey"].get<std::string>();
			_leaderPubKey.setHex(leaderPubKey);

			std::string newLeaderPubkey = j["NewLeaderPubKey"].get<std::string>();
			_newLeaderPubKey.setHex(newLeaderPubkey);

			std::string leaderSign = j["LeaderSign"].get<std::string>();
			_leaderSign.setHex(leaderSign);

			if (j.find("NewLeaderSign") != j.end()) {
				std::string newLeaderSign = j["NewLeaderSign"].get<std::string>();
				_newLeaderSign.setHex(newLeaderSign);
			}

			if (j.find("SecretaryGeneralSign") != j.end()) {
				std::string secretaryGeneralSign = j["SecretaryGeneralSign"].get<std::string>();
				_secretaryGeneralSign.setHex(secretaryGeneralSign);
			}
		}

		IPayload &CRCProposalTracking::operator=(const IPayload &payload) {
			try {
				const CRCProposalTracking &tracking = dynamic_cast<const CRCProposalTracking &>(payload);
				operator=(tracking);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CRCProposal");
			}
			return *this;
		}

		CRCProposalTracking &CRCProposalTracking::operator=(const CRCProposalTracking &payload) {
			_type = payload._type;
			_proposalHash = payload._proposalHash;
			_documentHash = payload._documentHash;
			_stage = payload._stage;
			_appropriation = payload._appropriation;
			_leaderPubKey = payload._leaderPubKey;
			_newLeaderPubKey = payload._newLeaderPubKey;
			_leaderSign = payload._leaderSign;
			_newLeaderSign = payload._newLeaderSign;
			_secretaryGeneralSign = payload._secretaryGeneralSign;
			return *this;
		}
	}
}