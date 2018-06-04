// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>

#include "catch.hpp"

#include "MasterWallet.h"

using namespace Elastos::SDK;

class TestMasterWallet : public MasterWallet {
public:
	TestMasterWallet(const std::string &phrasePassword,
					 const std::string &payPassword, const std::string language) :
			MasterWallet(phrasePassword, payPassword, language) {
	}

protected:
	virtual void startPeerManager(SubWallet *wallet) {
	}

	virtual void stopPeermanager(SubWallet *wallet) {
	}
};

TEST_CASE("Master wallet basic", "[MasterWallet]") {

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword, language));
	REQUIRE(masterWallet->Initialized());

	std::string chainId = "chainid";
	ISubWallet *subWallet;
	SECTION("Create sub wallet") {
		subWallet = masterWallet->CreateSubWallet(Normal, chainId, 0, payPassword, false);
		REQUIRE(subWallet != nullptr);

		//Return same sub wallet with same chain id
		ISubWallet *subWallet1 = masterWallet->CreateSubWallet(Normal, chainId, 0, payPassword, false);
		REQUIRE(subWallet == subWallet1);

		//Return same sub wallet even if parameter of others are different
		ISubWallet *subWallet2 = masterWallet->CreateSubWallet(Normal, chainId, 1, "other password", true);
		REQUIRE(subWallet == subWallet2);

		//Create another sub wallet
		ISubWallet *subWallet3 = masterWallet->CreateSubWallet(Normal, "chain2", 1, payPassword, false);
		REQUIRE(subWallet3 != nullptr);
		REQUIRE(subWallet != subWallet3);

		masterWallet->DestroyWallet(subWallet);
		masterWallet->DestroyWallet(subWallet3);
	}
}