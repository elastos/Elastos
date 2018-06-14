// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <climits>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>
#include <Interface/Enviroment.h>

#include "catch.hpp"

#include "MasterWalletManager.h"
#include "MasterWallet.h"

using namespace Elastos::SDK;

class TestMasterWalletManager : public MasterWalletManager {
public:
	TestMasterWalletManager() : MasterWalletManager(MasterWalletMap()) {
	}
};

class TestMasterWallet : public MasterWallet {
public:
	TestMasterWallet(const MasterWallet &wallet) : MasterWallet(wallet) {
	}

protected:
	virtual void startPeerManager(SubWallet *wallet) {
	}

	virtual void stopPeerManager(SubWallet *wallet) {
	}
};


TEST_CASE("Master wallet manager CreateMasterWallet test", "[CreateMasterWallet]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "MasterWalletId";

	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	SECTION("Normal creation") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId);
		MasterWallet *masterWallet1 = static_cast<MasterWallet *>(masterWallet);
		REQUIRE_FALSE(masterWallet1->Initialized());

		masterWalletManager->DestroyWallet(masterWallet->GetId());
	}
	SECTION("Master id should not be empty") {
		REQUIRE_THROWS_AS(masterWalletManager->CreateMasterWallet(""), std::invalid_argument);
	}
	SECTION("Language should not be null") {
		REQUIRE_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, ""), std::invalid_argument);
	}
}

TEST_CASE("Master wallet manager InitializeMasterWallet test", "[InitializeMasterWallet]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "MasterWalletId";

	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId);

	SECTION("Normal initialization") {
		CHECK_THROWS_AS(masterWallet->GetPublicKey(), std::logic_error);

		std::string mnemonic = masterWallet->GenerateMnemonic();
		CHECK(masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), mnemonic, phrasePassword,
														  payPassword));
		CHECK_NOTHROW(masterWallet->GetPublicKey());

		masterWalletManager->DestroyWallet(masterWallet->GetId());
	}
	SECTION("Should initialize with valid master wallet id") {
		std::string mnemonic = masterWallet->GenerateMnemonic();
		CHECK_THROWS_AS(
				masterWalletManager->InitializeMasterWallet("InvalidId", mnemonic, phrasePassword, payPassword),
				std::invalid_argument);
	}
	SECTION("Mnemonic should not be empty") {
		CHECK_THROWS_AS(
				masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), "", phrasePassword, payPassword),
				std::invalid_argument);
		masterWalletManager->DestroyWallet(masterWallet->GetId());
	}
	SECTION("Create with phrase password can be empty") {
		std::string mnemonic = masterWallet->GenerateMnemonic();
		CHECK(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, "",
														  payPassword));
		masterWalletManager->DestroyWallet(masterWallet->GetId());
	}
	SECTION("Create with phrase password that is empty or less than 8") {
		std::string mnemonic = masterWallet->GenerateMnemonic();
		CHECK_THROWS_AS(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, "ilegal", payPassword),
						std::invalid_argument);
		masterWalletManager->DestroyWallet(masterWallet->GetId());
	}
	SECTION("Create with phrase password that is more than 128") {
		std::string mnemonic = masterWallet->GenerateMnemonic();
		CHECK_THROWS_AS(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic,
																	"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
																	payPassword), std::invalid_argument);
		masterWalletManager->DestroyWallet(masterWallet->GetId());
	}
	SECTION("Create with pay password that is empty or less than 8") {
		std::string mnemonic = masterWallet->GenerateMnemonic();
		CHECK_THROWS_AS(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, ""),
						std::invalid_argument);
		CHECK_THROWS_AS(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, "ilegal"),
						std::invalid_argument);
		masterWalletManager->DestroyWallet(masterWallet->GetId());
	}
	SECTION("Create with pay password that is more than 128") {
		std::string mnemonic = masterWallet->GenerateMnemonic();
		CHECK_THROWS_AS(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword,
																	"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						std::invalid_argument);
		masterWalletManager->DestroyWallet(masterWallet->GetId());
	}
}


