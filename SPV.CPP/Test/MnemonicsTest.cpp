// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN


#include "catch.hpp"
#include <SDK/BIPs/Mnemonic.h>

using namespace Elastos::ElaWallet;

#define ROOT_PATH "./Data/"

TEST_CASE("Mnemonic of English test", "[English]") {
	Mnemonic mnemonic(ROOT_PATH);

	std::string phrase;

	REQUIRE_NOTHROW(phrase = mnemonic.Create("english"));

	uint512 seed;
	REQUIRE_NOTHROW(seed = mnemonic.DeriveSeed(phrase, ""));
	REQUIRE(seed != 0);
}

TEST_CASE("Mnemonic of Chinese test", "[Chinese]") {
	Mnemonic mnemonic(ROOT_PATH);

	std::string phrase;

	REQUIRE_NOTHROW(phrase = mnemonic.Create("chinese"));

	uint512 seed;
	REQUIRE_NOTHROW(seed = mnemonic.DeriveSeed(phrase, "123"));
	REQUIRE(seed != 0);
}

TEST_CASE("Mnemonic of French test", "[French]") {
	Mnemonic mnemonic(ROOT_PATH);

	std::string phrase;

	REQUIRE_NOTHROW(phrase = mnemonic.Create("french"));

	uint512 seed;
	REQUIRE_NOTHROW(seed = mnemonic.DeriveSeed(phrase, "456"));
	REQUIRE(seed != 0);
}

TEST_CASE("Mnemonic of Italian test", "[Italian]") {
	Mnemonic mnemonic(ROOT_PATH);

	std::string phrase;

	REQUIRE_NOTHROW(phrase = mnemonic.Create("italian"));

	uint512 seed;
	REQUIRE_NOTHROW(seed = mnemonic.DeriveSeed(phrase, "abc"));
	REQUIRE(seed != 0);
}

TEST_CASE("Mnemonic of Japanese test", "[Japanese]") {
	Mnemonic mnemonic(ROOT_PATH);

	std::string phrase;

	REQUIRE_NOTHROW(phrase = mnemonic.Create("japanese"));

	uint512 seed;
	REQUIRE_NOTHROW(seed = mnemonic.DeriveSeed(phrase, "hello"));
	REQUIRE(seed != 0);
}

TEST_CASE("Mnemonic of Spanish test", "[Spanish]") {
	Mnemonic mnemonic(ROOT_PATH);

	std::string phrase;

	REQUIRE_NOTHROW(phrase = mnemonic.Create("spanish"));

	uint512 seed;
	REQUIRE_NOTHROW(seed = mnemonic.DeriveSeed(phrase, "world"));
	REQUIRE(seed != 0);
}