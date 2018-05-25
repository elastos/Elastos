// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>

#include "catch.hpp"

#include "WalletFactory.h"

using namespace Elastos::SDK;

TEST_CASE( "Master wallet basic", "[WalletFactory]" )
{

}

TEST_CASE( "Master wallet key store export & import", "[WalletFactory]" )
{
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string testName = "testName";
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(testName, phrasePassword, payPassword);

	std::string backupPassword = "backupPassword";
	std::string keystorePath = "test.json";
	SECTION("Key store export") {
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
		//todo keystorePath should be exist
	}

	SECTION("Key store import") {
		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
		//todo check properties of masterWallet and masterWallet2
	}
	//todo delete key store file
}