TEST_CASE(
		"Wallet factory ExportWalletWithMnemonic generate  mnemonic (english  chinese italian japanese spanish french)",
		"[MasterWalletManager]") {

	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";


	SECTION("generate french mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
				masterWalletManager->CreateMasterWallet("MasterWalletId", "french"));
		std::string mnemonic = masterWallet->GenerateMnemonic();
		masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), mnemonic, phrasePassword, payPassword);

		std::string mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(mnemonic == mnemonic2);
	}
	SECTION("generate spanish mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
				masterWalletManager->CreateMasterWallet("MasterWalletId", "spanish"));

		std::string mnemonic = masterWallet->GenerateMnemonic();
		masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), mnemonic, phrasePassword, payPassword);

		std::string mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(mnemonic == mnemonic2);
	}

	SECTION("generate japanese mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
				masterWalletManager->CreateMasterWallet("MasterWalletId", "japanese"));
		std::string mnemonic = masterWallet->GenerateMnemonic();
		masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), mnemonic, phrasePassword, payPassword);

		std::string mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(mnemonic == mnemonic2);
	}

	SECTION("generate italian mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
				masterWalletManager->CreateMasterWallet("MasterWalletId", "italian"));
		std::string mnemonic = masterWallet->GenerateMnemonic();
		masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), mnemonic, phrasePassword, payPassword);

		std::string mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(mnemonic == mnemonic2);
	}

	SECTION("generate chinese mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
				masterWalletManager->CreateMasterWallet("MasterWalletId", "chinese"));
		std::string mnemonic = masterWallet->GenerateMnemonic();
		masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), mnemonic, phrasePassword, payPassword);

		std::string mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(mnemonic == mnemonic2);
	}

	SECTION("generate english mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
				masterWalletManager->CreateMasterWallet("MasterWalletId", "english"));
		std::string mnemonic = masterWallet->GenerateMnemonic();
		masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), mnemonic, phrasePassword, payPassword);

		std::string mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(mnemonic == mnemonic2);
	}
}


TEST_CASE("Wallet factory basic", "[MasterWalletManager]") {
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";

	boost::scoped_ptr<IMasterWallet> masterWallet;
	SECTION("Create master wallet") {
		masterWallet.reset(masterWalletManager->CreateMasterWallet(masterWalletId));
		REQUIRE(masterWallet != nullptr);

		std::string mnemonic = masterWallet->GenerateMnemonic();
		masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), mnemonic, phrasePassword, payPassword);
		REQUIRE_FALSE(masterWallet->GetPublicKey().empty());
	}
}

TEST_CASE("InitializeMasterWallet", "[MasterWalletManager]") {
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";

	boost::scoped_ptr<IMasterWallet> masterWallet;
	SECTION("InitializeMasterWallet masterWalletId not CreateMasterWallet") {
//		masterWallet.reset(masterWalletManager->CreateMasterWallet(masterWalletId));
//		REQUIRE(masterWallet != nullptr);

		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";

		CHECK_THROWS_AS((masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword))
		, std::invalid_argument);

	}
	SECTION("InitializeMasterWallet twice") {
		masterWallet.reset(masterWalletManager->CreateMasterWallet(masterWalletId));
		REQUIRE(masterWallet != nullptr);

		std::string mnemonic = masterWallet->GenerateMnemonic();
		masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword);
		REQUIRE_FALSE(masterWallet->GetPublicKey().empty());

		REQUIRE_FALSE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));
	}
	SECTION("InitializeMasterWallet normal  success") {
		masterWallet.reset(masterWalletManager->CreateMasterWallet(masterWalletId));
		REQUIRE(masterWallet != nullptr);

		std::string mnemonic = masterWallet->GenerateMnemonic();
		masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), mnemonic, phrasePassword, payPassword);
		REQUIRE_FALSE(masterWallet->GetPublicKey().empty());
	}
	SECTION("InitializeMasterWallet masterWalletId empty str") {
		masterWallet.reset(masterWalletManager->CreateMasterWallet(masterWalletId));
		REQUIRE(masterWallet != nullptr);

		std::string mnemonic = masterWallet->GenerateMnemonic();

		masterWalletId = "";
		CHECK_THROWS_AS( masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword)
		, std::invalid_argument);
	}

}

