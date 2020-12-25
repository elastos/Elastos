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

#ifndef __ELASTOS_SPVSDK_NEXTTURNDPOSINFO_H__
#define __ELASTOS_SPVSDK_NEXTTURNDPOSINFO_H__

#include <Plugin/Transaction/Payload/IPayload.h>

namespace Elastos {
	namespace ElaWallet {

		class NextTurnDPoSInfo : public IPayload {
		public:
			NextTurnDPoSInfo();

			NextTurnDPoSInfo(uint32_t blockHeight, const std::vector<bytes_t> &crPubkeys, const std::vector<bytes_t> &dposPubkeys);

			NextTurnDPoSInfo(const NextTurnDPoSInfo &payload);

			~NextTurnDPoSInfo();

			void SetWorkingHeight(uint32_t height);

			uint32_t GetWorkingHeight() const;

			void SetCRPublicKeys(const std::vector<bytes_t> pubkeys);

			std::vector<bytes_t> GetCRPublicKeys() const;

			void SetDPoSPublicKeys(const std::vector<bytes_t> pubkeys);

			std::vector<bytes_t> GetDPoSPublicKeys() const;

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			NextTurnDPoSInfo &operator=(const NextTurnDPoSInfo &payload);

			virtual bool Equal(const IPayload &payload, uint8_t version) const;
		private:
			uint32_t _workingHeight;
			std::vector<bytes_t> _crPublicKeys;
			std::vector<bytes_t> _dposPublicKeys;
		};

	}
}

#endif
