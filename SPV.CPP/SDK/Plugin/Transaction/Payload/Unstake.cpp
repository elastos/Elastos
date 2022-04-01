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

#include "Unstake.h"
#include <WalletCore/Base58.h>
#include <Common/Log.h>
#include <WalletCore/Address.h>
#include <Common/BigInt.h>
#include <Common/hash.h>

namespace Elastos {
    namespace ElaWallet {

        Unstake::Unstake() {

        }

        Unstake::~Unstake() {

        }

        Unstake::Unstake(uint168 toAddr, const bytes_t &code, uint64_t value, const bytes_t &signature) :
            _toAddr(toAddr), _code(code), _value(value), _signature(signature) {

        }

        void Unstake::SerializeUnsigned(ByteStream &stream, uint8_t version) const {
            stream.WriteBytes(_toAddr);
            stream.WriteVarBytes(_code);
            stream.WriteUint64(_value);
        }

        bool Unstake::DeserializeUnsigned(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadBytes(_toAddr)) {
                Log::error("Unstake deserialize toAddr");
                return false;
            }

            if (!stream.ReadVarBytes(_code)) {
                Log::error("Unstake deserialize code");
                return false;
            }

            if (!stream.ReadUint64(_value)) {
                Log::error("Unstake deserialize value");
                return false;
            }

            return true;
        }

        nlohmann::json Unstake::ToJsonUnsigned(uint8_t version) const {
            return nlohmann::json {
                    {"ToAddress", Address(_toAddr).String()},
                    {"Code", _code.getHex()},
                    {"Value", std::to_string(_value)},
            };
        }

        void Unstake::FromJsonUnsigned(const nlohmann::json &j, uint8_t version) {
            _toAddr = Address(j.at("ToAddress").get<std::string>()).ProgramHash();
            _code.setHex(j.at("Code").get<std::string>());

            BigInt bgValue;
            bgValue.setDec(j.at("Value").get<std::string>());
            _value = bgValue.getUint64();
        }

        uint256 Unstake::DigestUnstake(uint8_t version) const {
            ByteStream stream;
            SerializeUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        size_t Unstake::EstimateSize(uint8_t version) const {
            size_t size = 0;
            ByteStream stream;

            size += _toAddr.size();
            size += stream.WriteVarUint(_code.size());
            size += _code.size();
            size += sizeof(_value);
            size += stream.WriteVarUint(_signature.size());
            size += _signature.size();

            return size;
        }

        void Unstake::Serialize(ByteStream &stream, uint8_t version) const {
            SerializeUnsigned(stream, version);
            stream.WriteVarBytes(_signature);
        }

        bool Unstake::Deserialize(const ByteStream &stream, uint8_t version) {
            if (!DeserializeUnsigned(stream, version)) {
                Log::error("Unstake deserialize unsigned");
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                Log::error("Unstake deserialize signature");
                return false;
            }

            return true;
        }

        nlohmann::json Unstake::ToJson(uint8_t version) const {
            nlohmann::json j = ToJsonUnsigned(version);
            j["Signature"] = _signature.getHex();
            return j;
        }

        void Unstake::FromJson(const nlohmann::json &j, uint8_t version) {
            FromJsonUnsigned(j, version);
            _signature.setHex(j.at("Signature").get<std::string>());
        }

        bool Unstake::IsValid(uint8_t version) const {
            return true;
        }

        IPayload &Unstake::operator=(const IPayload &payload) {
            try {
                const Unstake &p= dynamic_cast<const Unstake&>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of Unstake");
            }

            return *this;
        }

        Unstake &Unstake::operator=(const Unstake &payload) {
            _toAddr = payload._toAddr;
            _code = payload._code;
            _value = payload._value;
            _signature = payload._signature;
            return *this;
        }

        bool Unstake::Equal(const IPayload &payload, uint8_t version) const {
            try {
                const Unstake &p= dynamic_cast<const Unstake&>(payload);
                return _toAddr == p._toAddr && _code == p._code && _value == p._value && _signature == p._signature;
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of Unstake");
            }

            return false;
        }

    }
}
