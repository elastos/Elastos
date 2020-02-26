// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Payload/UnregisterCR.h>

using namespace Elastos::ElaWallet;

TEST_CASE("UnregisterCRTest test", "[UnregisterCRTest]") {
	SECTION("Serialize and deserialize") {
		UnregisterCR unregisterCR1;
		unregisterCR1.SetCID(uint168(PrefixIDChain, getRandBytes(20)));
		unregisterCR1.SetSignature(getRandBytes(36));

		ByteStream stream;
		unregisterCR1.Serialize(stream, 0);

		UnregisterCR unregisterCR2;
		REQUIRE(unregisterCR2.Deserialize(stream, 0));
		REQUIRE(unregisterCR2.GetCID() == unregisterCR1.GetCID());
		REQUIRE(unregisterCR2.GetSignature() == unregisterCR1.GetSignature());
	}

	SECTION("to json and from json") {
		UnregisterCR unregisterCR1;
		unregisterCR1.SetCID(uint168(PrefixIDChain, getRandBytes(20)));
		unregisterCR1.SetSignature(getRandBytes(36));

		UnregisterCR unregisterCR2;
		unregisterCR2.FromJson(unregisterCR1.ToJson(0), 0);

		REQUIRE(unregisterCR2.GetCID() == unregisterCR1.GetCID());
		REQUIRE(unregisterCR2.GetSignature() == unregisterCR1.GetSignature());
	}
}

