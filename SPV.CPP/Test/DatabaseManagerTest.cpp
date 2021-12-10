// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Database/DatabaseManager.h>
#include <Common/Log.h>

#include <fstream>

using namespace Elastos::ElaWallet;

#define DBFILE "wallet.db"

TEST_CASE("DatabaseManager test", "[DatabaseManager]") {
	Log::registerMultiLogger();
	std::string pluginType = "ELA";
#define DEFAULT_RECORD_CNT 20

	srand(time(nullptr));

	SECTION("Used address Test") {
#define TEST_USED_ADDRESS_CNT DEFAULT_RECORD_CNT
		std::vector<std::string> usedAddress;

		// prepare data
		for (int i = 0; i < TEST_USED_ADDRESS_CNT; ++i) {
			std::string hash = getRanduint256().GetHex();
			usedAddress.push_back(hash);
		}

		// save
		DatabaseManager *dbm = new DatabaseManager(DBFILE);
		REQUIRE(dbm->DeleteAllUsedAddresses());
		REQUIRE(dbm->PutUsedAddresses(usedAddress, false));
		// test insert or replace
		REQUIRE(dbm->PutUsedAddresses(usedAddress, false));

		// read & verify
		std::vector<std::string> usedAddressVerify = dbm->GetUsedAddresses();
		REQUIRE(TEST_USED_ADDRESS_CNT == usedAddressVerify.size());
		for (int i = 0; i < TEST_USED_ADDRESS_CNT; ++i)
			REQUIRE(usedAddress[i] == usedAddressVerify[i]);

		// save and replace
		usedAddress.clear();

		// prepare data
		for (int i = 0; i < TEST_USED_ADDRESS_CNT; ++i) {
			std::string hash = getRanduint256().GetHex();
			usedAddress.push_back(hash);
		}

		// save & replace
		REQUIRE(dbm->PutUsedAddresses(usedAddress, true));

		// read & verify
		usedAddressVerify = dbm->GetUsedAddresses();
		REQUIRE(TEST_USED_ADDRESS_CNT == usedAddressVerify.size());
		for (int i = 0; i < TEST_USED_ADDRESS_CNT; ++i)
			REQUIRE(usedAddress[i] == usedAddressVerify[i]);

		// delete all
		REQUIRE(dbm->DeleteAllUsedAddresses());

		delete dbm;
	}

}