TEST_CASE("GetAllMasterWallets", "[MasterWalletManager]") {
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	std::string mnemonic = "";
	std::string mnemonic2 = "";
	//boost::scoped_ptr<IMasterWallet> masterWallet;
	IMasterWallet*	masterWallet = nullptr;
	IMasterWallet*  masterWallet2 = nullptr;
	SECTION("GetAllMasterWallets only one master wallet") {
		masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, "english");
		mnemonic = masterWallet->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::vector<IMasterWallet *> loMasterWalletVec;
		loMasterWalletVec = masterWalletManager->GetAllMasterWallets();

		REQUIRE(loMasterWalletVec.size() == 1);

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("GetAllMasterWallets two master wallet") {
		masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, "english");

		mnemonic = masterWallet->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string masterWalletId2 = "masterWalletId2";
		masterWallet2 = masterWalletManager->CreateMasterWallet(masterWalletId2, "english");
		REQUIRE(masterWallet2 != nullptr);

		mnemonic2 = masterWallet->GenerateMnemonic();
		REQUIRE(mnemonic2.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId2, mnemonic2, phrasePassword, payPassword));

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



TEST_CASE("export & import password is character ,digit,special symbol,  chinese, french,  italian, japanese, spanish and mix(all of this)"
		  , "[MasterWalletManager]") {
	Enviroment::InitializeRootPath("Data");
	std::string masterWalletId = "masterWalletId";

	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";

	std::string mnemonic2 = "";


	SECTION("export & import password is character") {
		std::string phrasePassword = "phrasePassword";
		std::string payPassword = "payPassword";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																					phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(!mnemonic2.empty());
		REQUIRE(mnemonic2 == mnemonic);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("export & import password is special symbol") {
		std::string phrasePassword = "❤❥웃유♋☮㊣㊎";
		std::string payPassword = "❤❥웃유♋☮㊣㊎";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																					phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(!mnemonic2.empty());
		REQUIRE(mnemonic2 == mnemonic);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("export & import password is special symbol") {
		std::string phrasePassword = "❤❥웃유♋☮㊣㊎";
		std::string payPassword = "❤❥웃유♋☮㊣㊎";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																					phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(!mnemonic2.empty());
		REQUIRE(mnemonic2 == mnemonic);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("export & import password is chinese") {
		std::string phrasePassword = "测试中文为密码";
		std::string payPassword = "的情况怎么样";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																					phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(!mnemonic2.empty());
		REQUIRE(mnemonic2 == mnemonic);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("export & import password is french") {
		std::string phrasePassword = "abaisseraboyer";
		std::string payPassword = "aboutiraborder";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																					phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(!mnemonic2.empty());
		REQUIRE(mnemonic2 == mnemonic);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("export & import password is italian") {
		std::string phrasePassword = "abacoabbaglio";
		std::string payPassword = "accennoaccusato";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																					phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(!mnemonic2.empty());
		REQUIRE(mnemonic2 == mnemonic);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("export & import password is italian") {
		std::string phrasePassword = "あいこくしんあいさつあいた";
		std::string payPassword = "あきるあけがた";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																					phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(!mnemonic2.empty());
		REQUIRE(mnemonic2 == mnemonic);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("export & import password is spanish") {
		std::string phrasePassword = "ábacoabdomenabeja";
		std::string payPassword = "abogadoabonoabuelo";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																					phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(!mnemonic2.empty());
		REQUIRE(mnemonic2 == mnemonic);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("export & import password is mix all") {

		std::string phrasePassword = "1a웃유abdiquerabacoあいこくしんábaco";
		std::string payPassword = "1a웃유abdiquerabacoあいこくしんábacos";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																					phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		mnemonic2 = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		REQUIRE(!mnemonic2.empty());
		REQUIRE(mnemonic2 == mnemonic);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
}



//TEST_CASE("test p2p net stop error use", "[MasterWalletManager]") {
//	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
//
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//	std::string mnemonic = "drink false ribbon equal reward happy olive later silly track business sail";
//
//
//	SECTION("Create master wallet mnemonic english") {
//		int i = 1;
//		i++;
//
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"english");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		printf("before  CreateSubWallet -----> \n");
//		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//		sleep(1);
//		printf("before DestroyWallet subWallet -----> \n");
//		masterWallet->DestroyWallet(subWallet);
//
//		printf("before DestroyWallet masterWallet -----> \n");
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//}
//

//TEST_CASE("Test crash", "[MasterWalletManager]") {
//	Enviroment::InitializeRootPath("Data");
//	boost::scoped_ptr<TestMasterWalletManager> walletFact(new TestMasterWalletManager());
//	std::string phrasePassword = "phrasepassword";
//	std::string payPassword = "payPassword";
//	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
//	std::string masterWalletId = "masterWalletId";
//
//	SECTION("my test carsh") {
//		IMasterWallet *masterWallet = walletFact->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, payPassword,
//																		   "english");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//		sleep(1);
//		masterWallet->DestroyWallet(subWallet);
//		walletFact->DestroyWallet(masterWallet);
//	}
//
//}
//////////////////////////////////////////////

TEST_CASE("WalletFactoryInner::importWalletInternal Test ", "[WalletFactoryInner]") {
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());

	SECTION("function para valid languae is valid ImportWalletWithMnemonic correct") {
		std::string phrasePassword = "phrasePassword";
		std::string payPassword = "payPassword";
		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic("MasterWalletId", mnemonic, phrasePassword,
																					payPassword,
																					"chinese");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet("MasterWalletId"));
	}
	SECTION("function para invalid  MasterWalletId empty str "){
		std::string phrasePassword = "phrasePassword";
		std::string payPassword = "payPassword";
		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";

		CHECK_THROWS_AS( masterWalletManager->ImportWalletWithMnemonic("", mnemonic, phrasePassword, payPassword
			, "chinese"), std::invalid_argument);
	}
	SECTION("function para invalid languae not support germany.") {
		std::string phrasePassword = "phrasePassword";
		std::string payPassword = "payPassword";
		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";

		CHECK_THROWS_AS( masterWalletManager->ImportWalletWithMnemonic("MasterWalletId", mnemonic, phrasePassword, payPassword
			, "Germany"), std::invalid_argument);

	}
	SECTION("walletImportFun return false ") {
		std::string phrasePassword = "phrasePassword";
		std::string payPassword = "payPassword";
		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 漫";

		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic("MasterWalletId", mnemonic, phrasePassword,
																	  payPassword,
																	  "chinese"), std::invalid_argument);
	}


}


TEST_CASE("MasterWalletManager create destroy wallet", "[MasterWalletManager]") {
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "MasterWalletId";
	std::string mnemonic = "";
	std::string mnemonic2 = "";

	SECTION("CreateMasterWallet MasterWalletId empty str") {
		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet("", "french")
		, std::invalid_argument);
	}
	SECTION("CreateMasterWallet twice destroy wallet") {
		IMasterWallet *masterWallet1 = masterWalletManager->CreateMasterWallet(masterWalletId,  "french");

		mnemonic = masterWallet1->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));

		REQUIRE(masterWallet1 != nullptr);
		REQUIRE(!masterWallet1->GetPublicKey().empty());

		IMasterWallet *masterWallet2 = masterWalletManager->CreateMasterWallet(masterWalletId, "french");
		REQUIRE(masterWallet1 == masterWallet2);
		REQUIRE(mnemonic.length() > 0);
		REQUIRE_FALSE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));

	}
	SECTION("CreateMasterWallet twice destroy twice wallet") {
		Enviroment::InitializeRootPath("Data");
		boost::scoped_ptr<TestMasterWalletManager> masterWalletManager1(new TestMasterWalletManager());
		boost::scoped_ptr<TestMasterWalletManager> masterWalletManager2(new TestMasterWalletManager());

		IMasterWallet *masterWallet1 = masterWalletManager1->CreateMasterWallet(masterWalletId, "french");
		mnemonic = masterWallet1->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager1->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));

		REQUIRE(masterWallet1 != nullptr);
		REQUIRE(!masterWallet1->GetPublicKey().empty());

		IMasterWallet *masterWallet2 = masterWalletManager2->CreateMasterWallet(masterWalletId, "french");
		mnemonic2 = masterWallet2->GenerateMnemonic();
		REQUIRE(mnemonic2.length() > 0);
		REQUIRE(masterWalletManager2->InitializeMasterWallet(masterWalletId, mnemonic2, phrasePassword, payPassword));

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
	SECTION("create destroy french wallet") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, "french");

		mnemonic = masterWallet->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("create destroy spanish wallet") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, "spanish");
		mnemonic = masterWallet->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("create destroy japanese wallet") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, "japanese");
		mnemonic = masterWallet->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("create destroy italian wallet") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId,  "italian");

		mnemonic = masterWallet->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("create destroy chinese wallet") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, "chinese");

		mnemonic = masterWallet->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("create destroy english wallet") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, "english");
		mnemonic = masterWallet->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
}



