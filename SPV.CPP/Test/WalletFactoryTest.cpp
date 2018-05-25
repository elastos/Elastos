// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

#include "catch.hpp"

#include "WalletFactory.h"

using namespace Elastos::SDK;

TEST_CASE("Wallet factory basic", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string testName = "testName";
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	boost::scoped_ptr<IMasterWallet> masterWallet;
	SECTION("Create master wallet") {
		masterWallet.reset(walletFactory->CreateMasterWallet(testName, phrasePassword, payPassword));
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
	}
	SECTION("Return exist master wallet of same name") {
		IMasterWallet *masterWallet2 = walletFactory->CreateMasterWallet(testName, phrasePassword, payPassword);
		REQUIRE(masterWallet == masterWallet2);
	}
	SECTION("Return exist master wallet even if give the wrong phrase password and pay password") {
		IMasterWallet *masterWallet2 = walletFactory->CreateMasterWallet(testName, "", "");
		REQUIRE(masterWallet == masterWallet2);
	}
}

TEST_CASE("Wallet factory mnemonic export & import", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string testName = "testName";
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
			walletFactory->CreateMasterWallet(testName, phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export") {
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(!mnemonic.empty());
	}
	SECTION("Mnemonic import") {
		boost::scoped_ptr<IMasterWallet> masterWallet2(walletFactory->ImportWalletWithMnemonic(
				mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
	}
	SECTION("Mnemonic import with invalid mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
				"invalid mnemonic", phrasePassword, payPassword));
		REQUIRE(masterWallet3 == nullptr);
	}
	SECTION("Mnemonic import with wrong mnemonic") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
				wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
	SECTION("Mnemonic import with wrong phrase password") {
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
				mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
	SECTION("Mnemonic import of different pay password") {
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
				mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory key store export & import", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string testName = "testName";
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
			walletFactory->CreateMasterWallet(testName, phrasePassword, payPassword));

	std::string backupPassword = "backupPassword";
	std::string keystorePath = "test.json";
	SECTION("Key store export") {
		walletFactory->ExportWalletWithKeystore(masterWallet.get(), backupPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));
	}

	SECTION("Key store import") {
		boost::scoped_ptr<IMasterWallet> masterWallet2(
				walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
	}

	//todo delete key store file
}