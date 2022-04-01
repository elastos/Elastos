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

#include "CancelVotes.h"
#include <Common/ByteStream.h>
#include <Common/Log.h>
#include <Common/JsonSerializer.h>

namespace Elastos {
    namespace ElaWallet {

        CancelVotes::CancelVotes() {

        }

        CancelVotes::~CancelVotes() {

        }

        CancelVotes::CancelVotes(const std::vector<uint256> &referKeys) :
            _referKeys(referKeys) {

        }

        size_t CancelVotes::EstimateSize(uint8_t version) const {
            ByteStream stream;
            size_t size = 0;

            size += stream.WriteVarUint(_referKeys.size());
            for (const uint256 &h : _referKeys)
                size += h.size();

            return size;
        }

        void CancelVotes::Serialize(ByteStream &stream, uint8_t version) const {
            stream.WriteVarUint(_referKeys.size());
            for (const uint256 &h : _referKeys)
                stream.WriteBytes(h);
        }

        bool CancelVotes::Deserialize(const ByteStream &stream, uint8_t version) {
            uint64_t size = 0;

            if (!stream.ReadVarUint(size)) {
                Log::error("CancelVotes deserialize size");
                return false;
            }

            for (size_t i = 0; i < size; ++i) {
                uint256 h;
                if (!stream.ReadBytes(h)) {
                    Log::error("CancelVotes deserialize referKeys[{}]", i);
                    return false;
                }
                _referKeys.push_back(h);
            }

            return true;
        }

        nlohmann::json CancelVotes::ToJson(uint8_t version) const {
            return nlohmann::json{
                    {"ReferKeys", _referKeys}
            };
        }

        void CancelVotes::FromJson(const nlohmann::json &j, uint8_t version) {
            j.at("ReferKeys").get_to(_referKeys);
        }

        bool CancelVotes::IsValid(uint8_t version) const {
            return true;
        }

        IPayload &CancelVotes::operator=(const IPayload &payload) {
            try {
                const CancelVotes &p= dynamic_cast<const CancelVotes &>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of CancelVotes");
            }

            return *this;
        }

        CancelVotes &CancelVotes::operator=(const CancelVotes &payload) {
            _referKeys = payload._referKeys;
            return *this;
        }

        bool CancelVotes::Equal(const IPayload &payload, uint8_t version) const {
            try {
                const CancelVotes &p= dynamic_cast<const CancelVotes &>(payload);
                if (_referKeys.size() != p._referKeys.size()) {
                    return false;
                }

                for (size_t i = 0; i < _referKeys.size(); ++i) {
                    if (_referKeys[i] != p._referKeys[i])
                        return false;
                }
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of CancelVotes");
                return false;
            }

            return true;
        }

    }
}