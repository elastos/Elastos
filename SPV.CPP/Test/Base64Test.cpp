// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <Common/Base64.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE( "Decode method test", "[Base64]" ) {
	Log::registerMultiLogger();

	SECTION("test1") {
		bytes_t expectData("6515636B82C5AC56");

		bytes_t actual = Base64::Decode("ZRVja4LFrFY=");
		REQUIRE((actual == expectData));
	}

	SECTION("test2") {
		bytes_t expectData("9F62544C9D3FCAB2DD0833DF21CA80CF");

		bytes_t actual = Base64::Decode("n2JUTJ0/yrLdCDPfIcqAzw==");
		REQUIRE((actual == expectData));
	}

}

TEST_CASE( "Encode method test", "[Base64]" ) {

	SECTION("test1") {
		std::string expect = "ZRVja4LFrFY=";
		const unsigned char data[] = {
			0x65, 0x15, 0x63, 0x6B, 0x82, 0xC5, 0xAC, 0x56
		};
		std::string actual = Base64::Encode(data, sizeof(data));

		REQUIRE(expect == actual);
	}

	SECTION("test2") {
		std::string expect = "n2JUTJ0/yrLdCDPfIcqAzw==";

		const unsigned char data[] = {
			0x9F, 0x62, 0x54, 0x4C, 0x9D, 0x3F, 0xCA, 0xB2,
			0xDD, 0x08, 0x33, 0xDF, 0x21, 0xCA, 0x80, 0xCF
		};
		std::string actual = Base64::Encode(data, sizeof(data));

		REQUIRE(expect == actual);
	}

}
