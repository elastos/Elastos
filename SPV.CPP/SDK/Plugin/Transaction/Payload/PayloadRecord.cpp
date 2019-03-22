// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadRecord.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadRecord::PayloadRecord() :
				_recordType("") {

		}

		PayloadRecord::PayloadRecord(const std::string &recordType, const bytes_t &recordData) {
			_recordType = recordType;
			_recordData = recordData;
		}

		PayloadRecord::PayloadRecord(const PayloadRecord &payload) {
			operator=(payload);
		}

		PayloadRecord::~PayloadRecord() {
		}

		void PayloadRecord::SetRecordType(const std::string &recordType) {
			_recordType = recordType;
		}

		void PayloadRecord::SetRecordData(const bytes_t &recordData) {
			_recordData = recordData;
		}

		std::string PayloadRecord::GetRecordType() const {
			return _recordType;
		}

		bytes_t PayloadRecord::GetRecordData() const {
			return _recordData;
		}

		void PayloadRecord::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarString(_recordType);
			ostream.WriteVarBytes(_recordData);
		}

		bool PayloadRecord::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadVarString(_recordType)) {
				Log::error("Payload record deserialize type fail");
				return false;
			}

			if (!istream.ReadVarBytes(_recordData)) {
				Log::error("Payload record deserialize data fail");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRecord::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["RecordType"] = _recordType;
			j["RecordData"] = _recordData.getHex();

			return j;
		}

		void PayloadRecord::FromJson(const nlohmann::json &j, uint8_t version) {
			_recordType = j["RecordType"].get<std::string>();
			_recordData.setHex(j["RecordData"].get<std::string>());
		}

		IPayload &PayloadRecord::operator=(const IPayload &payload) {
			try {
				const PayloadRecord &payloadRecord = dynamic_cast<const PayloadRecord &>(payload);
				operator=(payloadRecord);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadRecord");
			}

			return *this;
		}

		PayloadRecord &PayloadRecord::operator=(const PayloadRecord &payload) {
			_recordData = payload._recordData;
			_recordType = payload._recordType;

			return *this;
		}

	}
}