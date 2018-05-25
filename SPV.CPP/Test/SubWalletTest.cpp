// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>

#include "catch.hpp"

#include "MasterWallet.h"
#include "SubWallet.h"

using namespace Elastos::SDK;

class TestMasterWallet : public MasterWallet {
public:
	TestMasterWallet() {
	}
};

class TestSubWallet : public SubWallet {
public:
	TestSubWallet(const CoinInfo &info,
				  const std::string &payPassword,
				  MasterWallet *parent) :
			SubWallet(info, ChainParams::mainNet(), payPassword, parent) {
	}
};

TEST_CASE("Sub wallet basic", "[MasterWallet]") {

	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet);

	CoinInfo info;
	std::string payPassword = "payPassword";
	boost::scoped_ptr<TestSubWallet> subWallet(new TestSubWallet(info, payPassword, masterWallet.get()));

	SECTION("Address related") {
		nlohmann::json j = subWallet->GetAllAddress(0, INT_MAX);
		std::vector<std::string> addresses = j["addresses"].get<std::vector<std::string>>();
		size_t addressSize = 0;
		REQUIRE(addresses.size() == addressSize);

		std::string newAddress = subWallet->CreateAddress();
		REQUIRE(!newAddress.empty());

		nlohmann::json j2 = subWallet->GetAllAddress(0, INT_MAX);
		std::vector<std::string> addresses2 = j2["addresses"].get<std::vector<std::string>>();
		REQUIRE(addresses.size() + 1 == addresses2.size());
		REQUIRE(std::find(addresses2.begin(), addresses2.end(), newAddress) != addresses2.end());
	}
	SECTION("Balance related") {
		REQUIRE(subWallet->GetBalance() == 0);

		std::string balanceInfo = subWallet->GetBalanceInfo();
		REQUIRE(balanceInfo.empty());

		std::string newAddress = subWallet->CreateAddress();
		REQUIRE(!newAddress.empty());
		REQUIRE(subWallet->GetBalanceWithAddress(newAddress) == 0);
	}
}