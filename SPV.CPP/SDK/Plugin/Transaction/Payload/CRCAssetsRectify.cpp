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

#include "CRCAssetsRectify.h"
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		CRCAssetsRectify::CRCAssetsRectify() {

		}

		CRCAssetsRectify::CRCAssetsRectify(const CRCAssetsRectify &payload) {

		}

		CRCAssetsRectify::~CRCAssetsRectify() {

		}

		size_t CRCAssetsRectify::EstimateSize(uint8_t version) const {

			return 0;
		}

		void CRCAssetsRectify::Serialize(ByteStream &ostream, uint8_t version) const {

		}

		bool CRCAssetsRectify::Deserialize(const ByteStream &istream, uint8_t version) {

			return true;
		}

		nlohmann::json CRCAssetsRectify::ToJson(uint8_t version) const {

			return nlohmann::json();
		}

		void CRCAssetsRectify::FromJson(const nlohmann::json &j, uint8_t version) {

		}

		IPayload &CRCAssetsRectify::operator=(const IPayload &payload) {
			try {
				const CRCAssetsRectify &p= dynamic_cast<const CRCAssetsRectify &>(payload);
				operator=(p);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CRCAssetsRectify");
			}

			return *this;
		}

		CRCAssetsRectify &CRCAssetsRectify::operator=(const CRCAssetsRectify &payload) {

			return *this;
		}

		bool CRCAssetsRectify::Equal(const IPayload &payload, uint8_t version) const {
			try {
				const CRCAssetsRectify &p = dynamic_cast<const CRCAssetsRectify &>(payload);
				return true;
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CRCAssetsRectify");
			}

			return false;
		}

	}
}