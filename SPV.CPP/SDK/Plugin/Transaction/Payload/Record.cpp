// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Record.h"

#include <Common/Log.h>
#include <Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		Record::Record() :
				_recordType("") {

		}

		Record::Record(const std::string &recordType, const bytes_t &recordData) {
			_recordType = recordType;
			_recordData = recordData;
		}

		Record::Record(const Record &payload) {
			operator=(payload);
		}

		Record::~Record() {
		}

		void Record::SetRecordType(const std::string &recordType) {
			_recordType = recordType;
		}

		void Record::SetRecordData(const bytes_t &recordData) {
			_recordData = recordData;
		}

		std::string Record::GetRecordType() const {
			return _recordType;
		}

		bytes_t Record::GetRecordData() const {
			return _recordData;
		}

		size_t Record::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_recordType.size());
			size += _recordType.size();
			size += stream.WriteVarUint(_recordData.size());
			size += _recordData.size();

			return size;
		}

		void Record::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarString(_recordType);
			ostream.WriteVarBytes(_recordData);
		}

		bool Record::Deserialize(const ByteStream &istream, uint8_t version) {
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

		nlohmann::json Record::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["RecordType"] = _recordType;
			j["RecordData"] = _recordData.getHex();

			return j;
		}

		void Record::FromJson(const nlohmann::json &j, uint8_t version) {
			_recordType = j["RecordType"].get<std::string>();
			_recordData.setHex(j["RecordData"].get<std::string>());
		}

		IPayload &Record::operator=(const IPayload &payload) {
			try {
				const Record &payloadRecord = dynamic_cast<const Record &>(payload);
				operator=(payloadRecord);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of Record");
			}

			return *this;
		}

		Record &Record::operator=(const Record &payload) {
			_recordData = payload._recordData;
			_recordType = payload._recordType;

			return *this;
		}

	}
}