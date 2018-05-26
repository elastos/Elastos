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

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	boost::scoped_ptr<IMasterWallet> masterWallet;
	SECTION("Create master wallet") {
		masterWallet.reset(walletFactory->CreateMasterWallet(phrasePassword, payPassword));
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
	}
}

TEST_CASE("Wallet factory mnemonic export & import", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
			walletFactory->CreateMasterWallet(phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export") {
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(walletFactory->ImportWalletWithMnemonic(
				mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
				mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(walletFactory->ImportWalletWithMnemonic(
				mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
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
}

TEST_CASE("Wallet factory key store export & import", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
			walletFactory->CreateMasterWallet(phrasePassword, payPassword));

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