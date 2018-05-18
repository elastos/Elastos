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

		PayloadRecord::PayloadRecord(const std::string &recordType, const CMBlock recordData) {
			_recordType = recordType;
			_recordData = recordData;
		}

		PayloadRecord::~PayloadRecord() {
		}

		void PayloadRecord::setRecordType(const std::string &recordType) {
			_recordType = recordType;
		}

		void PayloadRecord::setRecordData(const CMBlock recordData) {
			_recordData = recordData;
		}

		std::string PayloadRecord::getRecordType() const {
			return _recordType;
		}

		CMBlock PayloadRecord::getRecordData() const {
			return _recordData;
		}

		CMBlock PayloadRecord::getData() const {
			//todo: implement IPayload interface
			ByteStream stream;
			Serialize(stream);

			CMBlock ret(stream.length());
			uint8_t *tmp = stream.getBuf();
			memcpy(ret, tmp, stream.length());
			delete []tmp;

			return ret;
		}

		void PayloadRecord::Serialize(ByteStream &ostream) const {
			size_t len = _recordType.length();
			ostream.putVarUint(len);
			ostream.putBytes((uint8_t *)_recordType.c_str(), (uint64_t)len);

			ostream.putVarUint(_recordData.GetSize());
			if (_recordData.GetSize() > 0) {
				ostream.putBytes(_recordData, _recordData.GetSize());
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
					_recordData.Resize(len);
					memcpy(_recordData, buff, len);
				}
			}
			//delete[] buff;
		}
	}
}