TEST_CASE("Wallet factory export & import  WithKeystore mnemonic", "[MasterWalletManager]") {
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";


	SECTION("export & import  WithKeystore mnemonic spanish ") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);


		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testSpanishMnemonic.json";
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));

		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));

	}
	SECTION("export & import  WithKeystore mnemonic chinese ") {
		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"chinese");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
		std::string backupPassword = "backupPassword";

		std::string keystorePath = "testchineseMnemonic.json";
		std::string payPassword = "payPassword";
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));

		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("export & import  WithKeystore mnemonic french ") {
		std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"french");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);


		std::string backupPassword = "backupPassword";
		std::string payPassword = "payPassword";
		std::string keystorePath = "testfrenchMnemonic.json";
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));
		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("export & import  WithKeystore mnemonic japanese ") {
		std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"japanese");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
		std::string backupPassword = "backupPassword";
		std::string payPassword = "payPassword";
		std::string keystorePath = "testjapaneseMnemonic.json";
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));


		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("export & import  WithKeystore mnemonic italian ") {
		std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"italian");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);


		std::string backupPassword = "backupPassword";
		std::string payPassword = "payPassword";
		std::string keystorePath = "testitalianMnemonic.json";
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));


		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("export & import  WithKeystore mnemonic english ") {
		std::string mnemonic = "drink false ribbon equal reward happy olive later silly track business sail";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
		std::string backupPassword = "backupPassword";
		std::string payPassword = "payPassword";
		std::string keystorePath = "testenglishMnemonic.json";
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));
		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
}


