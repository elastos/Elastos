// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Log.h>
#include <SDK/Implement/MainchainSubWallet.h>
#include <SDK/Implement/IDChainSubWallet.h>
#include <SDK/Implement/MasterWallet.h>
#include <SDK/WalletCore/Crypto/AES.h>
#include <SDK/Wallet/UTXO.h>

#include <Interface/MasterWalletManager.h>

#include <climits>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

#include "TestHelper.h"

using namespace Elastos::ElaWallet;

class TestMasterWalletManager : public MasterWalletManager {
public:
	TestMasterWalletManager() :
			MasterWalletManager(MasterWalletMap(), "Data", "Data") {
		_p2pEnable = false;
	}

	TestMasterWalletManager(const std::string &rootPath) :
			MasterWalletManager(rootPath) {
		_p2pEnable = false;
	}
};


TEST_CASE("Master wallet manager CreateMasterWallet test", "[CreateMasterWallet]") {
	Log::registerMultiLogger();

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "MasterWalletId";
	bool singleAddress = false;

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	SECTION("Normal creation") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
					payPassword, singleAddress);
		MasterWallet *masterWallet1 = dynamic_cast<MasterWallet *>(masterWallet);
		REQUIRE(masterWallet1 != nullptr);

		CHECK_NOTHROW(masterWallet->GetPublicKey());

		masterWalletManager->DestroyWallet(masterWallet->GetID());
	}
	SECTION("Master id should not be empty") {
		REQUIRE_THROWS_AS(masterWalletManager->CreateMasterWallet("", mnemonic, phrasePassword, payPassword, singleAddress),
						  std::invalid_argument);
	}
	SECTION("Mnemonic should not be empty") {
		CHECK_THROWS_AS(
				masterWalletManager->CreateMasterWallet(masterWalletId, "", phrasePassword, payPassword, singleAddress),
				std::invalid_argument);
	}
	SECTION("Create with phrase password can be empty") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, "",
																			  payPassword, singleAddress);
		masterWalletManager->DestroyWallet(masterWallet->GetID());
	}
	SECTION("Create with phrase password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, "ilegal", payPassword, singleAddress),
						std::invalid_argument);
	}
	SECTION("Create with phrase password that is more than 128") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic,
																"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
																payPassword, singleAddress), std::invalid_argument);
	}
	SECTION("Create with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword, "", singleAddress),
						std::invalid_argument);
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword, "ilegal", singleAddress),
						std::invalid_argument);
	}
	SECTION("Create with pay password that is more than 128") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
																"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", singleAddress),
						std::invalid_argument);
	}
}

TEST_CASE(
		"Wallet factory ExportWalletWithMnemonic generate  mnemonic",
		"[MasterWalletManager]") {
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	SECTION("generate english mnemonic") {
		std::string mnemonic = MasterWallet::GenerateMnemonic("english", "Data");
		IMasterWallet *masterWallet(
				masterWalletManager->CreateMasterWallet("MasterWalletId", mnemonic, phrasePassword, payPassword,
														"english"));

		std::string mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(mnemonic == mnemonic2);
	}
}

TEST_CASE("Wallet factory basic", "[MasterWalletManager]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	bool singleAddress = true;

	SECTION("Create master wallet") {
		std::string mnemonic = MasterWallet::GenerateMnemonic("english", "Data");
		IMasterWallet *masterWallet(masterWalletManager->CreateMasterWallet(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress));
		REQUIRE(masterWallet != nullptr);

		REQUIRE_FALSE(masterWallet->GetPublicKey().empty());

		masterWalletManager->DestroyWallet(masterWalletId);
	}

	SECTION("Verify multi sign public key") {
		std::string mnemonic = "令 厘 选 健 爱 轨 烯 距 握 译 控 礼";
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId + "1", mnemonic, "", payPassword, false);

		REQUIRE(masterWallet != nullptr);

		REQUIRE("023deb010c9318a46175e79d7b6c385f6c3ca525b7ba6a277b1d69dbead6a09664" == masterWalletManager->GetMultiSignPubKey(mnemonic, ""));

		REQUIRE(masterWallet->GetPublicKey() == "023deb010c9318a46175e79d7b6c385f6c3ca525b7ba6a277b1d69dbead6a09664");

		masterWalletManager->DestroyWallet(masterWalletId + "1");
	}
}

