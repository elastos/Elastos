// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "BRBIP39Mnemonic.h"

#include "catch.hpp"
#include "KeyStore/Mnemonic.h"

using namespace Elastos::ElaWallet;

#define I18N_RELATIVE_PATH "./Data/"

TEST_CASE("Mnemonic of English test", "[English]") {

	Mnemonic mnemonic;
	REQUIRE(mnemonic.words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.words()[0] == "abandon");
	REQUIRE(mnemonic.words()[BIP39_WORDLIST_COUNT - 1] == "zoo");
}

TEST_CASE("Mnemonic of Chinese test", "[Chinese]") {

	Mnemonic mnemonic("chinese", I18N_RELATIVE_PATH);
	REQUIRE(mnemonic.words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.words()[0] == "的");
	REQUIRE(mnemonic.words()[BIP39_WORDLIST_COUNT - 1] == "歇");
}

TEST_CASE("Mnemonic of French test", "[French]") {

	Mnemonic mnemonic("french", I18N_RELATIVE_PATH);
	REQUIRE(mnemonic.words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.words()[0] == "abaisser");
	REQUIRE(mnemonic.words()[BIP39_WORDLIST_COUNT - 1] == "zoologie");
}

TEST_CASE("Mnemonic of Italian test", "[Italian]") {

	Mnemonic mnemonic("italian", I18N_RELATIVE_PATH);
	REQUIRE(mnemonic.words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.words()[0] == "abaco");
	REQUIRE(mnemonic.words()[BIP39_WORDLIST_COUNT - 1] == "zuppa");
}

TEST_CASE("Mnemonic of Japanese test", "[Japanese]") {

	Mnemonic mnemonic("japanese", I18N_RELATIVE_PATH);
	REQUIRE(mnemonic.words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.words()[0] == "あいこくしん");
	REQUIRE(mnemonic.words()[BIP39_WORDLIST_COUNT - 1] == "われる");
}

TEST_CASE("Mnemonic of Spanish test", "[Spanish]") {

	Mnemonic mnemonic("spanish", I18N_RELATIVE_PATH);
	REQUIRE(mnemonic.words().size() == BIP39_WORDLIST_COUNT);
	REQUIRE(mnemonic.words()[0] == "ábaco");
	REQUIRE(mnemonic.words()[BIP39_WORDLIST_COUNT - 1] == "zurdo");
}