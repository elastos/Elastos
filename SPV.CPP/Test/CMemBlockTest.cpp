// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"

#include <SDK/Common/CMemBlock.h>

using namespace Elastos::ElaWallet;

TEST_CASE("CMemBlock test", "[CMemBlock]") {
	SECTION("operator == test") {
		CMBlock cm1 = getRandCMBlock(100);
		CMBlock cm2 = getRandCMBlock(100);
		REQUIRE((cm1 != cm2));

		memcpy(cm2, cm1, 100);
		REQUIRE((cm1 == cm2));

		cm1.Resize(50);
		REQUIRE((cm1 != cm2));

		cm1 = cm2 = getRandCMBlock(100);
		REQUIRE((cm1 == cm2));

		cm1.Clear();
		REQUIRE((cm1 == cm2));

		cm1 = getRandCMBlock(50);
		cm2 = getRandCMBlock(100);
		REQUIRE((cm1 != cm2));
	}
}