TEST_CASE("GetAllMasterWallets", "[MasterWalletManager]") {
	Log::registerMultiLogger();
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	std::string mnemonic = "";
	std::string mnemonic2 = "";
	bool singleAddress = false;
	//boost::scoped_ptr<IMasterWallet> masterWallet;
	IMasterWallet *masterWallet = nullptr;
	IMasterWallet *masterWallet2 = nullptr;
	SECTION("GetAllMasterWallets only one master wallet") {
		mnemonic = MasterWallet::GenerateMnemonic("english", "Data");
		masterWallet = masterWalletManager->CreateMasterWallet(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::vector<IMasterWallet *> loMasterWalletVec;
		loMasterWalletVec = masterWalletManager->GetAllMasterWallets();

		REQUIRE(loMasterWalletVec.size() == 1);

		REQUIRE(masterWallet->GetAllSubWallets().size() == 0);

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("GetAllMasterWallets two master wallet") {
		mnemonic = MasterWallet::GenerateMnemonic("english", "Data");
		masterWallet = masterWalletManager->CreateMasterWallet(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);

		REQUIRE(mnemonic.length() > 0);

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string masterWalletId2 = "masterWalletId2";
		mnemonic2 = MasterWallet::GenerateMnemonic("english", "Data");
		masterWallet2 = masterWalletManager->CreateMasterWallet(
			masterWalletId2, mnemonic2, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(mnemonic2.length() > 0);

		REQUIRE(!masterWallet2->GetPublicKey().empty());
		REQUIRE_FALSE(masterWallet2 == masterWallet);

		std::vector<IMasterWallet *> loMasterWalletVec;
		loMasterWalletVec = masterWalletManager->GetAllMasterWallets();

		REQUIRE(loMasterWalletVec.size() == 2);


		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId2));
	}

	SECTION("GetAllMasterWallets no one master wallet") {
		std::vector<IMasterWallet *> loMasterWalletVec;
		loMasterWalletVec = masterWalletManager->GetAllMasterWallets();

		REQUIRE(loMasterWalletVec.empty());

	}
}

TEST_CASE("test p2p net stop error use", "[MasterWalletManager]") {
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "drink false ribbon equal reward happy olive later silly track business sail";
	std::string masterWalletId = "masterWalletId";
	bool singleAddress = false;
	uint64_t feePerKB = 10000;

	SECTION("Create master wallet mnemonic english") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
				phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", feePerKB);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
		sleep(1);
		masterWallet->DestroyWallet(subWallet);

		masterWalletManager->DestroyWallet(masterWallet->GetID());
	}
}

TEST_CASE("WalletFactoryInner::importWalletInternal Test ", "[WalletFactoryInner]") {

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
	bool singleAddress = false;
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	SECTION("function para valid languae is valid ImportWalletWithMnemonic correct") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			"MasterWalletId", mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_FALSE(masterWallet->GetPublicKey().empty());
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet("MasterWalletId"));
	}
	SECTION("function para invalid  MasterWalletId empty str ") {
		CHECK_THROWS_AS(
				masterWalletManager->ImportWalletWithMnemonic("", mnemonic, phrasePassword, payPassword, singleAddress),
				std::invalid_argument);
	}
}


TEST_CASE("MasterWalletManager create destroy wallet", "[MasterWalletManager]") {
	Log::registerMultiLogger();
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "MasterWalletId";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	std::string mnemonic2 = "";
	bool singleAddress = false;

	SECTION("CreateMasterWallet MasterWalletId empty str") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet("", mnemonic, phrasePassword, payPassword, singleAddress),
						std::invalid_argument);
	}
	SECTION("CreateMasterWallet twice destroy wallet") {
		mnemonic = MasterWallet::GenerateMnemonic("french", "Data");
		REQUIRE(mnemonic.length() > 0);
		IMasterWallet *masterWallet1 = masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
																			   payPassword, "french");

		REQUIRE(masterWallet1 != nullptr);
		REQUIRE(!masterWallet1->GetPublicKey().empty());

		IMasterWallet *masterWallet2 = masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
																			   payPassword, "french");
		REQUIRE(masterWallet1 == masterWallet2);

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));

	}
	SECTION("CreateMasterWallet twice destroy twice wallet") {
		boost::scoped_ptr<TestMasterWalletManager> masterWalletManager1(new TestMasterWalletManager());
		boost::scoped_ptr<TestMasterWalletManager> masterWalletManager2(new TestMasterWalletManager());

		mnemonic = MasterWallet::GenerateMnemonic("french", "Data");
		REQUIRE(mnemonic.length() > 0);
		IMasterWallet *masterWallet1 = masterWalletManager1->CreateMasterWallet(masterWalletId, mnemonic,
																				phrasePassword, payPassword, "french");

		REQUIRE(masterWallet1 != nullptr);
		REQUIRE(!masterWallet1->GetPublicKey().empty());

		mnemonic2 = MasterWallet::GenerateMnemonic("french", "Data");
		REQUIRE(mnemonic2.length() > 0);
		IMasterWallet *masterWallet2 = masterWalletManager2->CreateMasterWallet(masterWalletId, mnemonic2,
																				phrasePassword, payPassword, "french");

		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		//REQUIRE(masterWallet1 == masterWallet2);
		REQUIRE_NOTHROW(masterWalletManager1->DestroyWallet(masterWalletId));
		//below    nothing will happen
		REQUIRE_NOTHROW(masterWalletManager1->DestroyWallet(masterWalletId));
		//this will destroy masterWallet2
		REQUIRE_NOTHROW(masterWalletManager2->DestroyWallet(masterWalletId));
		//REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWallet2));
	}
}


TEST_CASE("Wallet factory export & import  WithKeystore mnemonic", "[MasterWalletManager]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	bool singleAddress = false;
	uint64_t feePerKB = 10000;

	SECTION("export & import  WithKeystore mnemonic spanish ") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", feePerKB);


		std::string backupPassword = "backupPassword";
		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
																					   payPassword);

		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));

	}
}


TEST_CASE("Wallet factory Import Export  WalletWithMnemonic mnemonic ", "[MasterWalletManager]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string masterWalletId = "masterWalletId";
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	bool singleAddress = false;
	uint64_t feePerKB = 10000;

	SECTION("Import Export  WalletWithMnemonic mnemonic english ") {
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", feePerKB);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		masterWalletManager->DestroyWallet(masterWalletId);
	}
}


