// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include "PayloadRecord.h"
#include "BRAddress.h"
#include "BRInt.h"

namespace Elastos {
	namespace ElaWallet {

		PayloadRecord::PayloadRecord() :
				_recordType("") {

		}

		PayloadRecord::PayloadRecord(const std::string &recordType, const CMBlock &recordData) {
			_recordType = recordType;
			_recordData = recordData;
		}

		PayloadRecord::~PayloadRecord() {
		}

		void PayloadRecord::setRecordType(const std::string &recordType) {
			_recordType = recordType;
		}

		void PayloadRecord::setRecordData(const CMBlock &recordData) {
			_recordData = recordData;
		}

		std::string PayloadRecord::getRecordType() const {
			return _recordType;
		}

		CMBlock PayloadRecord::getRecordData() const {
			return _recordData;
		}

		CMBlock PayloadRecord::getData() const {
			ByteStream stream;
			Serialize(stream);

			return stream.getBuffer();
		}

		void PayloadRecord::Serialize(ByteStream &ostream) const {
			ostream.writeVarString(_recordType);
			ostream.writeVarBytes(_recordData);
		}

		bool PayloadRecord::Deserialize(ByteStream &istream) {
			if (!istream.readVarString(_recordType)) {
				Log::getLogger()->error("Payload record deserialize type fail");
				return false;
			}

			if (!istream.readVarBytes(_recordData)) {
				Log::getLogger()->error("Payload record deserialize data fail");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRecord::toJson() const {
			nlohmann::json j;

			j["RecordType"] = _recordType;
			j["RecordData"] = Utils::encodeHex(_recordData);

			return j;
		}

		void PayloadRecord::fromJson(const nlohmann::json &j) {
			_recordType = j["RecordType"].get<std::string>();
			_recordData = Utils::decodeHex(j["RecordData"].get<std::string>());
		}
	}
}