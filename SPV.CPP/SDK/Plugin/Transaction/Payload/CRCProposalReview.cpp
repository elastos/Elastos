// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CRCProposalReview.h"
#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		CRCProposalReview::CRCProposalReview() {

		}

		CRCProposalReview::~CRCProposalReview() {

		}

		void CRCProposalReview::SetProposalHash(const uint256 &hash) {
			_proposalHash = hash;
		}

		const uint256 &CRCProposalReview::GetProposalHash() const {
			return _proposalHash;
		}

		void CRCProposalReview::SetResult(VoteResult result) {
			_result = result;
		}

		CRCProposalReview::VoteResult CRCProposalReview::GetResult() const {
			return _result;
		}

		void CRCProposalReview::SetCRDID(const uint168 &crDID) {
			_crDID = crDID;
		}

		const uint168 &CRCProposalReview::GetCRDID() const {
			return _crDID;
		}

		void CRCProposalReview::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		const bytes_t &CRCProposalReview::GetSignature() const {
			return _signature;
		}

		size_t CRCProposalReview::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			size += _proposalHash.size();
			size += sizeof(uint8_t);
			size += _crDID.size();
			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			return size;
		}

		void CRCProposalReview::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteBytes(_proposalHash);
			ostream.WriteUint8(_result);
			ostream.WriteBytes(_crDID);
		}

		bool CRCProposalReview::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadBytes(_proposalHash)) {
				Log::error("CRCProposalReview DeserializeUnsigned: read proposalHash key");
				return false;
			}

			uint8_t result = 0;
			if (!istream.ReadUint8(result)) {
				Log::error("CRCProposalReview DeserializeUnsigned: read result key");
				return false;
			}
			_result = VoteResult(result);

			if (!istream.ReadBytes(_crDID)) {
				Log::error("CRCProposalReview DeserializeUnsigned: read crDID key");
				return false;
			}

			return true;
		}

		void CRCProposalReview::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);
			ostream.WriteVarBytes(_signature);
		}

		bool CRCProposalReview::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!DeserializeUnsigned(istream, version)) {
				return false;
			}

			if (!istream.ReadVarBytes(_signature)) {
				Log::error("CRCProposalReview DeserializeUnsigned: read signature key");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposalReview::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["ProposalHash"] = _proposalHash.GetHex();
			j["Result"] = _result;
			j["CRDID"] = _crDID.GetHex();
			j["Signature"] = _signature.getHex();
			return j;
		}

		void CRCProposalReview::FromJson(const nlohmann::json &j, uint8_t version) {
			std::string hash = j["ProposalHash"].get<std::string>();
			_proposalHash.SetHex(hash);

			uint8_t result = j["Result"].get<uint8_t>();
			_result = (VoteResult) result;

			std::string did = j["CRDID"].get<std::string>();
			_crDID.SetHex(did);

			std::string signature = j["Signature"].get<std::string>();
			_signature.setHex(signature);
		}

		IPayload &CRCProposalReview::operator=(const IPayload &payload) {
			try {
				const CRCProposalReview &review = dynamic_cast<const CRCProposalReview &>(payload);
				operator=(review);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CRCProposalReview");
			}
			return *this;
		}

		CRCProposalReview &CRCProposalReview::operator=(const CRCProposalReview &payload) {
			_proposalHash = payload._proposalHash;
			_result = payload._result;
			_crDID = payload._crDID;
			_signature = payload._signature;
			return *this;
		}
	}
}