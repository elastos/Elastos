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
	std::string masterWalletId = "MasterWalletId";

	boost::scoped_ptr<IMasterWallet> masterWallet;
	SECTION("Create master wallet") {
		masterWallet.reset(masterWalletManager->CreateMasterWallet(masterWalletId));
		REQUIRE(masterWallet != nullptr);

		std::string mnemonic = masterWallet->GenerateMnemonic();
		masterWalletManager->InitializeMasterWallet(masterWallet->GetId(), mnemonic, phrasePassword, payPassword);
		REQUIRE_FALSE(masterWallet->GetPublicKey().empty());
	}
}

/*
 //////////////////////////////// password is character ,digit,special symbol,  chinese, french,  italian, japanese, spanish and mix(all of this)
TEST_CASE("Wallet factory mnemonic export & import password is character", "[MasterWalletManager]") {

	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export") {
		mnemonic = MasterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
	}

	SECTION("Mnemonic import with wrong mnemonic") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}


TEST_CASE("Wallet factory mnemonic export & import password is digit", "[MasterWalletManager]") {

	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());

	std::string phrasePassword = "1234567890123";
	std::string payPassword = "22222222222";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is digit") {
		mnemonic = MasterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
	}

	SECTION("Mnemonic import with wrong mnemonic password is digit") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is special symbol", "[MasterWalletManager]") {

	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());

	std::string phrasePassword = "❤❥웃유♋☮㊣㊎";
	std::string payPassword = "❤❥웃유♋☮㊣㊎";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is special symbol") {
		mnemonic = MasterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
	}

	SECTION("Mnemonic import with wrong mnemonic password is special symbol") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is digit|character|special symbol", "[MasterWalletManager]") {

	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());

	std::string phrasePassword = "❤❥웃유123ab";
	std::string payPassword = "❤❥웃유456cd";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is digit|character|special symbol") {
		mnemonic = MasterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
	}

	SECTION("Mnemonic import with wrong mnemonic password is digit|character|special symbol") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is chinese", "[MasterWalletManager]") {

	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());

	std::string phrasePassword = "测试中文为密码";
	std::string payPassword = "的情况怎么样";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is chinese") {
		mnemonic = MasterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
	}

	SECTION("Mnemonic import with wrong mnemonic password is chinese") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}


TEST_CASE("Wallet factory mnemonic export & import password is french", "[MasterWalletManager]") {

	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());

	std::string phrasePassword = "abaisseraboyer";
	std::string payPassword = "aboutiraborder";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is french") {
		mnemonic = MasterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
	}

	SECTION("Mnemonic import with wrong mnemonic password is french") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is italian", "[MasterWalletManager]") {

	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());

	std::string phrasePassword = "abacoabbaglio";
	std::string payPassword = "accennoaccusato";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is italian") {
		mnemonic = MasterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
	}

	SECTION("Mnemonic import with wrong mnemonic password is italian") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is japanese", "[MasterWalletManager]") {

	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());

	std::string phrasePassword = "あいこくしんあいさつあいた";
	std::string payPassword = "あきるあけがた";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is japanese") {
		mnemonic = MasterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
	}

	SECTION("Mnemonic import with wrong mnemonic password is japanese") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is spanish", "[MasterWalletManager]") {

	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());

	std::string phrasePassword = "ábacoabdomenabeja";
	std::string payPassword = "abogadoabonoabuelo";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is spanish") {
		mnemonic = MasterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
	}

	SECTION("Mnemonic import with wrong mnemonic password is spanish") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is mix all", "[MasterWalletManager]") {

	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());

	std::string phrasePassword = "1a웃유abdiquerabacoあいこくしんábaco";
	std::string payPassword = "1a웃유abdiquerabacoあいこくしんábacos";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is mix all") {
		mnemonic = MasterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());

		boost::scoped_ptr<IMasterWallet> masterWallet2(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

		//Mnemonic import with wrong phrase password
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, "wrong pass", payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());

		//Mnemonic import of different pay password
		boost::scoped_ptr<IMasterWallet> masterWallet4(MasterWalletManager->ImportWalletWithMnemonic(
			mnemonic, phrasePassword, "diff pass"));
		REQUIRE(masterWallet4 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() == masterWallet4->GetPublicKey());
	}

	SECTION("Mnemonic import with wrong mnemonic password is mix all") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(MasterWalletManager->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}
*/