TEST_CASE("Wallet ImportWalletWithKeystore method", "[ImportWalletWithKeystore]") {


	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string backupPassword = "backupPassword";
	std::string masterWalletId = "masterWalletId";
	bool singleAddress = false;
	uint64_t feePerKB = 10000;

	SECTION("ImportWalletWithKeystore MasterWalletId twice str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		REQUIRE(nullptr != masterWallet->CreateSubWallet("ELA", feePerKB));

		std::string backupPassword = "backupPassword";
		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
																					   payPassword);
		keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword);

		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet == masterWallet2);

		IMasterWallet *masterWallet3 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet3 == masterWallet2);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("ImportWalletWithKeystore MasterWalletId empty str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			"MasterWalletId", mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";
		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
																					   payPassword);

		masterWalletId = "";
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent, backupPassword,
																	  payPassword), std::invalid_argument);
	}
	SECTION("ImportWalletWithKeystore invalid backupPassword ") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			"MasterWalletId", mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		REQUIRE(nullptr != masterWallet->CreateSubWallet("ELA", feePerKB));

		std::string backupPassword = "backupPassword";
		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
																					   payPassword);
		backupPassword = "";
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent, backupPassword,
																	  payPassword), std::invalid_argument);
	}
	SECTION("ImportWalletWithKeystore invalid payPassword ") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			"MasterWalletId", mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		REQUIRE(nullptr != masterWallet->CreateSubWallet("ELA", feePerKB));

		std::string backupPassword = "backupPassword";

		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
																					   payPassword);

		payPassword = "";
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent, backupPassword,
																	  payPassword), std::invalid_argument);
	}

	SECTION("ImportWalletWithKeystore success") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		REQUIRE(nullptr != masterWallet->CreateSubWallet("ELA", feePerKB));

		std::string backupPassword = "backupPassword";
		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
																					   payPassword);

		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet == masterWallet2);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Full version of export and import") {
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", feePerKB);
		REQUIRE(subWallet != nullptr);
		REQUIRE(dynamic_cast<MainchainSubWallet *>(subWallet) != nullptr);
		subWallet = masterWallet->CreateSubWallet("IDChain", feePerKB);
		REQUIRE(subWallet != nullptr);
		REQUIRE(dynamic_cast<IDChainSubWallet *>(subWallet) != nullptr);

		nlohmann::json keyStoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
																					   payPassword);
		masterWalletManager->DestroyWallet(masterWalletId);

		std::stringstream ss;
		ss << keyStoreContent;
		std::string test = ss.str();

		nlohmann::json resultKeyStoreContent;
		std::stringstream resultSs;
		resultSs << test;
		resultSs >> resultKeyStoreContent;

		masterWallet = masterWalletManager->ImportWalletWithKeystore(masterWalletId, resultKeyStoreContent,
																	 backupPassword,
																	 payPassword);

		std::vector<ISubWallet *> subwallets = masterWallet->GetAllSubWallets();
		REQUIRE(subwallets.size() == 2);
		REQUIRE(subwallets[0] != nullptr);
		REQUIRE(subwallets[1] != nullptr);
		for (int i = 0; i < 2; ++i) {
			if (subwallets[i]->GetChainID() == "ELA")
				REQUIRE(dynamic_cast<MainchainSubWallet *>(subwallets[i]) != nullptr);
			else if (subwallets[i]->GetChainID() == "IDChain")
				REQUIRE(dynamic_cast<IDChainSubWallet *>(subwallets[i]) != nullptr);
		}
	}
}


TEST_CASE("Wallet ExportWalletWithKeystore method", "[ExportWalletWithKeystore]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string backupPassword = "backupPassword";
	std::string masterWalletId = "masterWalletId";
	bool singleAddress = false;

	std::string mnemonic = "";

	SECTION("ExportWalletWithKeystore exportKeyStore fail  payPassword error ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";

		payPassword = "payPasswordChg";
		CHECK_THROWS_AS(
				masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword),
				std::logic_error);
	}
	SECTION("ExportWalletWithKeystore payPassword invalid str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";
		payPassword = "";

		REQUIRE_THROWS(masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword));
	}
	SECTION("ExportWalletWithKeystore backupPassword invalid str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "";

		CHECK_THROWS_AS(
				masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword),
				std::invalid_argument);
	}
	SECTION("Save") {
		mnemonic = MasterWallet::GenerateMnemonic("english", "Data");
		REQUIRE(mnemonic.length() > 0);
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
																			  payPassword, singleAddress);

		REQUIRE(masterWallet != nullptr);
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
}

TEST_CASE("Wallet ImportWalletWithMnemonic method", "[ImportWalletWithMnemonic]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	bool singleAddress = false;

	SECTION("ImportWalletWithMnemonic masterWalletId empty str ") {
		masterWalletId = "";
		REQUIRE_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress), std::invalid_argument);
	}
	SECTION("ImportWalletWithMnemonic masterWalletId twice ") {

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);


		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(masterWallet2 == masterWallet);

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Normal importing") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Normal importing with default parameters") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Import with phrase password can be empty") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, "", payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Import with phrase password that is less than 8") {
		REQUIRE_THROWS_AS(
				masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, "ilegal", payPassword, singleAddress),
				std::invalid_argument);
	}
	SECTION("Import with phrase password that is more than 128") {
		REQUIRE_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
			payPassword, singleAddress), std::invalid_argument);
	}
	SECTION("Import with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, "", singleAddress),
						std::invalid_argument);
		CHECK_THROWS_AS(
				masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, "ilegal", singleAddress),
				std::invalid_argument);
	}
	SECTION("Import with pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(
			masterWalletId, mnemonic, phrasePassword,
			"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
			singleAddress), std::invalid_argument);
	}
}

