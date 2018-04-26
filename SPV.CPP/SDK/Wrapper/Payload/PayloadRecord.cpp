// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadRecord.h"

namespace Elastos {
	namespace SDK {
		PayloadRecord::PayloadRecord() :
				_recordType("") {

		}

		PayloadRecord::~PayloadRecord() {

		}

		ByteData PayloadRecord::getData() const {
			//todo: implement IPayload interface
			ByteData data;
			return data;
		}

		void PayloadRecord::Serialize(ByteStream &ostream) const {

		}

		void PayloadRecord::Deserialize(ByteStream &istream) {

		}
	}
}