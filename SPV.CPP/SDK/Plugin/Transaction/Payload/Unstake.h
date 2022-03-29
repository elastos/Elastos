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

#ifndef __ELASTOS_SPVSDK_UNSTAKE_H__
#define __ELASTOS_SPVSDK_UNSTAKE_H__

#include "IPayload.h"

namespace Elastos {
    namespace ElaWallet {

        // 把没有投票的票权转换成ELA
        class Unstake : public IPayload {
        public:
            Unstake();

            ~Unstake();

            void SerializeUnsigned(ByteStream &stream, uint8_t version) const;

            bool DeserializeUnsigned(const ByteStream &stream, uint8_t version);

            nlohmann::json ToJsonUnsigned(uint8_t version) const;

            void FromJsonUnsigned(const nlohmann::json &j, uint8_t version);

            uint256 DigestUnstake(uint8_t version) const;
        public:
            virtual size_t EstimateSize(uint8_t version) const;

            virtual void Serialize(ByteStream &stream, uint8_t version) const;

            virtual bool Deserialize(const ByteStream &stream, uint8_t version);

            virtual nlohmann::json ToJson(uint8_t version) const;

            virtual void FromJson(const nlohmann::json &j, uint8_t version);

            virtual bool IsValid(uint8_t version) const;

            virtual IPayload &operator=(const IPayload &payload);

            Unstake &operator=(const Unstake &payload);

            virtual bool Equal(const IPayload &payload, uint8_t version) const;
        private:
            uint168 _toAddr;
            bytes_t _code;
            uint64_t _value;
            bytes_t _signature;
        };

    }
}

#endif