//fixme [hp]
//TEST_CASE("test p2p net stop error use", "[MasterWalletManager]") {
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
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
//
//TEST_CASE("Test crash", "[MasterWalletManager]") {
//	boost::scoped_ptr<MasterWalletManager> walletFact(new MasterWalletManager());
//	std::string phrasePassword = "phrasepassword";
//	std::string payPassword = "payPassword";
//	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
//
//	SECTION("my test carsh") {
//		IMasterWallet *masterWallet = walletFact->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
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
//
//
//
//
//TEST_CASE("WalletFactoryInner::importWalletInternal Test ", "[WalletFactoryInner]") {
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//
//	SECTION("languae is valid ImportWalletWithMnemonic correct") {
//		std::string phrasePassword = "phrasePassword";
//		std::string payPassword = "payPassword";
//		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"chinese");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
////	SECTION("languae is invalid not support germany. will be assert.    so closed-------->") {
////		std::string phrasePassword = "phrasePassword";
////		std::string payPassword = "payPassword";
////		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
////
////		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword, "Germany");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////	}
//	SECTION("walletImportFun return false ") {
//		std::string phrasePassword = "phrasePassword";
//		std::string payPassword = "payPassword";
//		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 漫";
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"chinese");
//		REQUIRE(masterWallet == nullptr);
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//
////	SECTION("masterWallet->Initialized() return false  no this situation so cloesed -------->") {
////		std::string phrasePassword = "phrasePassword";
////		std::string payPassword = "payPassword";
////		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 Hello";
////
////		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword, "chinese");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////	}
//
//}
//
//
//TEST_CASE("MasterWalletManager create destroy wallet", "[MasterWalletManager]") {
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//
//	SECTION("create destroy french wallet") {
//		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "french");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//	SECTION("create destroy spanish wallet") {
//		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "spanish");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//	SECTION("create destroy japanese wallet") {
//		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "japanese");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//	SECTION("create destroy italian wallet") {
//		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "italian");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//	SECTION("create destroy chinese wallet") {
//		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "chinese");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//	SECTION("create destroy english wallet") {
//		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "english");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//}
//
//TEST_CASE("Wallet factory export & import  WithKeystore mnemonic", "[MasterWalletManager]") {
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//	std::string masterWalletId = "MasterWalletId";
//
//
//	SECTION("export & import  WithKeystore mnemonic spanish ") {
//		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"spanish");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string keystorePath = "testSpanishMnemonic.json";
//		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
//																					 backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//		masterWalletManager->DestroyWallet(masterWallet2);
//	}
//	SECTION("export & import  WithKeystore mnemonic chinese ") {
//		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"chinese");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//
//		std::string keystorePath = "testchineseMnemonic.json";
//		std::string payPassword = "payPassword";
//		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
//																					 backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//		masterWalletManager->DestroyWallet(masterWallet2);
//	}
//	SECTION("export & import  WithKeystore mnemonic french ") {
//		std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"french");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string payPassword = "payPassword";
//		std::string keystorePath = "testfrenchMnemonic.json";
//		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
//																					 backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//		masterWalletManager->DestroyWallet(masterWallet2);
//	}
//	SECTION("export & import  WithKeystore mnemonic japanese ") {
//		std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"japanese");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string payPassword = "payPassword";
//		std::string keystorePath = "testjapaneseMnemonic.json";
//		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
//																					 backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//		masterWalletManager->DestroyWallet(masterWallet2);
//	}
//	SECTION("export & import  WithKeystore mnemonic italian ") {
//		std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";
//
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"italian");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string payPassword = "payPassword";
//		std::string keystorePath = "testitalianMnemonic.json";
//		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
//																					 backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//		masterWalletManager->DestroyWallet(masterWallet2);
//	}
//	SECTION("export & import  WithKeystore mnemonic english ") {
//		std::string mnemonic = "drink false ribbon equal reward happy olive later silly track business sail";
//
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"english");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string payPassword = "payPassword";
//		std::string keystorePath = "testenglishMnemonic.json";
//		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
//																					 backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//		masterWalletManager->DestroyWallet(masterWallet2);
//	}
//}
//
//
//TEST_CASE("Wallet factory Import Export  WalletWithMnemonic mnemonic ", "[MasterWalletManager]") {
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//
//	SECTION("Import Export  WalletWithMnemonic mnemonic english ") {
//		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"english");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//
//	SECTION("Import Export  WalletWithMnemonic mnemonic chinese ") {
//
//		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"chinese");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//	SECTION("Import Export  WalletWithMnemonic mnemonic french ") {
//
//		std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";
//
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"french");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//	SECTION("Import Export  WalletWithMnemonic mnemonic spanish ") {
//
//		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
//
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"spanish");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//	SECTION("Import Export  WalletWithMnemonic mnemonic japanese ") {
//
//		std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";
//
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"japanese");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//	SECTION("Import Export  WalletWithMnemonic mnemonic italian ") {
//
//		std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";
//
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"italian");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet("ELA", payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = masterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		masterWalletManager->DestroyWallet(masterWallet);
//	}
//}
//
//TEST_CASE("Wallet CreateMasterWallet method", "[CreateMasterWallet]") {
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//	std::string masterWalletId = "MasterWalletId";
//
//	SECTION("Normal creation") {
//		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, phrasePassword,
//																			  payPassword);
//		REQUIRE(masterWallet != nullptr);
//		delete masterWallet;
//	}
//	SECTION("Create with phrase password can be empty") {
//		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, "", payPassword);
//		REQUIRE(masterWallet != nullptr);
//		delete masterWallet;
//	}
//	SECTION("Create with phrase password that is empty or less than 8") {
//		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, "ilegal", payPassword),
//						std::invalid_argument);
//	}
//	SECTION("Create with phrase password that is more than 128") {
//		REQUIRE_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId,
//																  "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
//																  payPassword), std::invalid_argument);
//	}
//	SECTION("Create with pay password that is empty or less than 8") {
//		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, phrasePassword, ""),
//						std::invalid_argument);
//		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, phrasePassword, "ilegal"),
//						std::invalid_argument);
//	}
//	SECTION("Create with pay password that is more than 128") {
//		REQUIRE_THROWS_AS(masterWalletManager->CreateMasterWallet(masterWalletId, phrasePassword,
//																  "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
//						  std::invalid_argument);
//	}
//	SECTION("Language should not be null") {
//		CHECK_THROWS_AS(masterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, ""),
//						std::invalid_argument);
//	}
//	//mnemonic related test is in mnemonic special test suit
//}
//
//TEST_CASE("Wallet DestroyWallet method", "[DestroyWallet]") {
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//	std::string masterWalletId = "MasterWalletId";
//
//	SECTION("Normal destroying") {
//		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, phrasePassword,
//																			  payPassword);
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWallet));
//	}
//	SECTION("Destroying null pointer") {
//		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(nullptr));
//	}
//}
//
//TEST_CASE("Wallet ImportWalletWithKeystore method", "[ImportWalletWithKeystore]") {
//	//todo [zcl] complete me
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//	std::string backupPassword = "backupPassword";
//
//	SECTION("Open") {
//		if (boost::filesystem::exists("wallet.store")) {
//			IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithKeystore("wallet.store", backupPassword,
//																						payPassword, phrasePassword,
//																						"Data");
//			REQUIRE(masterWallet != nullptr);
//			REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWallet));
//		}
//	}
//}
//
//TEST_CASE("Wallet ExportWalletWithKeystore method", "[ExportWalletWithKeystore]") {
//	//todo [zcl] complete me
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//	std::string backupPassword = "backupPassword";
//	std::string masterWalletId = "MasterWalletId";
//
//	SECTION("Save") {
//		IMasterWallet *masterWallet = masterWalletManager->CreateMasterWallet(masterWalletId, phrasePassword,
//																			  payPassword);
//		REQUIRE(masterWallet != nullptr);
//		masterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, "wallet.store");
//		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWallet));
//	}
//}
//
//TEST_CASE("Wallet ImportWalletWithMnemonic method", "[ImportWalletWithMnemonic]") {
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//	std::string masterWalletId = "MasterWalletId";
//	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
//
//	SECTION("Normal importing") {
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword,
//																					payPassword,
//																					"english", "Data");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWallet));
//	}
//	SECTION("Normal importing with default parameters") {
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
//																					phrasePassword, payPassword);
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWallet));
//	}
//	SECTION("Import with phrase password can be empty") {
//		IMasterWallet *masterWallet = masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, "",
//																					payPassword);
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE_NOTHROW(masterWalletManager->DestroyWallet(masterWallet));
//	}
//	SECTION("Import with phrase password that is less than 8") {
//		REQUIRE_THROWS_AS(
//				masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, "ilegal", payPassword),
//				std::invalid_argument);
//	}
//	SECTION("Import with phrase password that is more than 128") {
//		REQUIRE_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
//																		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
//																		payPassword), std::invalid_argument);
//	}
//	SECTION("Import with pay password that is empty or less than 8") {
//		CHECK_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, ""),
//						std::invalid_argument);
//		CHECK_THROWS_AS(
//				masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, "ilegal"),
//				std::invalid_argument);
//	}
//	SECTION("Import with pay password that is more than 128") {
//		REQUIRE_THROWS_AS(masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
//																		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
//						  std::invalid_argument);
//	}
//	SECTION("Language should not be null") {
//		CHECK_THROWS_AS(
//				masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, payPassword,
//															  ""),
//				std::invalid_argument);
//	}
//}
//
//TEST_CASE("Wallet ExportWalletWithMnemonic method", "[ExportWalletWithMnemonic]") {
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//	std::string masterWalletId = "MasterWalletId";
//	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
//	boost::scoped_ptr<IMasterWallet> masterWallet(
//			masterWalletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, payPassword));
//
//	SECTION("Normal exporting") {
//		//todo [zcl] should pass
//		std::string actual = masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
//		REQUIRE(actual == mnemonic);
//	}
//	SECTION("Export with wrong password") {
//		//todo [zcl] should pass
//		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), "wrongPassword"),
//						std::logic_error);
//	}
//
//	SECTION("Export with null pointer of mater wallet") {
//		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(nullptr, "wrongPassword"),
//						std::invalid_argument);
//	}
//	SECTION("Export with pay password that is empty or less than 8") {
//		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), ""), std::invalid_argument);
//		CHECK_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(), "ilegal"),
//						std::invalid_argument);
//	}
//	SECTION("Export with pay password that is more than 128") {
//		REQUIRE_THROWS_AS(masterWalletManager->ExportWalletWithMnemonic(masterWallet.get(),
//																		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
//						  std::invalid_argument);
//	}
//}
//
//TEST_CASE("Wallet factory key store import", "[MasterWalletManager]") {
//	boost::scoped_ptr<MasterWalletManager> masterWalletManager(new MasterWalletManager());
//	std::string payPassword = "11111111";
//	std::string backupPassword = "11111111";
//	std::string masterWalletId = "MasterWalletId";
//	std::string keystorePath = "webwallet.json";
//
//	if (boost::filesystem::exists(keystorePath)) {
//		IMasterWallet *pMasterWallet = masterWalletManager->ImportWalletWithKeystore(masterWalletId, keystorePath,
//																					 backupPassword, payPassword);
//		if (pMasterWallet) {
//			boost::scoped_ptr<IMasterWallet> masterWallet(pMasterWallet);
//		}
//	}
//}
//
////	SECTION("create destroy spanish wallet") {
////		IMasterWallet *masterWallet = MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "spanish");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////	SECTION("create destroy japanese wallet") {
////		IMasterWallet *masterWallet = MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "japanese");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////	SECTION("create destroy italian wallet") {
////		IMasterWallet *masterWallet = MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "italian");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////	SECTION("create destroy chinese wallet") {
////		IMasterWallet *masterWallet = MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "chinese");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////	SECTION("create destroy english wallet") {
////		IMasterWallet *masterWallet = MasterWalletManager->CreateMasterWallet("MasterWalletId", phrasePassword, payPassword, "english");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////}
////
////
////TEST_CASE("Wallet factory export & import  WithKeystore mnemonic", "[MasterWalletManager]") {
////	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());
////	std::string phrasePassword = "phrasePassword";
////	std::string payPassword = "payPassword";
////
////
////	SECTION("export & import  WithKeystore mnemonic spanish ") {
////		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
////																			  "spanish");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////
////
////		std::string backupPassword = "backupPassword";
////		std::string keystorePath = "testSpanishMnemonic.json";
////		MasterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
////		REQUIRE(boost::filesystem::exists(keystorePath));
////
////
////		IMasterWallet *masterWallet2 = MasterWalletManager->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
////		REQUIRE(masterWallet2 != nullptr);
////		REQUIRE(!masterWallet2->GetPublicKey().empty());
////
////		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
////		boost::filesystem::remove(keystorePath);
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////		MasterWalletManager->DestroyWallet(masterWallet2);
////	}
////	SECTION("export & import  WithKeystore mnemonic chinese ") {
////		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
////																			  "chinese");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////
////
////		std::string backupPassword = "backupPassword";
////		std::string keystorePath = "testchineseMnemonic.json";
////		MasterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
////		REQUIRE(boost::filesystem::exists(keystorePath));
////
////
////		IMasterWallet *masterWallet2 = MasterWalletManager->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
////		REQUIRE(masterWallet2 != nullptr);
////		REQUIRE(!masterWallet2->GetPublicKey().empty());
////
////		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
////		boost::filesystem::remove(keystorePath);
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////		MasterWalletManager->DestroyWallet(masterWallet2);
////	}
////	SECTION("export & import  WithKeystore mnemonic french ") {
////		std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
////																			  "french");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////
////
////		std::string backupPassword = "backupPassword";
////		std::string keystorePath = "testfrenchMnemonic.json";
////		MasterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
////		REQUIRE(boost::filesystem::exists(keystorePath));
////
////
////		IMasterWallet *masterWallet2 = MasterWalletManager->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
////		REQUIRE(masterWallet2 != nullptr);
////		REQUIRE(!masterWallet2->GetPublicKey().empty());
////
////		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
////		boost::filesystem::remove(keystorePath);
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////		MasterWalletManager->DestroyWallet(masterWallet2);
////	}
////	SECTION("export & import  WithKeystore mnemonic japanese ") {
////		std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
////																			  "japanese");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////
////
////		std::string backupPassword = "backupPassword";
////		std::string keystorePath = "testjapaneseMnemonic.json";
////		MasterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
////		REQUIRE(boost::filesystem::exists(keystorePath));
////
////
////		IMasterWallet *masterWallet2 = MasterWalletManager->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
////		REQUIRE(masterWallet2 != nullptr);
////		REQUIRE(!masterWallet2->GetPublicKey().empty());
////
////		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
////		boost::filesystem::remove(keystorePath);
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////		MasterWalletManager->DestroyWallet(masterWallet2);
////	}
////	SECTION("export & import  WithKeystore mnemonic italian ") {
////		std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";
////
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
////																			  "italian");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////
////
////		std::string backupPassword = "backupPassword";
////		std::string keystorePath = "testitalianMnemonic.json";
////		MasterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
////		REQUIRE(boost::filesystem::exists(keystorePath));
////
////
////		IMasterWallet *masterWallet2 = MasterWalletManager->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
////		REQUIRE(masterWallet2 != nullptr);
////		REQUIRE(!masterWallet2->GetPublicKey().empty());
////
////		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
////		boost::filesystem::remove(keystorePath);
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////		MasterWalletManager->DestroyWallet(masterWallet2);
////	}
////	SECTION("export & import  WithKeystore mnemonic english ") {
////		std::string mnemonic = "drink false ribbon equal reward happy olive later silly track business sail";
////
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
////																			  "english");
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////
////
////		std::string backupPassword = "backupPassword";
////		std::string keystorePath = "testenglishMnemonic.json";
////		MasterWalletManager->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
////		REQUIRE(boost::filesystem::exists(keystorePath));
////
////
////		IMasterWallet *masterWallet2 = MasterWalletManager->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
////		REQUIRE(masterWallet2 != nullptr);
////		REQUIRE(!masterWallet2->GetPublicKey().empty());
////
////		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
////		boost::filesystem::remove(keystorePath);
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////		MasterWalletManager->DestroyWallet(masterWallet2);
////	}
////}
////
////
////TEST_CASE("Wallet factory Import Export  WalletWithMnemonic mnemonic ", "[MasterWalletManager]") {
////	boost::scoped_ptr<MasterWalletManager> MasterWalletManager(new MasterWalletManager());
////
////	std::string phrasePassword = "phrasePassword";
////	std::string payPassword = "payPassword";
////
////	SECTION("Import Export  WalletWithMnemonic mnemonic english ") {
////		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
////			, "english");
////
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
////
////		std::string mnemonicExport = MasterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
////		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
////		REQUIRE(mnemonicExport == mnemonic);
////
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////
////	SECTION("Import Export  WalletWithMnemonic mnemonic chinese ") {
////
////		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
////			, "chinese");
////
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
////
////		std::string mnemonicExport = MasterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
////		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
////		REQUIRE(mnemonicExport == mnemonic);
////
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////	SECTION("Import Export  WalletWithMnemonic mnemonic french ") {
////
////		std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";
////
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
////			, "french");
////
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
////
////		std::string mnemonicExport = MasterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
////		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
////		REQUIRE(mnemonicExport == mnemonic);
////
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////	SECTION("Import Export  WalletWithMnemonic mnemonic spanish ") {
////
////		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
////
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
////			, "spanish");
////
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
////
////		std::string mnemonicExport = MasterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
////		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
////		REQUIRE(mnemonicExport == mnemonic);
////
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////	SECTION("Import Export  WalletWithMnemonic mnemonic japanese ") {
////
////		std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";
////
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
////			, "japanese");
////
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
////
////		std::string mnemonicExport = MasterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
////		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
////		REQUIRE(mnemonicExport == mnemonic);
////
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////	SECTION("Import Export  WalletWithMnemonic mnemonic italian ") {
////
////		std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";
////
////		IMasterWallet *masterWallet = MasterWalletManager->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
////			, "italian");
////
////		REQUIRE(masterWallet != nullptr);
////		REQUIRE(!masterWallet->GetPublicKey().empty());
////
////		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
////		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
////		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
////
////		std::string mnemonicExport = MasterWalletManager->ExportWalletWithMnemonic(masterWallet, payPassword);
////		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
////		REQUIRE(mnemonicExport == mnemonic);
////
////		dumyMasterWallet.DestroyWallet(subWallet);
////		MasterWalletManager->DestroyWallet(masterWallet);
////	}
////}
//
