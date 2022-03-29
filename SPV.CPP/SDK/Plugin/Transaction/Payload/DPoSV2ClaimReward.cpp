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
#include <Common/hash.h>
#include <Common/BigInt.h>
#include "DPoSV2ClaimReward.h"

namespace Elastos {
    namespace ElaWallet {

        DPoSV2ClaimReward::DPoSV2ClaimReward() {

        }

        DPoSV2ClaimReward::~DPoSV2ClaimReward() {

        }

        void DPoSV2ClaimReward::SerializeUnsigned(ByteStream &stream, uint8_t version) const {
            stream.WriteUint64(_amount);
        }

        bool DPoSV2ClaimReward::DeserializeUnsigned(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadUint64(_amount)) {
                Log::error("DPoSV2ClaimReward deserialize unsigned amount");
                return false;
            }

            return true;
        }

        nlohmann::json DPoSV2ClaimReward::ToJsonUnsigned(uint8_t version) const {
            nlohmann::json j;
            BigInt bgAmount;
            bgAmount.setUint64(_amount);
            j["Amount"] = bgAmount.getDec();
            return j;
        }

        void DPoSV2ClaimReward::FromJsonUnsigned(const nlohmann::json &j, uint8_t version) {
            BigInt bgAmount;
            bgAmount.setDec(j["Amount"].get<std::string>());
            _amount = bgAmount.getUint64();
        }

        uint256 DPoSV2ClaimReward::DigestDPoSV2ClaimReward(uint8_t version) const {
            ByteStream stream;
            SerializeUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        size_t DPoSV2ClaimReward::EstimateSize(uint8_t version) const {
            size_t size = 0;
            ByteStream stream;

            size += sizeof(_amount);
            size += stream.WriteVarUint(_signature.size());
            size += _signature.size();

            return size;
        }

        void DPoSV2ClaimReward::Serialize(ByteStream &stream, uint8_t version) const {
            SerializeUnsigned(stream, version);

            stream.WriteVarBytes(_signature);
        }

        bool DPoSV2ClaimReward::Deserialize(const ByteStream &stream, uint8_t version) {
            if (!DeserializeUnsigned(stream, version)) {
                Log::error("deserialize unsigned");
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                Log::error("DPoSV2ClaimReward deserialize signature");
                return false;
            }

            return true;
        }

        nlohmann::json DPoSV2ClaimReward::ToJson(uint8_t version) const {
            nlohmann::json j = ToJsonUnsigned(version);
            j["Signature"] = _signature.getHex();
            return j;
        }

        void DPoSV2ClaimReward::FromJson(const nlohmann::json &j, uint8_t version) {
            FromJsonUnsigned(j, version);
            _signature.setHex(j["Signature"].get<std::string>());
        }

        bool DPoSV2ClaimReward::IsValid(uint8_t version) const {
            return true;
        }

        IPayload &DPoSV2ClaimReward::operator=(const IPayload &payload) {
            try {
                const DPoSV2ClaimReward &p= dynamic_cast<const DPoSV2ClaimReward &>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of DPoSV2ClaimReward");
            }

            return *this;
        }

        DPoSV2ClaimReward &DPoSV2ClaimReward::operator=(const DPoSV2ClaimReward &payload) {
            _amount = payload._amount;
            _signature = payload._signature;
            return *this;
        }

        bool DPoSV2ClaimReward::Equal(const IPayload &payload, uint8_t version) const {
            try {
                const DPoSV2ClaimReward &p= dynamic_cast<const DPoSV2ClaimReward &>(payload);
                return _amount == p._amount && _signature == p._signature;
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of DPoSV2ClaimReward");
            }

            return false;
        }

    }
}