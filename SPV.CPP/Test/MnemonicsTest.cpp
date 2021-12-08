// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <WalletCore/Mnemonic.h>
#include <Common/Log.h>

#include <boost/algorithm/string.hpp>

using namespace Elastos::ElaWallet;

TEST_CASE("Mnemonic words test", "[Mnemonic]") {
    Log::registerMultiLogger();

    SECTION("Word count 12") {
		std::vector<Mnemonic::WordCount> wordCount = {
			Mnemonic::WORDS_12,
			Mnemonic::WORDS_15,
			Mnemonic::WORDS_18,
			Mnemonic::WORDS_21,
			Mnemonic::WORDS_24
		};

		std::vector<std::string> lang = {
			"English",
			"ChineseSimplified",
			"ChineseTraditional",
			"Czech",
			"French",
			"Japanese",
			"Spanish",
			"Korean",
			"Italian",
			"Portuguese"
		};

		for (size_t k = 0; k < wordCount.size(); k++) {
			Mnemonic::WordCount wc = wordCount[k];
			for (size_t l = 0; l < lang.size(); l++) {
				for (size_t i = 0; i < 10; ++i) {
					std::string phrase;

					REQUIRE_NOTHROW(phrase = Mnemonic::Create(lang[l], wc));
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

					REQUIRE_NOTHROW(Mnemonic::DeriveSeed(phrase, ""));
				}
			}
		}
	}

    SECTION("mnemonic to seed") {
        uint512 constSeed("086c1a55a6c8acc657357195f5ea233a89df54e977323e285f958036700ff4a30ea486e90d3c07154e987d7e1b7d10436fc9f7280851f3689b413a61d26880aa");

        std::string mnemonic = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";

        uint512 seed = Mnemonic::DeriveSeed(mnemonic, "");

        REQUIRE(constSeed == seed);
    }

    SECTION("Decode mnemonic") {
        std::string mnemonic = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";

        bytes_t entropy = Mnemonic::Entropy(mnemonic, Language::ChineseSimplified);
        REQUIRE("e9b2c1aa5a85467099067cdb5a44f2ac" == entropy.getHex());

        WordList words = Mnemonic::Create(entropy, Language::ChineseSimplified);

        REQUIRE(mnemonic == boost::algorithm::join(words, " "));
    }

    SECTION("invalid mnemonic test") {
        uint512 seed;
        REQUIRE_THROWS(seed = Mnemonic::DeriveSeed("闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 县", ""));
        REQUIRE_THROWS(seed = Mnemonic::DeriveSeed("闲 齿 兰 丹 请 毛 训 胁 浇 摄 县", ""));
    }
}
