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

#include <Common/Log.h>
#include <WalletCore/Key.h>
#include "CRCouncilMemberClaimNode.h"

namespace Elastos {
	namespace ElaWallet {

#define JsonKeyNodePublicKey "NodePublicKey"
#define JsonKeyCRCouncilMemberDID "CRCouncilMemberDID"
#define JsonKeyCRCouncilMemberSignature "CRCouncilMemberSignature"

		CRCouncilMemberClaimNode::CRCouncilMemberClaimNode() {

		}

		CRCouncilMemberClaimNode::~CRCouncilMemberClaimNode() {

		}

		size_t CRCouncilMemberClaimNode::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			size += stream.WriteVarUint(_nodePublicKey.size());
			size += _nodePublicKey.size();
			size += _crCouncilMemberDID.ProgramHash().size();
			size += stream.WriteVarUint(_crCouncilMemberSignature.size());
			size += _crCouncilMemberSignature.size();

			return size;
		}

		void CRCouncilMemberClaimNode::Serialize(ByteStream &stream, uint8_t version) const {
			SerializeUnsigned(stream, version);
			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCouncilMemberClaimNode::Deserialize(const ByteStream &stream, uint8_t version) {
			if (!DeserializeUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize unsigned fail");
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("deserialize signature fail");
				return false;
			}

			return true;
		}

		nlohmann::json CRCouncilMemberClaimNode::ToJson(uint8_t version) const {
			nlohmann::json j = ToJsonUnsigned(version);
			j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
			return j;
		}

		void CRCouncilMemberClaimNode::FromJson(const nlohmann::json &j, uint8_t version) {
			FromJsonUnsigned(j, version);
			_crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
		}

		bool CRCouncilMemberClaimNode::IsValid(uint8_t version) const {
			if (!IsValidUnsigned(version)) {
				SPVLOG_ERROR("unsigned is not valid");
				return false;
			}

			if (_crCouncilMemberSignature.empty()) {
				SPVLOG_ERROR("invalid signature");
				return false;
			}

			return true;
		}

		IPayload &CRCouncilMemberClaimNode::operator=(const IPayload &payload) {
			try {
				const CRCouncilMemberClaimNode &realPayload= dynamic_cast<const CRCouncilMemberClaimNode &>(payload);
				operator=(realPayload);
			} catch (const std::bad_cast &e) {
				SPVLOG_ERROR("payload is not instance of CRCouncilMemberClaimNode");
			}
			return *this;
		}

		CRCouncilMemberClaimNode &CRCouncilMemberClaimNode::operator=(const CRCouncilMemberClaimNode &payload) {
			_digestUnsigned = payload._digestUnsigned;
			_nodePublicKey = payload._nodePublicKey;
			_crCouncilMemberDID = payload._crCouncilMemberDID;
			_crCouncilMemberSignature = payload._crCouncilMemberSignature;
			return *this;
		}

		bool CRCouncilMemberClaimNode::Equal(const IPayload &payload, uint8_t version) const {
			bool equal = false;

			try {
				const CRCouncilMemberClaimNode &realPayload= dynamic_cast<const CRCouncilMemberClaimNode &>(payload);
				equal = _nodePublicKey == realPayload._nodePublicKey &&
						_crCouncilMemberDID == realPayload._crCouncilMemberDID &&
						_crCouncilMemberSignature == realPayload._crCouncilMemberSignature;
			} catch (const std::bad_cast &e) {
				SPVLOG_ERROR("payload is not instance of CRCouncilMemberClaimNode");
				equal = false;
			}

			return equal;
		}

		void CRCouncilMemberClaimNode::SerializeUnsigned(ByteStream &stream, uint8_t version) const {
			stream.WriteVarBytes(_nodePublicKey);
			stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCouncilMemberClaimNode::DeserializeUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarBytes(_nodePublicKey)) {
				SPVLOG_ERROR("deserialize node pubkey");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize cr council member did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		nlohmann::json CRCouncilMemberClaimNode::ToJsonUnsigned(uint8_t version) const {
			nlohmann::json j;

			j[JsonKeyNodePublicKey] = _nodePublicKey.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();

			return j;
		}

		void CRCouncilMemberClaimNode::FromJsonUnsigned(const nlohmann::json &j, uint8_t version) {
			_nodePublicKey = j[JsonKeyNodePublicKey].get<std::string>();
			std::string did = j[JsonKeyCRCouncilMemberDID].get<std::string>();
			_crCouncilMemberDID = Address(did);
		}

		bool CRCouncilMemberClaimNode::IsValidUnsigned(uint8_t version) const {
			try {
				Key key(CTElastos, _nodePublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid node pubkey");
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr council member did");
				return false;
			}

			return true;
		}

		const uint256 &CRCouncilMemberClaimNode::DigestUnsigned(uint8_t version) const {
			if (_digestUnsigned == 0) {
				ByteStream stream;
				SerializeUnsigned(stream, version);
				_digestUnsigned = sha256(stream.GetBytes());
			}

			return _digestUnsigned;
		}

		void CRCouncilMemberClaimNode::SetNodePublicKey(const bytes_t &pubkey) {
			_nodePublicKey = pubkey;
		}

		void CRCouncilMemberClaimNode::SetCRCouncilMemberDID(const Address &did) {
			_crCouncilMemberDID = did;
		}

		void CRCouncilMemberClaimNode::SetCRCouncilMemberSignature(const bytes_t &signature) {
			_crCouncilMemberSignature = signature;
		}

	}
}