TEST_CASE("Wallet ExportWalletWithMnemonic method", "[ExportWalletWithMnemonic]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	bool singleAddress = false;
	IMasterWallet *masterWallet(
			masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress));

	SECTION("Normal exporting") {
		std::string actual = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(actual == mnemonic);
	}
	SECTION("Export with wrong password") {
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet, "wrongPassword"),
						std::logic_error);
	}

	SECTION("Export with null pointer of mater wallet") {
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(nullptr, "wrongPassword"),
						std::invalid_argument);
	}
	SECTION("Export with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet, ""), std::invalid_argument);
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet, "ilegal"),
						std::invalid_argument);
	}
	SECTION("Export with pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet,
																		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
}
TEST_CASE("Master wallet manager test", "[CreateMasterWallet]") {
	Log::registerMultiLogger();

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "MasterWalletId";
	bool singleAddress = false;

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	SECTION("Normal creation") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
		                                                                      payPassword, singleAddress);
		MasterWallet *masterWallet1 = dynamic_cast<MasterWallet *>(masterWallet);
		REQUIRE(masterWallet1 != nullptr);

		CHECK_NOTHROW(masterWallet->GetPublicKey());

		masterWalletManager->DestroyWallet(masterWallet->GetID());
	}
	SECTION("Master id should not be empty") {
		REQUIRE_THROWS_AS(masterWalletManager->CreateMasterWallet("", mnemonic, phrasePassword, payPassword, singleAddress),
		                  std::invalid_argument);
	}
	SECTION("Mnemonic should not be empty") {
		CHECK_THROWS_AS(
				masterWalletManager->CreateMasterWallet(masterWalletId, "", phrasePassword, payPassword, singleAddress),
				std::invalid_argument);
	}
	SECTION("Create with phrase password can be empty") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, "",
		                                                                      payPassword, singleAddress);
		masterWalletManager->DestroyWallet(masterWallet->GetID());
	}
	SECTION("Create with phrase password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, "ilegal", payPassword, singleAddress),
		                std::invalid_argument);
	}
	SECTION("Create with phrase password that is more than 128") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic,
		                                                        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		                                                        payPassword, singleAddress), std::invalid_argument);
	}
	SECTION("Create with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword, "", singleAddress),
		                std::invalid_argument);
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword, "ilegal", singleAddress),
		                std::invalid_argument);
	}
	SECTION("Create with pay password that is more than 128") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
		                                                        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", singleAddress),
		                std::invalid_argument);
	}
}

TEST_CASE("Wallet factory basic test", "[MasterWalletManager]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	bool singleAddress = true;

	SECTION("Create master wallet") {
		std::string mnemonic = MasterWallet::GenerateMnemonic("english", "Data");
		IMasterWallet *masterWallet(masterWalletManager->CreateMasterWallet(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress));
		REQUIRE(masterWallet != nullptr);

		REQUIRE_FALSE(masterWallet->GetPublicKey().empty());

		masterWalletManager->DestroyWallet(masterWalletId);
	}

	SECTION("Verify multi sign public key") {
		std::string mnemonic = "令 厘 选 健 爱 轨 烯 距 握 译 控 礼";
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId + "1", mnemonic, "", payPassword, false);

		REQUIRE(masterWallet != nullptr);

		REQUIRE("023deb010c9318a46175e79d7b6c385f6c3ca525b7ba6a277b1d69dbead6a09664" == masterWalletManager->GetMultiSignPubKey(mnemonic, ""));

		REQUIRE(masterWallet->GetPublicKey() == "023deb010c9318a46175e79d7b6c385f6c3ca525b7ba6a277b1d69dbead6a09664");

		masterWalletManager->DestroyWallet(masterWalletId + "1");
	}
}

