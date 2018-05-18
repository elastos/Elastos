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

		Attribute::Attribute(Attribute::Usage usage, const CMBlock &data) :
			_usage(usage),
			_data(data) {

		}

		Attribute::~Attribute() {
		}

		void Attribute::Serialize(ByteStream &ostream) const {
			ostream.put(_usage);

			ostream.putVarUint(_data.GetSize());
			ostream.putBytes(_data, _data.GetSize());
		}

		void Attribute::Deserialize(ByteStream &istream) {
			_usage = (Usage)istream.get();

			uint64_t len = istream.getVarUint();
			_data.Resize(len);
			istream.getBytes(_data, len);
		}
	}
}