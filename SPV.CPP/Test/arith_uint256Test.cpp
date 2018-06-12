// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>
#include <catch.hpp>

#include "arith_uint256.h"

TEST_CASE("test", "[arith_uint256]") {
	base_uint<256> bu(3);
	bu << 3;
	int pause = 0;
}