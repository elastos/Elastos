// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRInt.h>
#include "Attribute.h"

namespace Elastos {
	namespace SDK {

		Attribute::Attribute() :
			_usage(Nonce) {
		}

		Attribute::Attribute(Attribute::Usage usage, const ByteData &data) :
			_usage(usage),
			_data(data) {

		}

		Attribute::~Attribute() {
			if (_data.data != nullptr) {
				delete[](_data.data);
			}
		}

		void Attribute::Serialize(std::istream &istream) const {
			uint8_t usageData[8 / 8];
			UInt8SetLE(usageData, uint8_t(_usage));
			istream >> usageData;

			uint8_t dataLengthData[64 / 8];
			UInt64SetLE(dataLengthData, _data.length);
			istream >> dataLengthData;

			istream >> _data.data;
		}

		void Attribute::Deserialize(std::ostream &ostream) {

		}
	}
}