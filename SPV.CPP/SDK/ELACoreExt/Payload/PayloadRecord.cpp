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
			if (recordData.data && 0 < recordData.length) {
				uint8_t *buf = new uint8_t[recordData.length];
				memcpy(buf, recordData.data, recordData.length);
				_recordData = ByteData(buf, recordData.length);
			}
		}

		PayloadRecord::~PayloadRecord() {
			if (_recordData.data) {
				delete[] _recordData.data;
			}
		}

		void PayloadRecord::setRecordType(const std::string &recordType) {
			_recordType = recordType;
		}

		void PayloadRecord::setRecordData(const ByteData recordData) {
			_recordData = recordData;
		}

		std::string PayloadRecord::getRecordType() const {
			return _recordType;
		}

		ByteData PayloadRecord::getRecordData() const {
			return _recordData;
		}

		ByteData PayloadRecord::getData() const {
			//todo: implement IPayload interface
			ByteStream stream;
			Serialize(stream);

			return ByteData(stream.getBuf(), stream.length());
			//return data;
		}

		void PayloadRecord::Serialize(ByteStream &ostream) const {
			size_t len = _recordType.length();
			ostream.putVarUint(len);
			ostream.putBytes((uint8_t *)_recordType.c_str(), (uint64_t)len);

			ostream.putVarUint(_recordData.length);
			if (_recordData.length > 0) {
				ostream.putBytes(_recordData.data, _recordData.length);
			}
		}

		void PayloadRecord::Deserialize(ByteStream &istream) {
			uint64_t len = istream.getVarUint();
			if (0 < len) {
				char *utfBuffer = new char[len + 1];
				if (utfBuffer) {
					istream.getBytes((uint8_t *) utfBuffer, len);
					utfBuffer[len] = '\0';
					_recordType = utfBuffer;
					delete[] utfBuffer;
				}
			}

			len = istream.getVarUint();
			if (0 < len) {
				uint8_t *buff = new uint8_t[len];
				if (buff) {
					istream.getBytes(buff, len);
					_recordData = ByteData(buff, len);
				}
			}
			//delete[] buff;
		}
	}
}