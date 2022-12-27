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
#include "Stake.h"

namespace Elastos {
    namespace ElaWallet {

        Stake::Stake() {

        }

        Stake::~Stake() {

        }

        size_t Stake::EstimateSize(uint8_t version) const {
            return 0;
        }

        void Stake::Serialize(ByteStream &stream, uint8_t version) const {

        }

        bool Stake::Deserialize(const ByteStream &stream, uint8_t version) {
            return true;
        }

        nlohmann::json Stake::ToJson(uint8_t version) const {
            return nlohmann::json();
        }

        void Stake::FromJson(const nlohmann::json &j, uint8_t version) {

        }

        bool Stake::IsValid(uint8_t version) const {
            return true;
        }

        IPayload &Stake::operator=(const IPayload &payload) {
            try {
                const Stake &p= dynamic_cast<const Stake&>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of Stake");
            }

            return *this;
        }

        Stake &Stake::operator=(const Stake &payload) {
            return *this;
        }

        bool Stake::Equal(const IPayload &payload, uint8_t version) const {
            try {
                const Stake &p= dynamic_cast<const Stake&>(payload);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of Stake");
                return false;
            }

            return true;
        }

    }
}