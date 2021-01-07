/*
 * Copyright (c) 2021 Elastos Foundation
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

#ifndef __ELASTOS_SPVSDK_PAYLOADCROSSCHAIN_H__
#define __ELASTOS_SPVSDK_PAYLOADCROSSCHAIN_H__

#include <Plugin/Transaction/Payload/OutputPayload/IOutputPayload.h>
#include <Common/BigInt.h>

namespace Elastos {
	namespace ElaWallet {

#define CrossChainOutputVersion 0x0

		class PayloadCrossChain : public IOutputPayload {
		public:
			PayloadCrossChain();

			PayloadCrossChain(uint8_t version, const std::string &addr, const BigInt &amount, const bytes_t &data);

			~PayloadCrossChain();

			uint8_t Version() const;

			const std::string &TargetAddress() const;

			const BigInt &TargetAmount() const;

			const bytes_t &TargetData() const;

			virtual size_t EstimateSize() const;

			virtual void Serialize(ByteStream &stream, bool extend = false) const;

			virtual bool Deserialize(const ByteStream &stream, bool extend = false);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			virtual IOutputPayload &operator=(const IOutputPayload &payload);

			PayloadCrossChain &operator=(const PayloadCrossChain &payload);

			virtual bool operator==(const IOutputPayload &payload) const;

		private:
			uint8_t _version;
			std::string _targetAddress;
			BigInt _targetAmount;
			bytes_t _targetData;
		};

	}
}

#endif
