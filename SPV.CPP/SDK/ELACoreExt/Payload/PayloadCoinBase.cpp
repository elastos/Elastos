// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>

#include "PayloadCoinBase.h"
#include "BRInt.h"

namespace Elastos {
	namespace SDK {
		PayloadCoinBase::PayloadCoinBase() {

		}

		PayloadCoinBase::PayloadCoinBase(ByteData &coinBaseData) {
			_coinBaseData = coinBaseData;
		}

		PayloadCoinBase::~PayloadCoinBase() {
		}

		ByteData PayloadCoinBase::getData() const {
			return _coinBaseData;
		}

		void PayloadCoinBase::Serialize(ByteStream &ostream) const {
			ostream.putVarUint(_coinBaseData.length);
			if (_coinBaseData.length > 0) {
				ostream.putBytes(_coinBaseData.data, _coinBaseData.length);
			}
		}

		void PayloadCoinBase::Deserialize(ByteStream &istream) {
			uint64_t len = istream.getVarUint();
			_coinBaseData.data = new uint8_t[len];
			memset(_coinBaseData.data, 0, len);
			istream.getBytes(_coinBaseData.data, len);
			_coinBaseData.length = len;
		}
	}
}