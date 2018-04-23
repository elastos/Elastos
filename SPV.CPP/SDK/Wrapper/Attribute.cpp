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

		void Attribute::Serialize(ByteStream &ostream) const {
			ostream.put(_usage);

			uint8_t dataLengthData[64 / 8];
			UInt64SetLE(dataLengthData, _data.length);
			ostream.putBytes(dataLengthData, 64 / 8);

			ostream.putBytes(_data.data, _data.length);
		}

		void Attribute::Deserialize(ByteStream &istream) {

		}
	}
}