TEST_CASE("GetAllMasterWallets test", "[MasterWalletManager]") {
	Log::registerMultiLogger();
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	std::string mnemonic = "";
	std::string mnemonic2 = "";
	bool singleAddress = false;
	//boost::scoped_ptr<IMasterWallet> masterWallet;
	IMasterWallet *masterWallet = nullptr;
	IMasterWallet *masterWallet2 = nullptr;
	SECTION("GetAllMasterWallets only one master wallet") {
		mnemonic = MasterWallet::GenerateMnemonic("english", "Data");
		masterWallet = masterWalletManager->CreateMasterWallet(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::vector<IMasterWallet *> loMasterWalletVec;
		loMasterWalletVec = masterWalletManager->GetAllMasterWallets();

		REQUIRE(loMasterWalletVec.size() == 1);

		REQUIRE(masterWallet->GetAllSubWallets().size() == 0);

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("GetAllMasterWallets two master wallet") {
		mnemonic = MasterWallet::GenerateMnemonic("english", "Data");
		masterWallet = masterWalletManager->CreateMasterWallet(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);

		REQUIRE(mnemonic.length() > 0);

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string masterWalletId2 = "masterWalletId2";
		mnemonic2 = MasterWallet::GenerateMnemonic("english", "Data");
		masterWallet2 = masterWalletManager->CreateMasterWallet(
				masterWalletId2, mnemonic2, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(mnemonic2.length() > 0);

		REQUIRE(!masterWallet2->GetPublicKey().empty());
		REQUIRE_FALSE(masterWallet2 == masterWallet);

		std::vector<IMasterWallet *> loMasterWalletVec;
		loMasterWalletVec = masterWalletManager->GetAllMasterWallets();

		REQUIRE(loMasterWalletVec.size() == 2);


		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId2));
	}

	SECTION("GetAllMasterWallets no one master wallet") {
		std::vector<IMasterWallet *> loMasterWalletVec;
		loMasterWalletVec = masterWalletManager->GetAllMasterWallets();

		REQUIRE(loMasterWalletVec.empty());

	}
}

TEST_CASE("p2p net stop error use test", "[MasterWalletManager]") {
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "drink false ribbon equal reward happy olive later silly track business sail";
	std::string masterWalletId = "masterWalletId";
	bool singleAddress = false;
	uint64_t feePerKB = 10000;

	SECTION("Create master wallet mnemonic english") {
		int i = 1;
		i++;

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
		                                                                            phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", feePerKB);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
		sleep(1);
		masterWallet->DestroyWallet(subWallet);

		masterWalletManager->DestroyWallet(masterWallet->GetID());
	}
}

//////////////////////////////////////////////

TEST_CASE("Test WalletFactoryInner::importWalletInternal", "[WalletFactoryInner]") {

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
	bool singleAddress = false;
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	SECTION("function para valid languae is valid ImportWalletWithMnemonic correct") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				"MasterWalletId", mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_FALSE(masterWallet->GetPublicKey().empty());
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet("MasterWalletId"));
	}
	SECTION("function para invalid  MasterWalletId empty str ") {
		CHECK_THROWS_AS(
				masterWalletManager->ImportWalletWithMnemonic("", mnemonic, phrasePassword, payPassword, singleAddress),
				std::invalid_argument);
	}
}


TEST_CASE("MasterWalletManager create destroy wallet test", "[MasterWalletManager]") {
	Log::registerMultiLogger();
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "MasterWalletId";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	std::string mnemonic2 = "";
	bool singleAddress = false;

	SECTION("CreateMasterWallet MasterWalletId empty str") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet("", mnemonic, phrasePassword, payPassword, singleAddress),
		                std::invalid_argument);
	}
	SECTION("CreateMasterWallet twice destroy wallet") {
		mnemonic = MasterWallet::GenerateMnemonic("french", "Data");
		REQUIRE(mnemonic.length() > 0);
		IMasterWallet *masterWallet1 = masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
		                                                                       payPassword, "french");

		REQUIRE(masterWallet1 != nullptr);
		REQUIRE(!masterWallet1->GetPublicKey().empty());

		IMasterWallet *masterWallet2 = masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
		                                                                       payPassword, "french");
		REQUIRE(masterWallet1 == masterWallet2);

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));

	}
	SECTION("CreateMasterWallet twice destroy twice wallet") {
		boost::scoped_ptr<TestMasterWalletManager> masterWalletManager1(new TestMasterWalletManager());
		boost::scoped_ptr<TestMasterWalletManager> masterWalletManager2(new TestMasterWalletManager());

		mnemonic = MasterWallet::GenerateMnemonic("french", "Data");
		REQUIRE(mnemonic.length() > 0);
		IMasterWallet *masterWallet1 = masterWalletManager1->CreateMasterWallet(masterWalletId, mnemonic,
		                                                                        phrasePassword, payPassword, "french");

		REQUIRE(masterWallet1 != nullptr);
		REQUIRE(!masterWallet1->GetPublicKey().empty());

		mnemonic2 = MasterWallet::GenerateMnemonic("french", "Data");
		REQUIRE(mnemonic2.length() > 0);
		IMasterWallet *masterWallet2 = masterWalletManager2->CreateMasterWallet(masterWalletId, mnemonic2,
		                                                                        phrasePassword, payPassword, "french");

		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		//REQUIRE(masterWallet1 == masterWallet2);
		REQUIRE_NOTHROW(masterWalletManager1->DestroyWallet(masterWalletId));
		//below    nothing will happen
		REQUIRE_NOTHROW(masterWalletManager1->DestroyWallet(masterWalletId));
		//this will destroy masterWallet2
		REQUIRE_NOTHROW(masterWalletManager2->DestroyWallet(masterWalletId));
		//REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWallet2));
	}
}


TEST_CASE("Wallet factory export & import WithKeystore mnemonic", "[MasterWalletManager]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	bool singleAddress = false;
	uint64_t feePerKB = 10000;

	SECTION("export & import  WithKeystore mnemonic spanish ") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", feePerKB);


		std::string backupPassword = "backupPassword";
		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
		                                                                               payPassword);

		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent,
		                                                                             backupPassword, payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));

	}
}


TEST_CASE("Wallet factory Import Export WalletWithMnemonic mnemonic", "[MasterWalletManager]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string masterWalletId = "masterWalletId";
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	bool singleAddress = false;
	uint64_t feePerKB = 10000;

	SECTION("Import Export  WalletWithMnemonic mnemonic english ") {
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", feePerKB);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		masterWalletManager->DestroyWallet(masterWalletId);
	}
}


