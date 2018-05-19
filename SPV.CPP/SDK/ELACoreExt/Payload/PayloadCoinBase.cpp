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

		PayloadCoinBase::PayloadCoinBase(CMBlock &coinBaseData) {
			_coinBaseData = coinBaseData;
		}

		PayloadCoinBase::~PayloadCoinBase() {
		}

		CMBlock PayloadCoinBase::getData() const {
            return _coinBaseData;
        }

		void PayloadCoinBase::setCoinBaseData(const CMBlock &coinBaseData) {
			_coinBaseData = coinBaseData;
		}

		void PayloadCoinBase::Serialize(ByteStream &ostream) const {
			ostream.putVarUint(_coinBaseData.GetSize());
			if (_coinBaseData.GetSize() > 0) {
				ostream.putBytes(_coinBaseData, _coinBaseData.GetSize());
			}
		}

		void PayloadCoinBase::Deserialize(ByteStream &istream) {
			uint64_t len = istream.getVarUint();

			if (0 < len) {
				CMBlock data(len);
				if (data) {
					istream.getBytes(data, len);
					_coinBaseData.Resize(len);
					memcpy(_coinBaseData, data, len);
				}
			}
		}
	}
}