// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadRecord.h"
#include "BRAddress.h"
#include "BRInt.h"

namespace Elastos {
	namespace SDK {

		PayloadRecord::PayloadRecord() :
				_recordType("") {

		}

		PayloadRecord::PayloadRecord(const std::string &recordType, const ByteData recordData) {
			_recordType = recordType;
			_recordData = recordData;
		}

		PayloadRecord::~PayloadRecord() {

		}

		std::string PayloadRecord::getRecordType() const {
			return _recordType;
		}

		ByteData PayloadRecord::getRecordData() const {
			return _recordData;
		}

		ByteData PayloadRecord::getData() const {
			//todo: implement IPayload interface
			ByteData data;
			return data;
		}

		void PayloadRecord::Serialize(ByteStream &ostream) const {
			size_t len = _recordType.length();
			ostream.putVarUint(len);
			ostream.putBytes((uint8_t *) _recordType.c_str(), len);

			ostream.putVarUint(_recordData.length);
			if (_recordData.length > 0) {
				ostream.putBytes(_recordData.data, _recordData.length);
			}
		}

		void PayloadRecord::Deserialize(ByteStream &istream) {
			uint64_t len = istream.getVarUint();
			char *utfBuffer = new char[len + 1];
			istream.getBytes((uint8_t *) utfBuffer, len);
			utfBuffer[len] = '\0';
			_recordType = utfBuffer;
			delete[] utfBuffer;

			len = istream.getVarUint();
			uint8_t *buff = new uint8_t[len];
			if (len > 0) {
				istream.getBytes(buff, len);
			}
			_recordData = ByteData(buff, len);
			delete[] buff;
		}
	}
}