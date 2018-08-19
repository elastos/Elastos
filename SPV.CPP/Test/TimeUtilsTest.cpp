// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include "Wrapper/CheckPoint.h"
#include "Common/TimeUtils.h"
#include "KeyStore/CoinConfig.h"

using namespace Elastos::ElaWallet;

TEST_CASE("TimeUtils getCurrentTime test", "[getCurrentTime]") {
	SECTION("Normal test") {
		uint64_t currentTime = TimeUtils::getCurrentTime();
		REQUIRE(currentTime > 0);
	}
}

TEST_CASE("TimeUtils convertToString test", "[convertToString]") {
	SECTION("Start time format") {
		std::string strTime = TimeUtils::convertToString("%Y-%m-%d %H:%M:%S", 0);
		REQUIRE("1970-01-01 00:00:00" == strTime);
	}
	SECTION("Time of ELA publish") {
		std::string strTime = TimeUtils::convertToString("%Y-%m-%d %H:%M:%S", 1513936800);
		REQUIRE("2017-12-22 10:00:00" == strTime);
	}
}

TEST_CASE("TimeUtils calculateBlockHeightByTime test", "[calculateBlockHeightByTime]") {
	CoinConfig coinConfig;
	coinConfig.NetType = "TestNet";
	coinConfig.Type = Mainchain;
	ChainParams chainParams(coinConfig);

	SECTION("Calculat ELA publish height") {
		REQUIRE(TimeUtils::calculateBlockHeightByTime(1532629387, chainParams) == 0);
	}
	SECTION("Calculate earlear time than genesis check point") {
		REQUIRE(TimeUtils::calculateBlockHeightByTime(0, chainParams) == 0);
	}
	SECTION("Calculate normal time") {
		REQUIRE(TimeUtils::calculateBlockHeightByTime(1532629387 + 100 * 120, chainParams) == 100);
	}
}