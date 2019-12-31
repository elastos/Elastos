// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <WalletCore/Mnemonic.h>
#include <Common/Log.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

#define ROOT_PATH "./Data/"

TEST_CASE("Mnemonic words test", "[Mnemonic]") {
	SECTION("Word count 12") {
		Mnemonic m(ROOT_PATH);

		std::vector<Mnemonic::WordCount> wordCount = {
			Mnemonic::WORDS_12,
			Mnemonic::WORDS_15,
			Mnemonic::WORDS_18,
			Mnemonic::WORDS_21,
			Mnemonic::WORDS_24
		};

		std::vector<std::string> lang = {
			"english",
			"chinese",
			"french",
			"japanese",
			"spanish"
		};

		for (size_t k = 0; k < wordCount.size(); k++) {
			Mnemonic::WordCount wc = wordCount[k];
			for (size_t l = 0; l < lang.size(); l++) {
				for (size_t i = 0; i < 10; ++i) {
					std::string phrase;

					REQUIRE_NOTHROW(phrase = m.Create(lang[l], wc));
					std::vector<std::string> words;
					boost::algorithm::split(words, phrase, boost::is_any_of(" \n\r\t"), boost::token_compress_on);
					words.erase(std::remove(words.begin(), words.end(), ""), words.end());

					if (wc == Mnemonic::WORDS_12)
						REQUIRE(words.size() == 12);
					else if (wc == Mnemonic::WORDS_15)
						REQUIRE(words.size() == 15);
					else if (wc == Mnemonic::WORDS_18)
						REQUIRE(words.size() == 18);
					else if (wc == Mnemonic::WORDS_21)
						REQUIRE(words.size() == 21);
					else if (wc == Mnemonic::WORDS_24)
						REQUIRE(words.size() == 24);

					REQUIRE_NOTHROW(m.DeriveSeed(phrase, ""));
				}
			}
		}

	}
}

TEST_CASE("Mnemonic of English test", "[English]") {
	Log::registerMultiLogger();
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
	REQUIRE_THROWS(seed = mnemonic.DeriveSeed("闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 县", ""));
	REQUIRE_THROWS(seed = mnemonic.DeriveSeed("闲 齿 兰 丹 请 毛 训 胁 浇 摄 县", ""));
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