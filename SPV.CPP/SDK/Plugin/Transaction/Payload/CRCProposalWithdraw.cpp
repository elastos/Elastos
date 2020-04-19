/*
 * Copyright (c) 2020 Elastos Foundation
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

#include "CRCProposalWithdraw.h"

#include <Common/hash.h>
#include <Common/Log.h>
#include <WalletCore/Key.h>

namespace Elastos {
	namespace ElaWallet {

#define JsonKeyProposalHash "ProposalHash"
#define JsonKeyOwnerPubkey "OwnerPublicKey"
#define JsonKeySignature "Signature"

		CRCProposalWithdraw::CRCProposalWithdraw() {

		}

		CRCProposalWithdraw::~CRCProposalWithdraw() {

		}

		void CRCProposalWithdraw::SetProposalHash(const uint256 &hash) {
			_proposalHash = hash;
		}

		const uint256 &CRCProposalWithdraw::GetProposalHash() const {
			return _proposalHash;
		}

		void CRCProposalWithdraw::SetOwnerPublicKey(const bytes_t &pubkey) {
			_ownerPubkey = pubkey;
		}

		const bytes_t &CRCProposalWithdraw::GetOwnerPublicKey() const {
			return _ownerPubkey;
		}

		void CRCProposalWithdraw::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		const bytes_t &CRCProposalWithdraw::GetSignature() const {
			return _signature;
		}

		const uint256 &CRCProposalWithdraw::DigestUnsigned(uint8_t version) const {
			if (_digest == 0) {
				ByteStream stream;
				SerializeUnsigned(stream, version);
				_digest = sha256(stream.GetBytes());
			}

			return _digest;
		}

		size_t CRCProposalWithdraw::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			size += _proposalHash.size();
			size += stream.WriteVarUint(_ownerPubkey.size());
			size += _ownerPubkey.size();
			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			return size;
		}

		void CRCProposalWithdraw::SerializeUnsigned(ByteStream &stream, uint8_t version) const {
			stream.WriteBytes(_proposalHash);
			stream.WriteVarBytes(_ownerPubkey);
		}

		void CRCProposalWithdraw::Serialize(ByteStream &stream, uint8_t version) const {
			SerializeUnsigned(stream, version);
			stream.WriteVarBytes(_signature);
		}

		bool CRCProposalWithdraw::DeserializeUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadBytes(_proposalHash)) {
				SPVLOG_ERROR("deserialize proposal hash");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPubkey)) {
				SPVLOG_ERROR("deserialize owner pubkey");
				return false;
			}

			return true;
		}

		bool CRCProposalWithdraw::Deserialize(const ByteStream &stream, uint8_t version) {
			if (!DeserializeUnsigned(stream, version))
				return false;

			if (!stream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize sign");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposalWithdraw::ToJsonUnsigned(uint8_t version) const {
			nlohmann::json j;
			j[JsonKeyProposalHash] = _proposalHash.GetHex();
			j[JsonKeyOwnerPubkey] = _ownerPubkey.getHex();
			return j;
		}

		nlohmann::json CRCProposalWithdraw::ToJson(uint8_t version) const {
			nlohmann::json j = ToJsonUnsigned(version);

			j[JsonKeySignature] = _signature.getHex();
			return j;
		}

		void CRCProposalWithdraw::FromJsonUnsigned(const nlohmann::json &j, uint8_t version) {
			_proposalHash.SetHex(j[JsonKeyProposalHash].get<std::string>());
			_ownerPubkey.setHex(j[JsonKeyOwnerPubkey].get<std::string>());
		}

		void CRCProposalWithdraw::FromJson(const nlohmann::json &j, uint8_t version) {
			FromJsonUnsigned(j, version);
			_signature.setHex(j[JsonKeySignature].get<std::string>());
		}

		bool CRCProposalWithdraw::IsValidUnsigned(uint8_t versin) const {
			try {
				Key key(_ownerPubkey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid owner pubkey");
				return false;
			}

			return true;
		}

		bool CRCProposalWithdraw::IsValid(uint8_t version) const {
			if (!IsValidUnsigned(version))
				return false;

			try {
				if (!Key(_ownerPubkey).Verify(DigestUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature excpetion: {}", e.what());
				return false;
			}

			return true;
		}

		IPayload &CRCProposalWithdraw::operator=(const IPayload &payload) {
			try {
				const CRCProposalWithdraw &tracking = dynamic_cast<const CRCProposalWithdraw &>(payload);
				operator=(tracking);
			} catch (const std::bad_cast &e) {
				SPVLOG_ERROR("payload is not instance of CRCProposalWithdraw");
			}
			return *this;
		}

		CRCProposalWithdraw &CRCProposalWithdraw::operator=(const CRCProposalWithdraw &payload) {
			_proposalHash = payload._proposalHash;
			_ownerPubkey = payload._ownerPubkey;
			_signature = payload._signature;
			return *this;
		}

	}
}