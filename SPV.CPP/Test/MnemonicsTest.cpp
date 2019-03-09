// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN


#include "catch.hpp"
#include <SDK/KeyStore/Mnemonic.h>
#include <Core/BRBIP39Mnemonic.h>

using namespace Elastos::ElaWallet;

#define ROOT_PATH "./Data/"

TEST_CASE("Mnemonic of English test", "[English]") {
	Mnemonic mnemonic("english", ROOT_PATH);
	REQUIRE(mnemonic.Words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.Words()[0] == "abandon");
	REQUIRE(mnemonic.Words()[BIP39_WORDLIST_COUNT - 1] == "zoo");
}

TEST_CASE("Mnemonic of Chinese test", "[Chinese]") {
	Mnemonic mnemonic("chinese", ROOT_PATH);
	REQUIRE(mnemonic.Words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.Words()[0] == "的");
	REQUIRE(mnemonic.Words()[BIP39_WORDLIST_COUNT - 1] == "歇");
}

TEST_CASE("Mnemonic of French test", "[French]") {
	Mnemonic mnemonic("french", ROOT_PATH);
	REQUIRE(mnemonic.Words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.Words()[0] == "abaisser");
	REQUIRE(mnemonic.Words()[BIP39_WORDLIST_COUNT - 1] == "zoologie");
}

TEST_CASE("Mnemonic of Italian test", "[Italian]") {
	Mnemonic mnemonic("italian", ROOT_PATH);
	REQUIRE(mnemonic.Words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.Words()[0] == "abaco");
	REQUIRE(mnemonic.Words()[BIP39_WORDLIST_COUNT - 1] == "zuppa");
}

TEST_CASE("Mnemonic of Japanese test", "[Japanese]") {
	Mnemonic mnemonic("japanese", ROOT_PATH);
	REQUIRE(mnemonic.Words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.Words()[0] == "あいこくしん");
	REQUIRE(mnemonic.Words()[BIP39_WORDLIST_COUNT - 1] == "われる");
}

TEST_CASE("Mnemonic of Spanish test", "[Spanish]") {
	Mnemonic mnemonic("spanish", ROOT_PATH);
	REQUIRE(mnemonic.Words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.Words()[0] == "ábaco");
	REQUIRE(mnemonic.Words()[BIP39_WORDLIST_COUNT - 1] == "zurdo");
}