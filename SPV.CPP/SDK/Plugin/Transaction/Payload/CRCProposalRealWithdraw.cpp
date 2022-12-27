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

#include "CRCProposalRealWithdraw.h"
#include "Common/Log.h"

namespace Elastos {
	namespace ElaWallet {

		CRCProposalRealWithdraw::~CRCProposalRealWithdraw() {

		}

		CRCProposalRealWithdraw::CRCProposalRealWithdraw() {

		}

		size_t CRCProposalRealWithdraw::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			size += sizeof(uint16_t);

			size += stream.WriteVarUint(_withdrawTxHashes.size());
			// 32 == sizeof(uint256)
			size += _withdrawTxHashes.size() * 32;

			return size;
		}

		void CRCProposalRealWithdraw::Serialize(ByteStream &stream, uint8_t version) const {
			stream.WriteVarUint(_withdrawTxHashes.size());
			for (size_t i = 0; i < _withdrawTxHashes.size(); ++i)
				stream.WriteBytes(_withdrawTxHashes[i]);
		}

		bool CRCProposalRealWithdraw::Deserialize(const ByteStream &stream, uint8_t version) {
			uint64_t size = 0;
			if (!stream.ReadVarUint(size)) {
				SPVLOG_ERROR("deserialize proposal real withdraw size");
				return false;
			}

			uint256 hash;
			for (size_t i = 0; i < size; ++i) {
				if (!stream.ReadBytes(hash)) {
					SPVLOG_ERROR("deserialize proposal real withdraw hash");
					return false;
				}
				_withdrawTxHashes.push_back(hash);
			}

			return true;
		}

		nlohmann::json CRCProposalRealWithdraw::ToJson(uint8_t version) const {
			nlohmann::json jarray = nlohmann::json::array();
			nlohmann::json j;

			for (const uint256 &u : _withdrawTxHashes)
				jarray.push_back(u.GetHex());

			j["WithdrawTxHashes"] = jarray;
			return j;
		}

		void CRCProposalRealWithdraw::FromJson(const nlohmann::json &j, uint8_t version) {
			nlohmann::json jarray = j["WithdrawTxHashes"];
			if (!jarray.is_array()) {
				SPVLOG_DEBUG("json is not array");
				return;
			}

			_withdrawTxHashes.clear();
			uint256 u;
			for (nlohmann::json::iterator it = jarray.begin(); it != jarray.end(); ++it) {
				u.SetHex((*it).get<std::string>());
				_withdrawTxHashes.push_back(u);
			}
		}

		bool CRCProposalRealWithdraw::IsValid(uint8_t version) const {
			return !_withdrawTxHashes.empty();
		}

		IPayload &CRCProposalRealWithdraw::operator=(const IPayload &payload) {
			try {
				const CRCProposalRealWithdraw &crcProposal = dynamic_cast<const CRCProposalRealWithdraw &>(payload);
				operator=(crcProposal);
			} catch (const std::bad_cast &e) {
				SPVLOG_ERROR("payload is not instance of CRCProposalRealWithdraw");
			}
			return *this;
		}

		CRCProposalRealWithdraw &CRCProposalRealWithdraw::operator=(const CRCProposalRealWithdraw &payload) {
			_withdrawTxHashes = payload._withdrawTxHashes;
			return *this;
		}

		bool CRCProposalRealWithdraw::Equal(const IPayload &payload, uint8_t version) const {
			try {
				const CRCProposalRealWithdraw &p = dynamic_cast<const CRCProposalRealWithdraw &>(payload);
				return _withdrawTxHashes == p._withdrawTxHashes;
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CRCProposalRealWithdraw");
			}

			return false;
		}

	}
}