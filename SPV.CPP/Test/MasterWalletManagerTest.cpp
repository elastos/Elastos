// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <Common/ErrorChecker.h>
#include <Common/Log.h>
#include <Implement/MainchainSubWallet.h>
#include <Implement/IDChainSubWallet.h>
#include <Implement/MasterWallet.h>
#include <Database/DatabaseManager.h>
#include <WalletCore/AES.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/Key.h>
#include <Wallet/UTXO.h>

#include <MasterWalletManager.h>

#include <climits>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>
#include <Plugin/Transaction/Payload/CoinBase.h>

#include "TestHelper.h"

using namespace Elastos::ElaWallet;

static const std::string __rootPath = "./";
static const std::string _netType = "MainNet";
static nlohmann::json _config = R"({
			"ELA": { },
			"IDChain": { },
			"ETHSC": { "ChainID": 20, "NetworkID": 20 },
			"ETHDID": { "ChainID": 20, "NetworkID": 20 },
			"ETHHECO": { "ChainID": 128, "NetworkID": 128 }
		})"_json;
static const std::string masterWalletId = "masterWalletId";
static const std::string masterWalletId2 = "masterWalletId2";
static const std::string phrasePassword = "phrasePassword";
static const std::string payPassword = "payPassword";

TEST_CASE("Master wallet manager CreateMasterWallet test", "[CreateMasterWallet]") {
	Log::registerMultiLogger();

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	bool singleAddress = false;

	boost::filesystem::remove_all(boost::filesystem::path(std::string(__rootPath) + masterWalletId));
	boost::filesystem::remove_all(boost::filesystem::path(std::string(__rootPath) + masterWalletId2));
	boost::filesystem::remove_all(boost::filesystem::path(std::string(__rootPath) + "MasterWalletId"));


	boost::scoped_ptr<MasterWalletManager> manager(new MasterWalletManager(__rootPath, _netType, _config));

	SECTION("Normal creation") {
		IMasterWallet *masterWallet = manager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
																  payPassword, singleAddress);
		MasterWallet *masterWallet1 = dynamic_cast<MasterWallet *>(masterWallet);
		REQUIRE(masterWallet1 != nullptr);

		manager->DestroyWallet(masterWalletId);
		REQUIRE(!boost::filesystem::exists(std::string(__rootPath) + masterWalletId));
	}

	SECTION("Create with phrase password can be empty") {
		IMasterWallet *masterWallet = manager->CreateMasterWallet(masterWalletId, mnemonic, "",
																  payPassword, singleAddress);
		manager->DestroyWallet(masterWallet->GetID());
		REQUIRE(!boost::filesystem::exists(std::string(__rootPath) + masterWalletId));
	}

	REQUIRE_THROWS(manager->CreateMasterWallet("", mnemonic, phrasePassword, payPassword, singleAddress));
	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, "", phrasePassword, payPassword, singleAddress));

	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, mnemonic, "ilegal", payPassword, singleAddress));

	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, mnemonic,
											   "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
											   payPassword, singleAddress));

	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword, "", singleAddress));
	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword, "ilegal", singleAddress));

	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
											   "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
											   singleAddress));
}


TEST_CASE("GetAllMasterWallets", "[MasterWalletManager]") {
	boost::scoped_ptr<MasterWalletManager> manager(new MasterWalletManager(__rootPath, _netType, _config));

	std::string mnemonic = "";
	std::string mnemonic2 = "";
	bool singleAddress = false;
	//boost::scoped_ptr<IMasterWallet> masterWallet;
	IMasterWallet *masterWallet = nullptr;
	IMasterWallet *masterWallet2 = nullptr;

	std::vector<IMasterWallet *> masterWallets;
	masterWallets = manager->GetAllMasterWallets();
	if (!masterWallets.empty()) {
		for (IMasterWallet *mw : masterWallets) {
			manager->DestroyWallet(mw->GetID());
		}
		masterWallets = manager->GetAllMasterWallets();
	}
	REQUIRE(masterWallets.empty());

	mnemonic = MasterWallet::GenerateMnemonic("english", __rootPath);
	masterWallet = manager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
	REQUIRE(masterWallet != nullptr);

	masterWallets = manager->GetAllMasterWallets();
	REQUIRE(masterWallets.size() == 1);
	REQUIRE(masterWallets[0] == masterWallet);
	REQUIRE(masterWallet->GetAllSubWallets().size() == 0);

	mnemonic2 = MasterWallet::GenerateMnemonic("english", __rootPath);
	masterWallet2 = manager->CreateMasterWallet(masterWalletId2, mnemonic2, phrasePassword, payPassword, singleAddress);
	REQUIRE(masterWallet2 != nullptr);

	masterWallets = manager->GetAllMasterWallets();
	REQUIRE(masterWallets.size() == 2);

	REQUIRE_NOTHROW(manager->DestroyWallet(masterWalletId));
	REQUIRE_NOTHROW(manager->DestroyWallet(masterWalletId2));
	REQUIRE(!boost::filesystem::exists(std::string(__rootPath) + masterWalletId));
	REQUIRE(!boost::filesystem::exists(std::string(__rootPath) + masterWalletId2));
}

TEST_CASE("Wallet Import/Export method", "[Import]") {
	boost::scoped_ptr<MasterWalletManager> manager(new MasterWalletManager(__rootPath, _netType, _config));
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string backupPassword = "backupPassword";
	bool singleAddress = false;

	std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";

	IMasterWallet *masterWallet = manager->ImportWalletWithMnemonic(
		masterWalletId, mnemonic, "", payPassword, singleAddress);
	REQUIRE(masterWallet != nullptr);
	REQUIRE_NOTHROW(manager->DestroyWallet(masterWalletId));
	masterWallet = nullptr;

	// Import mnemonic
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
													 "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
													 singleAddress));
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, "il", singleAddress));
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, "", singleAddress));
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
													 "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
													 payPassword, singleAddress));
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic(masterWalletId, mnemonic, "il", payPassword, singleAddress));
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic("", mnemonic, phrasePassword, payPassword, singleAddress));
	masterWallet = manager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);

	REQUIRE(masterWallet != nullptr);
	REQUIRE(nullptr != masterWallet->CreateSubWallet("ELA"));

	// Export keystore
	REQUIRE_THROWS(masterWallet->ExportKeystore(backupPassword, "WrongPayPassword"));
	REQUIRE_THROWS(masterWallet->ExportKeystore(backupPassword, ""));
	REQUIRE_THROWS(masterWallet->ExportKeystore("", payPassword));
	nlohmann::json keystoreContent = masterWallet->ExportKeystore(backupPassword, payPassword);

	// Export mnemonic
	REQUIRE_THROWS(masterWallet->ExportMnemonic(""));
	REQUIRE_THROWS(masterWallet->ExportMnemonic("ilegal"));
	REQUIRE_THROWS(masterWallet->ExportMnemonic(
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"));
	REQUIRE(mnemonic == masterWallet->ExportMnemonic(payPassword));

	// Import keystore
	REQUIRE_THROWS(manager->ImportWalletWithKeystore(masterWalletId2, keystoreContent, "", payPassword));
	REQUIRE_THROWS(manager->ImportWalletWithKeystore(masterWalletId2, keystoreContent, backupPassword, ""));
	REQUIRE_THROWS(manager->ImportWalletWithKeystore(masterWalletId2, keystoreContent, backupPassword, payPassword));

	manager->DestroyWallet(masterWalletId);
	REQUIRE(!boost::filesystem::exists(std::string(__rootPath) + masterWalletId));
}


