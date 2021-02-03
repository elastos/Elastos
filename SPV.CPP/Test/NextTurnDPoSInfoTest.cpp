/*
 * Copyright (c) 2020 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"
#include <Common/Log.h>
#include <Plugin/Transaction/Payload/NextTurnDPoSInfo.h>

using namespace Elastos::ElaWallet;

static void InitRandNextTurnDPoSInfo(NextTurnDPoSInfo &p) {
	std::vector<bytes_t> pubkeys;
	p.SetWorkingHeight(getRandUInt32());

	for (size_t i = 0; i < 20; ++i) {
		pubkeys.push_back(getRandBytes(33));
	}
	p.SetCRPublicKeys(pubkeys);

	pubkeys.clear();
	for (size_t i = 0; i < 20; ++i) {
		pubkeys.push_back(getRandBytes(33));
	}
	p.SetDPoSPublicKeys(pubkeys);
}

TEST_CASE("NextTurnDPoSInfo Test", "[Payload]") {
	Log::registerMultiLogger();

	SECTION("Serialize and deserialize") {
		NextTurnDPoSInfo p1, p2;

		InitRandNextTurnDPoSInfo(p1);

		ByteStream stream;
		p1.Serialize(stream, 0);

		REQUIRE(p2.Deserialize(stream, 0));

		REQUIRE((p1.GetWorkingHeight() == p2.GetWorkingHeight()));
		REQUIRE(p1.GetCRPublicKeys() == p2.GetCRPublicKeys());
		REQUIRE(p1.GetDPoSPublicKeys() == p2.GetDPoSPublicKeys());
	}

	SECTION("to json and from json") {
		NextTurnDPoSInfo p1, p2;

		InitRandNextTurnDPoSInfo(p1);

		nlohmann::json p1Json = p1.ToJson(0);
		p2.FromJson(p1Json, 0);

		REQUIRE((p1.GetWorkingHeight() == p2.GetWorkingHeight()));
		REQUIRE(p1.GetCRPublicKeys() == p2.GetCRPublicKeys());
		REQUIRE(p1.GetDPoSPublicKeys() == p2.GetDPoSPublicKeys());
	}
}