TEST_CASE("Wallet factory Import Export  WalletWithMnemonic mnemonic ", "[MasterWalletManager]") {
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string masterWalletId = "masterWalletId";
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	SECTION("Import Export  WalletWithMnemonic mnemonic english ") {
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId,  mnemonic, phrasePassword,
																					payPassword,
																					"english");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		masterWalletManager->DestroyWallet(masterWalletId);
	}

	SECTION("Import Export  WalletWithMnemonic mnemonic chinese ") {

		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"chinese");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("Import Export  WalletWithMnemonic mnemonic french ") {

		std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"french");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("Import Export  WalletWithMnemonic mnemonic spanish ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"spanish");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("Import Export  WalletWithMnemonic mnemonic japanese ") {

		std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"japanese");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
	SECTION("Import Export  WalletWithMnemonic mnemonic italian ") {

		std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"italian");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		masterWalletManager->DestroyWallet(masterWalletId);
	}
}



TEST_CASE("Wallet ImportWalletWithKeystore method", "[ImportWalletWithKeystore]") {

	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string backupPassword = "backupPassword";
	std::string masterWalletId = "masterWalletId";

	SECTION("ImportWalletWithKeystore MasterWalletId twice str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testSpanishMnemonic.json";
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword
			, payPassword, keystorePath);

		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet == masterWallet2);

		IMasterWallet *masterWallet3 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet3 == masterWallet2);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("ImportWalletWithKeystore MasterWalletId empty str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic("MasterWalletId", mnemonic, phrasePassword,
																					payPassword,
																					"spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testSpanishMnemonic.json";
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));


		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword
			, payPassword, keystorePath);

		masterWalletId = "";
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath
			, backupPassword, payPassword),  std::invalid_argument);
	}
	SECTION("ImportWalletWithKeystore invalid backupPassword ") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic("MasterWalletId", mnemonic, phrasePassword,
																					payPassword,
																					"spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testSpanishMnemonic.json";

		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword
			, payPassword, keystorePath);
		backupPassword = "";
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath
			, backupPassword, payPassword),  std::invalid_argument);
	}
	SECTION("ImportWalletWithKeystore invalid payPassword ") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic("MasterWalletId", mnemonic, phrasePassword,
																					payPassword,
																					"spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";

		std::string keystorePath = "testSpanishMnemonic.json";

		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword
			, payPassword, keystorePath);

		payPassword = "";
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath
			, backupPassword, payPassword),  std::invalid_argument);
	}

	SECTION("ImportWalletWithKeystore success") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testSpanishMnemonic.json";
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));

