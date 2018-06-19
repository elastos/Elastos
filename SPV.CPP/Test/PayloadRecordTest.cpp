// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "Payload/PayloadRecord.h"

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadRecord test", "PayloadRecord") {

	SECTION("PayloadRecord interface test") {
		std::string recordType = "a test recordType";
		uint8_t script[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24, 60, 75, 8, 182, 57, 98};
		CMBlock data;
		data.SetMemFixed(script, sizeof(script));

		PayloadRecord record(recordType, data);
		REQUIRE(record.getRecordType() == recordType);
		CMBlock recordData = record.getRecordData();
		REQUIRE(recordData.GetSize() == data.GetSize());
		int result = memcmp(recordData, data, data.GetSize());
		REQUIRE(result == 0);

		ByteStream byteStream;
		record.Serialize(byteStream);
		byteStream.setPosition(0);
		REQUIRE(byteStream.length() > 0);

		PayloadRecord record1;
		record1.Deserialize(byteStream);

		REQUIRE(record1.getRecordType() == record.getRecordType());
		CMBlock recordData1 = record1.getRecordData();
		REQUIRE(recordData1.GetSize() == recordData.GetSize());
		result = memcmp(recordData1, recordData, recordData1.GetSize());

		REQUIRE(result == 0);
	}
}