TEST_CASE("Wallet ImportWalletWithKeystore method test", "[ImportWalletWithKeystore]") {


	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string backupPassword = "backupPassword";
	std::string masterWalletId = "masterWalletId";
	bool singleAddress = false;
	uint64_t feePerKB = 10000;

	SECTION("ImportWalletWithKeystore MasterWalletId twice str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		REQUIRE(nullptr != masterWallet->CreateSubWallet("ELA", feePerKB));

		std::string backupPassword = "backupPassword";
		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
		                                                                               payPassword);
		keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword);

		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent,
		                                                                             backupPassword, payPassword);
		REQUIRE(masterWallet == masterWallet2);

		IMasterWallet *masterWallet3 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent,
		                                                                             backupPassword, payPassword);
		REQUIRE(masterWallet3 == masterWallet2);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}

	SECTION("ImportWalletWithKeystore MasterWalletId empty str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				"MasterWalletId", mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";
		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
		                                                                               payPassword);

		masterWalletId = "";
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent, backupPassword,
		                                                              payPassword), std::invalid_argument);
	}

	SECTION("ImportWalletWithKeystore invalid backupPassword ") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				"MasterWalletId", mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		REQUIRE(nullptr != masterWallet->CreateSubWallet("ELA", feePerKB));

		std::string backupPassword = "backupPassword";
		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
		                                                                               payPassword);
		backupPassword = "";
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent, backupPassword,
		                                                              payPassword), std::invalid_argument);
	}

	SECTION("ImportWalletWithKeystore invalid payPassword ") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				"MasterWalletId", mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		REQUIRE(nullptr != masterWallet->CreateSubWallet("ELA", feePerKB));

		std::string backupPassword = "backupPassword";

		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
		                                                                               payPassword);

		payPassword = "";
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent, backupPassword,
		                                                              payPassword), std::invalid_argument);
	}

	SECTION("ImportWalletWithKeystore success") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		REQUIRE(nullptr != masterWallet->CreateSubWallet("ELA", feePerKB));

		std::string backupPassword = "backupPassword";
		nlohmann::json keystoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
		                                                                               payPassword);

		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystoreContent,
		                                                                             backupPassword, payPassword);
		REQUIRE(masterWallet == masterWallet2);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}

	SECTION("Full version of export and import") {
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", feePerKB);
		REQUIRE(subWallet != nullptr);
		REQUIRE(dynamic_cast<MainchainSubWallet *>(subWallet) != nullptr);
		subWallet = masterWallet->CreateSubWallet("IDChain", feePerKB);
		REQUIRE(subWallet != nullptr);
		REQUIRE(dynamic_cast<IDChainSubWallet *>(subWallet) != nullptr);

		nlohmann::json keyStoreContent = masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword,
		                                                                               payPassword);
		masterWalletManager->DestroyWallet(masterWalletId);

		std::stringstream ss;
		ss << keyStoreContent;
		std::string test = ss.str();

		nlohmann::json resultKeyStoreContent;
		std::stringstream resultSs;
		resultSs << test;
		resultSs >> resultKeyStoreContent;

		masterWallet = masterWalletManager->ImportWalletWithKeystore(masterWalletId, resultKeyStoreContent,
		                                                             backupPassword,
		                                                             payPassword);

		std::vector<ISubWallet *> subwallets = masterWallet->GetAllSubWallets();
		REQUIRE(subwallets.size() == 2);
		REQUIRE(subwallets[0] != nullptr);
		REQUIRE(subwallets[1] != nullptr);
		for (int i = 0; i < 2; ++i) {
			if (subwallets[i]->GetChainID() == "ELA")
				REQUIRE(dynamic_cast<MainchainSubWallet *>(subwallets[i]) != nullptr);
			else if (subwallets[i]->GetChainID() == "IDChain")
				REQUIRE(dynamic_cast<IDChainSubWallet *>(subwallets[i]) != nullptr);
		}
	}
}


TEST_CASE("Wallet ExportWalletWithKeystore method test", "[ExportWalletWithKeystore]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string backupPassword = "backupPassword";
	std::string masterWalletId = "masterWalletId";
	bool singleAddress = false;

	std::string mnemonic = "";

	SECTION("ExportWalletWithKeystore exportKeyStore fail  payPassword error ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";

		payPassword = "payPasswordChg";
		CHECK_THROWS_AS(
				masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword),
				std::logic_error);
	}
	SECTION("ExportWalletWithKeystore payPassword invalid str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";
		payPassword = "";

		REQUIRE_THROWS(masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword));
	}
	SECTION("ExportWalletWithKeystore backupPassword invalid str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "";

		CHECK_THROWS_AS(
				masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword),
				std::invalid_argument);
	}
	SECTION("Save") {
		mnemonic = MasterWallet::GenerateMnemonic("english", "Data");
		REQUIRE(mnemonic.length() > 0);
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
		                                                                      payPassword, singleAddress);

		REQUIRE(masterWallet != nullptr);
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
}

TEST_CASE("Wallet ImportWalletWithMnemonic method test", "[ImportWalletWithMnemonic]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	bool singleAddress = false;

	SECTION("ImportWalletWithMnemonic masterWalletId empty str ") {
		masterWalletId = "";
		REQUIRE_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress), std::invalid_argument);
	}
	SECTION("ImportWalletWithMnemonic masterWalletId twice ") {

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);


		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(masterWallet2 == masterWallet);

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Normal importing") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Normal importing with default parameters") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Import with phrase password can be empty") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, "", payPassword, singleAddress);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Import with phrase password that is less than 8") {
		REQUIRE_THROWS_AS(
				masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, "ilegal", payPassword, singleAddress),
				std::invalid_argument);
	}
	SECTION("Import with phrase password that is more than 128") {
		REQUIRE_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
				payPassword, singleAddress), std::invalid_argument);
	}
	SECTION("Import with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, "", singleAddress),
		                std::invalid_argument);
		CHECK_THROWS_AS(
				masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, "ilegal", singleAddress),
				std::invalid_argument);
	}
	SECTION("Import with pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(
				masterWalletId, mnemonic, phrasePassword,
				"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
				singleAddress), std::invalid_argument);
	}
}

