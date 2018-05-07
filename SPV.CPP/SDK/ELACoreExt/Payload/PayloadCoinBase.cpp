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

		void PayloadCoinBase::setCoinBaseData(const ByteData &coinBaseData) {
		    uint8_t* buf = new uint8_t[coinBaseData.length];
		    memcpy(buf, coinBaseData.data, coinBaseData.length);
			_coinBaseData.data = buf;
			_coinBaseData.length = coinBaseData.length;
		}

//		ByteData PayloadCoinBase::getData() const{
//			ByteStream stream;
//			Serialize(stream);
//			uint8_t *buf = stream.getBuf();
//			uint64_t len = stream.length();
//
//			return ByteData(buf, len);
		    //return _coinBaseData;
//		}

		void PayloadCoinBase::Serialize(ByteStream &ostream) const {
			ostream.putVarUint(_coinBaseData.length);
			if (_coinBaseData.length > 0) {
				ostream.putBytes(_coinBaseData.data, _coinBaseData.length);
			}
		}

		void PayloadCoinBase::Deserialize(ByteStream &istream) {
			uint64_t len = istream.getVarUint();

			if (0 < len) {
				uint8_t *data = new uint8_t[len];
				if (data) {
					memset(data, 0, len);
					istream.getBytes(data, len);

					if (_coinBaseData.data && data) {
						delete[] _coinBaseData.data;
						_coinBaseData.data = nullptr;
					}

					_coinBaseData = ByteData(data, len);
				}
			}
		}
	}
}