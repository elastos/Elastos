// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <Common/uint256.h>
#include <Common/Utils.h>

using namespace Elastos::ElaWallet;

TEST_CASE("uint256 test", "[uint256]") {

	SECTION("GetHex and SetHex test") {
		bytes_t data = Utils::GetRandom(21);
		uint168 u168(data);

		uint168 temp168;
		temp168.SetHex(u168.GetHex());
		REQUIRE(temp168 == u168);

		uint160 u160(bytes_t(data.data() + 1, data.size() - 1));
		uint160 temp160;
		temp160.SetHex(u160.GetHex());
		REQUIRE(temp160 == u160);
	}

}