TEST_CASE("Wallet ExportWalletWithMnemonic method test", "[ExportWalletWithMnemonic]") {

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	bool singleAddress = false;
	IMasterWallet *masterWallet(
			masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress));

	SECTION("Normal exporting") {
		std::string actual = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(actual == mnemonic);
	}
	SECTION("Export with wrong password") {
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet, "wrongPassword"),
		                std::logic_error);
	}

	SECTION("Export with null pointer of mater wallet") {
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(nullptr, "wrongPassword"),
		                std::invalid_argument);
	}
	SECTION("Export with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet, ""), std::invalid_argument);
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet, "ilegal"),
		                std::invalid_argument);
	}
	SECTION("Export with pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet,
		                                                                "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
		                  std::invalid_argument);
	}
	SECTION("Clear wallet data") {
		masterWalletManager->DestroyWallet(masterWalletId);
		boost::filesystem::path masterWalletPath = "Data/" + masterWalletId + "/";
		REQUIRE(boost::filesystem::exists(masterWalletPath) == false);
	}
}

TEST_CASE("Wallet GetBalance test", "[GetBalance]") {
	Log::registerMultiLogger();

	std::string masterWalletId = "testWallet";
	std::string payPassword = "payPassword";
	std::string path = "Data/" + masterWalletId + "/";

	SECTION("prepare wallet data") {
		boost::filesystem::path masterWalletPath = path;
		if (boost::filesystem::exists(masterWalletPath)) {
			boost::filesystem::remove_all(masterWalletPath);
		}

		std::string keyPath = "44'/0'/0'/0/0";
		LocalStore ls(nlohmann::json::parse("{\"account\":0,\"coinInfo\":[{\"ChainID\":\"ELA\",\"EarliestPeerTime\":1513936800,\"FeePerKB\":10000,\"VisibleAssets\":[\"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0\"]}],\"derivationStrategy\":\"BIP44\",\"m\":1,\"mnemonic\":\"P0C/7w2/h13rLqA8qgI+BwwDZrcK9g8ixjdPFFIEC6+G62Qsm4WsmoNbxJE+shQ2jy7tTsPsDYLKCow9hsGrWchJuBV5ULwwcnRhYimP9TlAYA6uTdk3aQgolw==\",\"mnemonicHasPassphrase\":true,\"n\":1,\"ownerPubKey\":\"027c876ac77226d6f25d198983b1ae58baa39b136ff0e09386e064b40d646767a1\",\"passphrase\":\"\",\"publicKeyRing\":[{\"requestPubKey\":\"027d917aa4732ebffcb496a40cce2bf5b57237570c106b97b98fa5be433c6b743d\",\"xPubKey\":\"xpub6CWoR5hv1BestMgnyWLPR1RXdttXhFLK9vTjri9J79SgYdCnpjTCNF5JiwyZXsaW4pMonQ8gaHWv5xUi9DgBLMzdWE75EULLzU444PkpF7E\"}],\"readonly\":false,\"requestPrivKey\":\"HiFHrGoxzoY3yCbshyUtMtY1fIO2rFw5265BALZgv08Vuen9c1llzg==\",\"requestPubKey\":\"027d917aa4732ebffcb496a40cce2bf5b57237570c106b97b98fa5be433c6b743d\",\"singleAddress\":false,\"xPrivKey\":\"SK1ian/e9X2YQdBdioAjo/P11xkGlaXp9AFXcSIYUmPuQwz8aepAkk4hUG7KfqqEtzwb4kOTt0Tm+ZiYiRXtLmVkaVLMaQD1ab49rIHlRj9zw2fSft8=\",\"xPubKey\":\"xpub6CWoR5hv1BestMgnyWLPR1RXdttXhFLK9vTjri9J79SgYdCnpjTCNF5JiwyZXsaW4pMonQ8gaHWv5xUi9DgBLMzdWE75EULLzU444PkpF7E\"}"));
		ls.SaveTo(path);

		std::string iso = "TEST";
		DatabaseManager dm("Data/" + masterWalletId + "/ELA.db");

		std::string xprv = ls.GetxPrivKey();
		bytes_t bytes = AES::DecryptCCM(xprv, payPassword);
		Key key = HDKeychain(bytes).getChild(keyPath);

		int txCount = 20;
		std::vector<TransactionPtr> txlist;
		for (int i = 0; i < txCount; ++i) {
			TransactionPtr tx(new Transaction());
			tx->SetVersion(Transaction::TxVersion::Default);
			tx->SetLockTime(getRandUInt32());
			tx->SetBlockHeight(i + 1);
			tx->SetTimestamp(getRandUInt32());
			tx->SetTransactionType(Transaction::transferAsset);
			tx->SetPayloadVersion(getRandUInt8());
			tx->SetFee(getRandUInt64());

			for (size_t i = 0; i < 1; ++i) {
				InputPtr input(new TransactionInput());
				input->SetTxHash(getRanduint256());
				input->SetIndex(getRandUInt16());
				input->SetSequence(getRandUInt32());
				tx->AddInput(input);

				Address address(PrefixStandard, key.PubKey());
				ProgramPtr program(new Program(keyPath, address.RedeemScript(), bytes_t()));
				tx->AddUniqueProgram(program);
			}

			for (size_t i = 0; i < 20; ++i) {
				Address toAddress(PrefixStandard, key.PubKey());
				OutputPtr output(new TransactionOutput(10, toAddress));
				tx->AddOutput(output);
			}

			uint256 md = tx->GetShaData();
			const std::vector<ProgramPtr> &programs = tx->GetPrograms();
			for (size_t i = 0; i < programs.size(); ++i) {
				bytes_t parameter = key.Sign(md);
				ByteStream stream;
				stream.WriteVarBytes(parameter);
				programs[i]->SetParameter(stream.GetBytes());
			}

			ByteStream stream;
			tx->Serialize(stream);
			bytes_t data = stream.GetBytes();
			std::string txHash = tx->GetHash().GetHex();

			dm.PutTransaction(iso, tx);

			txlist.push_back(tx);
		}


		REQUIRE(dm.GetAllTransactions(iso).size() == txCount);

		//transfer to another address
		BigInt transferAmount(2005);
		BigInt totalInput(0);
		TransactionPtr tx(new Transaction());
		tx->SetVersion(Transaction::TxVersion::Default);
		tx->SetLockTime(getRandUInt32());
		tx->SetBlockHeight(getRandUInt32());
		tx->SetTimestamp(getRandUInt32());
		tx->SetTransactionType(Transaction::transferAsset);
		tx->SetPayloadVersion(getRandUInt8());
		tx->SetFee(getRandUInt64());

		for (size_t i = 0; i < txCount; ++i) {
			for (size_t j = 0; j < txlist[i]->GetOutputs().size(); ++j) {
				InputPtr input(new TransactionInput());
				input->SetTxHash(txlist[i]->GetHash());
				input->SetIndex(j);
				input->SetSequence(getRandUInt32());
				tx->AddInput(input);

				totalInput +=  txlist[i]->GetOutputs()[j]->Amount();

				Address address(PrefixStandard, key.PubKey());
				ProgramPtr program(new Program(keyPath, address.RedeemScript(), bytes_t()));
				tx->AddUniqueProgram(program);
				if (totalInput > transferAmount) {
					break;
				}
			}
			if (totalInput > transferAmount) {
				break;
			}
		}
		BigInt fee = totalInput - transferAmount;
		Address toAddress("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ");
		OutputPtr output(new TransactionOutput(transferAmount, toAddress));
		tx->AddOutput(output);
		if (fee > 0) {
			Address change(PrefixStandard, key.PubKey());
			OutputPtr output(new TransactionOutput(fee, toAddress));
			tx->AddOutput(output);
		}

		uint256 md = tx->GetShaData();
		const std::vector<ProgramPtr> &programs = tx->GetPrograms();
		for (size_t i = 0; i < programs.size(); ++i) {
			bytes_t parameter = key.Sign(md);
			ByteStream stream;
			stream.WriteVarBytes(parameter);
			programs[i]->SetParameter(stream.GetBytes());
		}

		ByteStream stream;
		tx->Serialize(stream);
		bytes_t data = stream.GetBytes();
		std::string txHash = tx->GetHash().GetHex();

		dm.PutTransaction(iso, tx);

		REQUIRE(dm.GetAllTransactions(iso).size() == txCount + 1);

		//put coinbase tx
		for (int i = 0; i < txCount; ++i) {
			OutputPtr output(new TransactionOutput(10, Address(PrefixStandard, key.PubKey())));
			UTXOPtr entity(new UTXO(uint256(getRandBytes(32)), getRandUInt16(), getRandUInt32(), getRandUInt32(), output));

			dm.PutCoinBase(entity);
		}
		REQUIRE(dm.GetAllCoinBase().size() == txCount);
	}

	SECTION("verify wallet balance") {
		TestMasterWalletManager manager("Data");
		std::vector<IMasterWallet *> masterWallets = manager.GetAllMasterWallets();
		REQUIRE(!masterWallets.empty());
		IMasterWallet *masterWallet = nullptr;
		for (size_t i = 0; i < masterWallets.size(); ++i) {
			if (masterWallets[i]->GetID() == masterWalletId) {
				masterWallet = masterWallets[i];
				break;
			}
		}
		REQUIRE(masterWallet != nullptr);
		std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
		REQUIRE(subWallets.size() == 1);
		ISubWallet *subWallet = subWallets[0];

		std::string balance = subWallet->GetBalance(BalanceType::Total);
		REQUIRE(balance == "1990");

		balance = subWallet->GetBalance(BalanceType::Default);
		REQUIRE(balance == "1990");

		balance = subWallet->GetBalance(BalanceType::Voted);
		REQUIRE(balance == "0");

		nlohmann::json balanceInfo = subWallet->GetBalanceInfo();
		REQUIRE(balanceInfo.size() == 1);
		balanceInfo = balanceInfo[0]["Summary"];
		balance = balanceInfo["LockedBalance"].get<std::string>();
		REQUIRE(balance == "200");

//		clear wallet data
		manager.DestroyWallet(masterWalletId);
		boost::filesystem::path masterWalletPath = path;
		REQUIRE(boost::filesystem::exists(masterWalletPath) == false);
	}

}

