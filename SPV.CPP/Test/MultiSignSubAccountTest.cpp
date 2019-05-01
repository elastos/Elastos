// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include <SDK/Account/Account.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("MultiSign account test", "[Readonly account test]") {
	Log::registerMultiLogger();
	SECTION("Address generate test") {

		std::vector<std::string> coSigners;

		coSigners.push_back("032f4540e915134f38ba24cdc08621ad7f5b8b62db36843ae8fa9422c047a04be8");
		coSigners.push_back("02e63efab72413b320a341b054d5d6ce5f1565a3b466aa4925de69d152027cce20");
		coSigners.push_back("02f212915764714d1a7cc85100d924f4657e7797da269ea2eb4968146e57ca364c");
		coSigners.push_back("02f3346a807786c4d040c7f7df75b2f4b64cc6f9f95aaf0bed79099fb1c48fdb3f");

		LocalStorePtr localstore(new LocalStore("Data/1", coSigners, 3));
		AccountPtr account(new Account(localstore, "Data"));

		REQUIRE("8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3" == account->GetAddress().String());
	}
}
