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
#include <Common/JsonSerializer.h>
#include <WalletCore/Base58.h>
#include <WalletCore/Address.h>
#include <Common/BigInt.h>
#include "UnstakeRealWithdraw.h"


namespace Elastos {
    namespace ElaWallet {

        UnstakeRealWithdraw::UnstakeRealWithdraw() {

        }

        UnstakeRealWithdraw::~UnstakeRealWithdraw() {

        }

        size_t UnstakeRealWithdraw::EstimateSize(uint8_t version) const {
            size_t size = 0;
            size += _retVotesTxHash.size();
            size += _stakeAddress.size();
            size += sizeof(_value);
            return size;
        }

        void UnstakeRealWithdraw::Serialize(ByteStream &stream, uint8_t version) const {
            stream.WriteBytes(_retVotesTxHash);
            stream.WriteBytes(_stakeAddress);
            stream.WriteUint64(_value);
        }

        bool UnstakeRealWithdraw::Deserialize(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadBytes(_retVotesTxHash)) {
                Log::error("UnstakeRealWithdraw deserialize retVotesTXHash");
                return false;
            }

            if (!stream.ReadBytes(_stakeAddress)) {
                Log::error("UnstakeRealWithdraw deserialize stakeAddress");
                return false;
            }

            if (!stream.ReadUint64(_value)) {
                Log::error("UnstakeRealWithdraw deserialize value");
                return false;
            }

            return true;
        }

        nlohmann::json UnstakeRealWithdraw::ToJson(uint8_t version) const {
            return nlohmann::json {
                    {"RetVotesTxHash", _retVotesTxHash},
                    {"StakeAddress", Address(_stakeAddress).String()},
                    {"Value", std::to_string(_value)}
            };
        }

        void UnstakeRealWithdraw::FromJson(const nlohmann::json &j, uint8_t version) {
            j.at("RetVotesTxHash").get_to(_retVotesTxHash);
            _stakeAddress = Address(j.at("StakeAddress").get<std::string>()).ProgramHash();

            BigInt bgValue;
            bgValue.setDec(j.at("Value").get<std::string>());
            _value = bgValue.getUint64();
        }

        bool UnstakeRealWithdraw::IsValid(uint8_t version) const {
            return true;
        }

        IPayload &UnstakeRealWithdraw::operator=(const IPayload &payload) {
            try {
                const UnstakeRealWithdraw &p= dynamic_cast<const UnstakeRealWithdraw&>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of UnstakeRealWithdraw");
            }

            return *this;
        }

        UnstakeRealWithdraw &UnstakeRealWithdraw::operator=(const UnstakeRealWithdraw &payload) {
            _retVotesTxHash = payload._retVotesTxHash;
            _stakeAddress = payload._stakeAddress;
            _value = payload._value;
            return *this;
        }

        bool UnstakeRealWithdraw::Equal(const IPayload &payload, uint8_t version) const {
            try {
                const UnstakeRealWithdraw &p= dynamic_cast<const UnstakeRealWithdraw&>(payload);
                return _retVotesTxHash == p._retVotesTxHash && _stakeAddress == p._stakeAddress && _value == p._value;
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of UnstakeRealWithdraw");
            }

            return false;
        }

    }
}