//		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword
//			, payPassword, keystorePath);

		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
																					 backupPassword, payPassword);
		REQUIRE(masterWallet == masterWallet2);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
}


TEST_CASE("Wallet ExportWalletWithKeystore method", "[ExportWalletWithKeystore]") {
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string backupPassword = "backupPassword";
	std::string masterWalletId = "masterWalletId";

	std::string mnemonic = "";

	SECTION("ExportWalletWithKeystore exportKeyStore fail  payPassword error ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";

		std::string keystorePath = "testSpanishMnemonic.json";
		payPassword = "payPasswordChg";
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath)
		,  std::logic_error);
	}
	SECTION("ExportWalletWithKeystore payPassword invalid str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "backupPassword";
		payPassword = "";
		std::string keystorePath = "testSpanishMnemonic.json";

		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath)
		,  std::invalid_argument);
	}
	SECTION("ExportWalletWithKeystore backupPassword invalid str ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		std::string backupPassword = "";
		std::string keystorePath = "testSpanishMnemonic.json";

		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath)
		,  std::invalid_argument);
	}
	SECTION("Save") {
		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId);

		mnemonic = masterWallet->GenerateMnemonic();
		REQUIRE(mnemonic.length() > 0);
		REQUIRE(masterWalletManager->InitializeMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword));


		REQUIRE(masterWallet != nullptr);
		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, "wallet.store");
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
}

TEST_CASE("Wallet ImportWalletWithMnemonic method", "[ImportWalletWithMnemonic]") {
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";

	SECTION("ImportWalletWithMnemonic masterWalletId empty str ") {
		masterWalletId = "";
		REQUIRE_THROWS_AS( masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																		 payPassword,
																		 "english"), std::invalid_argument);
	}
	SECTION("ImportWalletWithMnemonic masterWalletId twice ") {

		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);


		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(masterWallet2 == masterWallet);

		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Normal importing") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																					payPassword,
																					"english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Normal importing with default parameters") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																					phrasePassword, payPassword);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Import with phrase password can be empty") {
		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, "",
																					payPassword);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWalletId));
	}
	SECTION("Import with phrase password that is less than 8") {
		REQUIRE_THROWS_AS(
				masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, "ilegal", payPassword),
				std::invalid_argument);
	}
	SECTION("Import with phrase password that is more than 128") {
		REQUIRE_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
																		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
																		payPassword), std::invalid_argument);
	}
	SECTION("Import with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, ""),
						std::invalid_argument);
		CHECK_THROWS_AS(
				masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, "ilegal"),
				std::invalid_argument);
	}
	SECTION("Import with pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
																		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
	SECTION("Language should not be null") {
		CHECK_THROWS_AS(
				masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, payPassword,
															  ""),
				std::invalid_argument);
	}
}

TEST_CASE("Wallet ExportWalletWithMnemonic method", "[ExportWalletWithMnemonic]") {
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWalletManager> masterWalletManager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "masterWalletId";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	boost::scoped_ptr<IMasterWallet> masterWallet(
			masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, payPassword));

	SECTION("Normal exporting") {
		std::string actual = masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(actual == mnemonic);
	}
	SECTION("Export with wrong password") {
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), "wrongPassword"),
						std::logic_error);
	}

	SECTION("Export with null pointer of mater wallet") {
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(nullptr, "wrongPassword"),
						std::invalid_argument);
	}
	SECTION("Export with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), ""), std::invalid_argument);
		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), "ilegal"),
						std::invalid_argument);
	}
	SECTION("Export with pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(),
																		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
}



