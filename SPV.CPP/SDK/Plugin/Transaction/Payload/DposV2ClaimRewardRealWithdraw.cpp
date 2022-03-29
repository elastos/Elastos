/*
 * Copyright (c) 2022 Elastos Foundation LTD.
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
#include "DposV2ClaimRewardRealWithdraw.h"

namespace Elastos {
    namespace ElaWallet {

        DposV2ClaimRewardRealWithdraw::DposV2ClaimRewardRealWithdraw() {

        }

        DposV2ClaimRewardRealWithdraw::~DposV2ClaimRewardRealWithdraw() {

        }

        size_t DposV2ClaimRewardRealWithdraw::EstimateSize(uint8_t version) const {
            ByteStream stream;
            size_t size = 0;

            size += stream.WriteVarUint(_withdrawTxHashes.size());
            for (size_t i = 0; i < _withdrawTxHashes.size(); ++i)
                size += _withdrawTxHashes[i].size();

            return size;
        }

        void DposV2ClaimRewardRealWithdraw::Serialize(ByteStream &stream, uint8_t version) const {
            stream.WriteVarUint(_withdrawTxHashes.size());
            for (const uint256 &hash : _withdrawTxHashes) {
                stream.WriteBytes(hash);
            }
        }

        bool DposV2ClaimRewardRealWithdraw::Deserialize(const ByteStream &stream, uint8_t version) {
            uint64_t size;
            if (!stream.ReadVarUint(size)) {
                Log::error("DposV2ClaimRewardRealWithdraw deserialize size");
                return false;
            }

            _withdrawTxHashes.resize(size);
            for (size_t i = 0; i < size; ++i) {
                if (!stream.ReadBytes(_withdrawTxHashes[i])) {
                    Log::error("DposV2ClaimRewardRealWithdraw deserialize hash[{}]", i);
                    return false;
                }
            }

            return true;
        }

        nlohmann::json DposV2ClaimRewardRealWithdraw::ToJson(uint8_t version) const {
            nlohmann::json j, hashes;

            for (const uint256 &hash : _withdrawTxHashes)
                hashes.push_back(hash.GetHex());

            j["WithdrawTxHashes"] = hashes;

            return j;
        }

        void DposV2ClaimRewardRealWithdraw::FromJson(const nlohmann::json &j, uint8_t version) {
            nlohmann::json hashes = j["WithdrawTxHashes"];

            _withdrawTxHashes.clear();
            for (nlohmann::json::iterator it = hashes.begin(); it != hashes.end(); ++it)
                _withdrawTxHashes.emplace_back((*it).get<std::string>());
        }

        bool DposV2ClaimRewardRealWithdraw::IsValid(uint8_t version) const {
            return true;
        }

        IPayload &DposV2ClaimRewardRealWithdraw::operator=(const IPayload &payload) {
            try {
                const DposV2ClaimRewardRealWithdraw &p= dynamic_cast<const DposV2ClaimRewardRealWithdraw &>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of DposV2ClaimRewardRealWithdraw");
            }

            return *this;
        }

        DposV2ClaimRewardRealWithdraw &DposV2ClaimRewardRealWithdraw::operator=(const DposV2ClaimRewardRealWithdraw &payload) {
            _withdrawTxHashes = payload._withdrawTxHashes;
            return *this;
        }

        bool DposV2ClaimRewardRealWithdraw::Equal(const IPayload &payload, uint8_t version) const {
            try {
                const DposV2ClaimRewardRealWithdraw &p= dynamic_cast<const DposV2ClaimRewardRealWithdraw &>(payload);
                return _withdrawTxHashes == p._withdrawTxHashes;
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of DposV2ClaimRewardRealWithdraw");
            }
            return false;
        }

    }
}