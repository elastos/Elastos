// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>
#include <catch.hpp>

#include "BTCBase58.h"

using namespace Elastos::ElaWallet;

TEST_CASE("encode/decode", "[BTCBase58]") {
	SECTION("encode/decode") {
		BTCBase58 base58;
		unsigned char arr[] = {0, 1, 2, 3, 4};
		CMemBlock<unsigned char> arr_ret;

		std::string encoded = BTCBase58::EncodeBase58(arr, sizeof(arr));
		arr_ret = BTCBase58::DecodeBase58(encoded);


		REQUIRE(0 == memcmp(arr_ret, arr, sizeof(arr)));
	}
}
