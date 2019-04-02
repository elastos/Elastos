// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"
//#include "BRInt.h"
#include <SDK/Common/Log.h>
#include "SDK/Plugin/Transaction/Payload/PayloadCoinBase.h"

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadCoinBase Test", "[PayloadCoinBase]") {

	SECTION("Serialize and deserialize") {
		PayloadCoinBase p1(getRandBytes(50)), p2;

		ByteStream stream;
		p1.Serialize(stream, 0);

		REQUIRE(p2.Deserialize(stream, 0));
		REQUIRE((p1.GetCoinBaseData() == p2.GetCoinBaseData()));
	}

	SECTION("to json and from json") {
		PayloadCoinBase p1(getRandBytes(50)), p2;

		nlohmann::json p1Json = p1.ToJson(0);
		p2.FromJson(p1Json, 0);

		REQUIRE((p1.GetCoinBaseData() == p2.GetCoinBaseData()));
	}
}
