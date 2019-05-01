// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include <SDK/WalletCore/BIPs/BIP39.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/Common/Log.h>

#include <boost/algorithm/string/split.hpp>

using namespace Elastos::ElaWallet;

TEST_CASE("BIP39 test", "[BIP39]") {
	Log::registerMultiLogger();

	SECTION("BIP39 to seed") {
		uint512 constSeed("086c1a55a6c8acc657357195f5ea233a89df54e977323e285f958036700ff4a30ea486e90d3c07154e987d7e1b7d10436fc9f7280851f3689b413a61d26880aa");

		std::string mnemonic = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";

		uint512 seed = BIP39::DeriveSeed(mnemonic);

		REQUIRE(constSeed == seed);
	}

	SECTION("Decode mnemonic") {
		std::fstream infile("Data/mnemonic_chinese.txt");
		std::string line;
		std::vector<std::string> dictionary;
		std::string mnemonic = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";

		while (std::getline(infile, line)) {
			dictionary.push_back(line);
		}

		REQUIRE(dictionary.size() == 2048);

		bytes_t entropy = BIP39::Decode(dictionary, mnemonic);

		REQUIRE("e9b2c1aa5a85467099067cdb5a44f2ac" == entropy.getHex());
	}

	SECTION("Encode mnemonic") {
		std::fstream infile("Data/mnemonic_chinese.txt");
		std::string line;
		std::vector<std::string> dictionary;
		std::string mnemonic = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";

		while (std::getline(infile, line)) {
			dictionary.push_back(line);
		}

		REQUIRE(dictionary.size() == 2048);

		bytes_t entropy("e9b2c1aa5a85467099067cdb5a44f2ac");
		std::string result = BIP39::Encode(dictionary, entropy);
		REQUIRE(result == mnemonic);
	}

}
