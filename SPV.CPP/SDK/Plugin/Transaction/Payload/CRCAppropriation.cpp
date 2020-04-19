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

#include <Common/Log.h>
#include "CRCAppropriation.h"

namespace Elastos {
	namespace ElaWallet {

		CRCAppropriation::CRCAppropriation() {
		}

		CRCAppropriation::CRCAppropriation(const CRCAppropriation &payload) {
			operator=(payload);
		}

		CRCAppropriation::~CRCAppropriation() {
		}

		size_t CRCAppropriation::EstimateSize(uint8_t version) const {
			return 0;
		}

		void CRCAppropriation::Serialize(ByteStream &ostream, uint8_t version) const {

		}

		bool CRCAppropriation::Deserialize(const ByteStream &istream, uint8_t version) {
			return true;
		}

		nlohmann::json CRCAppropriation::ToJson(uint8_t version) const {
			return nlohmann::json();
		}

		void CRCAppropriation::FromJson(const nlohmann::json &j, uint8_t version) {

		}

		IPayload &CRCAppropriation::operator=(const IPayload &payload) {
			try {
				const CRCAppropriation &payloadCRCAppropriation  = dynamic_cast<const CRCAppropriation &>(payload);
				operator=(payloadCRCAppropriation);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of TransferAsset");
			}

			return *this;
		}

		CRCAppropriation &CRCAppropriation::operator=(const CRCAppropriation &payload) {
			return *this;
		}

	}
}