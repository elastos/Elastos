// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"
#include <Plugin/Transaction/Payload/Record.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Record test", "Record") {
	Log::registerMultiLogger();

	SECTION("Serialize and deserialize") {
		Record p1(getRandString(20), getRandBytes(50)), p2;

		ByteStream stream;

		p1.Serialize(stream, 0);

		REQUIRE(p2.Deserialize(stream, 0));

		REQUIRE(p1.GetRecordType() == p2.GetRecordType());
		REQUIRE((p1.GetRecordData() == p2.GetRecordData()));
	}

	SECTION("to json and from json") {
		Record p1(getRandString(20), getRandBytes(50)), p2;

		nlohmann::json p1Json = p1.ToJson(0);

		p2.FromJson(p1Json, 0);

		REQUIRE(p1.GetRecordType() == p2.GetRecordType());
		REQUIRE(p1.GetRecordData() == p2.GetRecordData());
	}
}
