// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>
#include <catch.hpp>
#include <SDK/Common/arith_uint256.h>


TEST_CASE("test", "[arith_uint256]") {
	arith_uint256 bu(3);
	bu <<= 3;
	REQUIRE(true == bu.EqualTo(24));
	bu >>= 3;
	REQUIRE(true == bu.EqualTo(3));

	uint32_t ui32_ct = bu.GetCompact();
	bu.SetCompact(ui32_ct);
	REQUIRE(true == bu.EqualTo(3));

	bu += 1;
	REQUIRE(true == bu.EqualTo(4));
	bu -= 1;
	REQUIRE(true == bu.EqualTo(3));
	bu *= 2;
	REQUIRE(true == bu.EqualTo(6));
	bu /= 2;
	REQUIRE(true == bu.EqualTo(3));
	bu |= 0;
	REQUIRE(true == bu.EqualTo(3));
	bu &= 0x03;
	REQUIRE(true == bu.EqualTo(3));
	bu ^= 0x03;
	REQUIRE(true == bu.EqualTo(0));
	bu = 3;
	arith_uint256 bu1(3);
	bu *= bu1;
	REQUIRE(true == bu.EqualTo(9));

	int pause = 0;
}