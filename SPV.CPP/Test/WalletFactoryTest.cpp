// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <climits>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

#include "catch.hpp"

#include "WalletFactory.h"
#include "MasterWallet.h"

using namespace Elastos::SDK;

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


TEST_CASE(
	"Wallet factory ExportWalletWithMnemonic generate  mnemonic (english  chinese italian japanese spanish french)",
	"[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic;


	SECTION("generate french mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
			walletFactory->CreateMasterWallet(phrasePassword, payPassword, "french"));
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());
	}
	SECTION("generate spanish mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
			walletFactory->CreateMasterWallet(phrasePassword, payPassword, "spanish"));
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());
	}

	SECTION("generate japanese mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
			walletFactory->CreateMasterWallet(phrasePassword, payPassword, "japanese"));
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());
	}

	SECTION("generate italian mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
			walletFactory->CreateMasterWallet(phrasePassword, payPassword, "italian"));

		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());
	}

	SECTION("generate chinese mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
			walletFactory->CreateMasterWallet(phrasePassword, payPassword, "chinese"));
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());
	}

	SECTION("generate english mnemonic") {
		boost::scoped_ptr<IMasterWallet> masterWallet(
			walletFactory->CreateMasterWallet(phrasePassword, payPassword, "english"));
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(!mnemonic.empty());
	}
}


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

/*
 //////////////////////////////// password is character ,digit,special symbol,  chinese, french,  italian, japanese, spanish and mix(all of this)
TEST_CASE("Wallet factory mnemonic export & import password is character", "[WalletFactory]") {

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

	SECTION("Mnemonic import with wrong mnemonic") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}


TEST_CASE("Wallet factory mnemonic export & import password is digit", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "1234567890123";
	std::string payPassword = "22222222222";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is digit") {
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

	SECTION("Mnemonic import with wrong mnemonic password is digit") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is special symbol", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "❤❥웃유♋☮㊣㊎";
	std::string payPassword = "❤❥웃유♋☮㊣㊎";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is special symbol") {
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

	SECTION("Mnemonic import with wrong mnemonic password is special symbol") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is digit|character|special symbol", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "❤❥웃유123ab";
	std::string payPassword = "❤❥웃유456cd";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is digit|character|special symbol") {
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

	SECTION("Mnemonic import with wrong mnemonic password is digit|character|special symbol") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is chinese", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "测试中文为密码";
	std::string payPassword = "的情况怎么样";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is chinese") {
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

	SECTION("Mnemonic import with wrong mnemonic password is chinese") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}


TEST_CASE("Wallet factory mnemonic export & import password is french", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "abaisseraboyer";
	std::string payPassword = "aboutiraborder";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is french") {
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

	SECTION("Mnemonic import with wrong mnemonic password is french") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is italian", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "abacoabbaglio";
	std::string payPassword = "accennoaccusato";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is italian") {
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

	SECTION("Mnemonic import with wrong mnemonic password is italian") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is japanese", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "あいこくしんあいさつあいた";
	std::string payPassword = "あきるあけがた";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is japanese") {
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

	SECTION("Mnemonic import with wrong mnemonic password is japanese") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is spanish", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "ábacoabdomenabeja";
	std::string payPassword = "abogadoabonoabuelo";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is spanish") {
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

	SECTION("Mnemonic import with wrong mnemonic password is spanish") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}

TEST_CASE("Wallet factory mnemonic export & import password is mix all", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "1a웃유abdiquerabacoあいこくしんábaco";
	std::string payPassword = "1a웃유abdiquerabacoあいこくしんábacos";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword));

	std::string mnemonic;
	SECTION("Mnemonic export password is mix all") {
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

	SECTION("Mnemonic import with wrong mnemonic password is mix all") {
		std::string wrongMnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<IMasterWallet> masterWallet3(walletFactory->ImportWalletWithMnemonic(
			wrongMnemonic, phrasePassword, payPassword));
		REQUIRE(masterWallet3 != nullptr);
		REQUIRE(masterWallet->GetPublicKey() != masterWallet3->GetPublicKey());
	}
}
*/




/*TEST_CASE("test p2p net stop error use --------", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "drink false ribbon equal reward happy olive later silly track business sail";


	SECTION("Create master wallet mnemonic english ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		masterWallet->DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
	}
}*/

////////////////////////////////////////////




TEST_CASE("WalletFactoryInner::importWalletInternal Test ", "[WalletFactoryInner]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	SECTION("languae is valid ImportWalletWithMnemonic correct") {
		std::string phrasePassword = "phrasePassword";
		std::string payPassword = "payPassword";
		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "chinese");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		walletFactory->DestroyWallet(masterWallet);
	}
//	SECTION("languae is invalid not support germany. will be assert.    so closed-------->") {
//		std::string phrasePassword = "phrasePassword";
//		std::string payPassword = "payPassword";
//		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
//
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword, "Germany");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//	}
	SECTION("walletImportFun return false ") {
		std::string phrasePassword = "phrasePassword";
		std::string payPassword = "payPassword";
		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 漫";
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "chinese");
		REQUIRE(masterWallet == nullptr);
		walletFactory->DestroyWallet(masterWallet);
	}

//	SECTION("masterWallet->Initialized() return false  no this situation so cloesed -------->") {
//		std::string phrasePassword = "phrasePassword";
//		std::string payPassword = "payPassword";
//		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 Hello";
//
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword, "chinese");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//	}

}


TEST_CASE("WalletFactory create destroy wallet", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	SECTION("create destroy french wallet") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "french");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		walletFactory->DestroyWallet(masterWallet);
	}
	SECTION("create destroy spanish wallet") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		walletFactory->DestroyWallet(masterWallet);
	}
	SECTION("create destroy japanese wallet") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "japanese");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		walletFactory->DestroyWallet(masterWallet);
	}
	SECTION("create destroy italian wallet") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "italian");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		walletFactory->DestroyWallet(masterWallet);
	}
	SECTION("create destroy chinese wallet") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "chinese");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		walletFactory->DestroyWallet(masterWallet);
	}
	SECTION("create destroy english wallet") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());
		walletFactory->DestroyWallet(masterWallet);
	}
}

TEST_CASE("Wallet factory export & import  WithKeystore mnemonic", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";


	SECTION("export & import  WithKeystore mnemonic spanish ") {
		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "spanish");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);


		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testSpanishMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));


		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword,
																			   payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
		walletFactory->DestroyWallet(masterWallet2);
	}
	SECTION("export & import  WithKeystore mnemonic chinese ") {
		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "chinese");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);


		std::string backupPassword = "backupPassword";

		std::string keystorePath = "testchineseMnemonic.json";
		std::string payPassword = "payPassword";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));


		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword,
																			   payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
		walletFactory->DestroyWallet(masterWallet2);
	}
	SECTION("export & import  WithKeystore mnemonic french ") {
		std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "french");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);


		std::string backupPassword = "backupPassword";
		std::string payPassword = "payPassword";
		std::string keystorePath = "testfrenchMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));


		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword,
																			   payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
		walletFactory->DestroyWallet(masterWallet2);
	}
	SECTION("export & import  WithKeystore mnemonic japanese ") {
		std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "japanese");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);


		std::string backupPassword = "backupPassword";
		std::string payPassword = "payPassword";
		std::string keystorePath = "testjapaneseMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));


		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword,
																			   payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
		walletFactory->DestroyWallet(masterWallet2);
	}
	SECTION("export & import  WithKeystore mnemonic italian ") {
		std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";

		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "italian");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);


		std::string backupPassword = "backupPassword";
		std::string payPassword = "payPassword";
		std::string keystorePath = "testitalianMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));


		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword,
																			   payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
		walletFactory->DestroyWallet(masterWallet2);
	}
	SECTION("export & import  WithKeystore mnemonic english ") {
		std::string mnemonic = "drink false ribbon equal reward happy olive later silly track business sail";

		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "english");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);


		std::string backupPassword = "backupPassword";
		std::string payPassword = "payPassword";
		std::string keystorePath = "testenglishMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));


		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword,
																			   payPassword);
		REQUIRE(masterWallet2 != nullptr);
		REQUIRE(!masterWallet2->GetPublicKey().empty());

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
		walletFactory->DestroyWallet(masterWallet2);
	}
}


TEST_CASE("Wallet factory Import Export  WalletWithMnemonic mnemonic ", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	SECTION("Import Export  WalletWithMnemonic mnemonic english ") {
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "english");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
	}

	SECTION("Import Export  WalletWithMnemonic mnemonic chinese ") {

		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "chinese");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
	}
	SECTION("Import Export  WalletWithMnemonic mnemonic french ") {

		std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";

		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "french");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
	}
	SECTION("Import Export  WalletWithMnemonic mnemonic spanish ") {

		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";

		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "spanish");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
	}
	SECTION("Import Export  WalletWithMnemonic mnemonic japanese ") {

		std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";

		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "japanese");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
	}
	SECTION("Import Export  WalletWithMnemonic mnemonic italian ") {

		std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";

		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "italian");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length());;
		REQUIRE(mnemonicExport == mnemonic);

		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
	}
}

TEST_CASE("Wallet CreateMasterWallet method", "[CreateMasterWallet]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	SECTION("Normal creation") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword);
		REQUIRE(masterWallet != nullptr);
		delete masterWallet;
	}
	SECTION("Create with phrase password can be empty") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet("", payPassword);
		REQUIRE(masterWallet != nullptr);
		delete masterWallet;
	}
	SECTION("Create with phrase password that is empty or less than 8") {
		CHECK_THROWS_AS(walletFactory->CreateMasterWallet("ilegal", payPassword), std::invalid_argument);
	}
	SECTION("Create with phrase password that is more than 128") {
		REQUIRE_THROWS_AS(walletFactory->CreateMasterWallet(
			"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
			payPassword), std::invalid_argument);
	}
	SECTION("Create with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(walletFactory->CreateMasterWallet(phrasePassword, ""), std::invalid_argument);
		CHECK_THROWS_AS(walletFactory->CreateMasterWallet(phrasePassword, "ilegal"), std::invalid_argument);
	}
	SECTION("Create with pay password that is more than 128") {
		REQUIRE_THROWS_AS(walletFactory->CreateMasterWallet(phrasePassword,
															"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
	SECTION("Language should not be null") {
		CHECK_THROWS_AS(walletFactory->CreateMasterWallet(phrasePassword, payPassword, ""), std::invalid_argument);
	}
	SECTION("Data path should not be null") {
		CHECK_THROWS_AS(walletFactory->CreateMasterWallet(phrasePassword, payPassword, "english", ""),
						std::invalid_argument);
	}
	//mnemonic related test is in mnemonic special test suit
}

TEST_CASE("Wallet DestroyWallet method", "[DestroyWallet]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	SECTION("Normal destroying") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(walletFactory->DestroyWallet(masterWallet));
	}
	SECTION("Destroying null pointer") {
		REQUIRE_NOTHROW(walletFactory->DestroyWallet(nullptr));
	}
}

TEST_CASE("Wallet ImportWalletWithKeystore method", "[ImportWalletWithKeystore]") {
	//todo [zcl] complete me
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string backupPassword = "backupPassword";

	SECTION("Open") {
		if (boost::filesystem::exists("wallet.store")) {
			IMasterWallet *masterWallet = walletFactory->ImportWalletWithKeystore("wallet.store", backupPassword,
																				  payPassword, phrasePassword, "Data");
			REQUIRE(masterWallet != nullptr);
			REQUIRE_NOTHROW(walletFactory->DestroyWallet(masterWallet));
		}
	}
}

TEST_CASE("Wallet ExportWalletWithKeystore method", "[ExportWalletWithKeystore]") {
	//todo [zcl] complete me
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string backupPassword = "backupPassword";

	SECTION("Save") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword);
		REQUIRE(masterWallet != nullptr);
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, payPassword, "wallet.store");
		REQUIRE_NOTHROW(walletFactory->DestroyWallet(masterWallet));
	}
}

TEST_CASE("Wallet ImportWalletWithMnemonic method", "[ImportWalletWithMnemonic]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";

	SECTION("Normal importing") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "english", "Data");
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(walletFactory->DestroyWallet(masterWallet));
	}
	SECTION("Normal importing with default parameters") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(walletFactory->DestroyWallet(masterWallet));
	}
	SECTION("Import with phrase password can be empty") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, "", payPassword);
		REQUIRE(masterWallet != nullptr);
		REQUIRE_NOTHROW(walletFactory->DestroyWallet(masterWallet));
	}
	SECTION("Import with phrase password that is less than 8") {
		REQUIRE_THROWS_AS(walletFactory->ImportWalletWithMnemonic(mnemonic, "ilegal", payPassword),
						  std::invalid_argument);
	}
	SECTION("Import with phrase password that is more than 128") {
		REQUIRE_THROWS_AS(walletFactory->ImportWalletWithMnemonic(mnemonic,
																  "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
																  payPassword), std::invalid_argument);
	}
	SECTION("Import with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, ""), std::invalid_argument);
		CHECK_THROWS_AS(walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, "ilegal"),
						std::invalid_argument);
	}
	SECTION("Import with pay password that is more than 128") {
		REQUIRE_THROWS_AS(walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword,
																  "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
	SECTION("Language should not be null") {
		CHECK_THROWS_AS(walletFactory->ImportWalletWithMnemonic(phrasePassword, payPassword, ""),
						std::invalid_argument);
	}
	SECTION("RootPath should not be null") {
		CHECK_THROWS_AS(walletFactory->ImportWalletWithMnemonic(phrasePassword, payPassword, "english", ""),
						std::invalid_argument);
	}
}

TEST_CASE("Wallet ExportWalletWithMnemonic method", "[ExportWalletWithMnemonic]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword));

	SECTION("Normal exporting") {
		//todo [zcl] should pass
		std::string actual = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		REQUIRE(actual == mnemonic);
	}
	SECTION("Export with wrong password") {
		//todo [zcl] should pass
		CHECK_THROWS_AS(walletFactory->ExportWalletWithMnemonic(masterWallet.get(), "wrongPassword"),
						std::logic_error);
	}

	SECTION("Export with null pointer of mater wallet") {
		CHECK_THROWS_AS(walletFactory->ExportWalletWithMnemonic(nullptr, "wrongPassword"),
						std::invalid_argument);
	}
	SECTION("Export with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(walletFactory->ExportWalletWithMnemonic(masterWallet.get(), ""), std::invalid_argument);
		CHECK_THROWS_AS(walletFactory->ExportWalletWithMnemonic(masterWallet.get(), "ilegal"),
						std::invalid_argument);
	}
	SECTION("Export with pay password that is more than 128") {
		REQUIRE_THROWS_AS(walletFactory->ExportWalletWithMnemonic(masterWallet.get(),
																  "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
}

TEST_CASE("Wallet factory key store import", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
	std::string payPassword = "11111111";
	std::string backupPassword = "11111111";
	std::string keystorePath = "webwallet.json";

	if (boost::filesystem::exists(keystorePath)) {
		IMasterWallet *pMasterWallet = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword,
																			   payPassword);
		if (pMasterWallet) {
			boost::scoped_ptr<IMasterWallet> masterWallet(pMasterWallet);
		}
	}
}

//	SECTION("create destroy spanish wallet") {
//		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "spanish");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		walletFactory->DestroyWallet(masterWallet);
//	}
//	SECTION("create destroy japanese wallet") {
//		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "japanese");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		walletFactory->DestroyWallet(masterWallet);
//	}
//	SECTION("create destroy italian wallet") {
//		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "italian");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		walletFactory->DestroyWallet(masterWallet);
//	}
//	SECTION("create destroy chinese wallet") {
//		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "chinese");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		walletFactory->DestroyWallet(masterWallet);
//	}
//	SECTION("create destroy english wallet") {
//		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword, "english");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//		walletFactory->DestroyWallet(masterWallet);
//	}
//}
//
//
//TEST_CASE("Wallet factory export & import  WithKeystore mnemonic", "[WalletFactory]") {
//	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//
//
//	SECTION("export & import  WithKeystore mnemonic spanish ") {
//		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
//																			  "spanish");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string keystorePath = "testSpanishMnemonic.json";
//		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//		walletFactory->DestroyWallet(masterWallet2);
//	}
//	SECTION("export & import  WithKeystore mnemonic chinese ") {
//		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
//																			  "chinese");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string keystorePath = "testchineseMnemonic.json";
//		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//		walletFactory->DestroyWallet(masterWallet2);
//	}
//	SECTION("export & import  WithKeystore mnemonic french ") {
//		std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
//																			  "french");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string keystorePath = "testfrenchMnemonic.json";
//		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//		walletFactory->DestroyWallet(masterWallet2);
//	}
//	SECTION("export & import  WithKeystore mnemonic japanese ") {
//		std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
//																			  "japanese");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string keystorePath = "testjapaneseMnemonic.json";
//		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//		walletFactory->DestroyWallet(masterWallet2);
//	}
//	SECTION("export & import  WithKeystore mnemonic italian ") {
//		std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";
//
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
//																			  "italian");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string keystorePath = "testitalianMnemonic.json";
//		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//		walletFactory->DestroyWallet(masterWallet2);
//	}
//	SECTION("export & import  WithKeystore mnemonic english ") {
//		std::string mnemonic = "drink false ribbon equal reward happy olive later silly track business sail";
//
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
//																			  "english");
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//
//
//		std::string backupPassword = "backupPassword";
//		std::string keystorePath = "testenglishMnemonic.json";
//		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
//		REQUIRE(boost::filesystem::exists(keystorePath));
//
//
//		IMasterWallet *masterWallet2 = walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword);
//		REQUIRE(masterWallet2 != nullptr);
//		REQUIRE(!masterWallet2->GetPublicKey().empty());
//
//		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
//		boost::filesystem::remove(keystorePath);
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//		walletFactory->DestroyWallet(masterWallet2);
//	}
//}
//
//
//TEST_CASE("Wallet factory Import Export  WalletWithMnemonic mnemonic ", "[WalletFactory]") {
//	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
//
//	std::string phrasePassword = "phrasePassword";
//	std::string payPassword = "payPassword";
//
//	SECTION("Import Export  WalletWithMnemonic mnemonic english ") {
//		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
//			, "english");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//	}
//
//	SECTION("Import Export  WalletWithMnemonic mnemonic chinese ") {
//
//		std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
//			, "chinese");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//	}
//	SECTION("Import Export  WalletWithMnemonic mnemonic french ") {
//
//		std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";
//
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
//			, "french");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//	}
//	SECTION("Import Export  WalletWithMnemonic mnemonic spanish ") {
//
//		std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";
//
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
//			, "spanish");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//	}
//	SECTION("Import Export  WalletWithMnemonic mnemonic japanese ") {
//
//		std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";
//
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
//			, "japanese");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//	}
//	SECTION("Import Export  WalletWithMnemonic mnemonic italian ") {
//
//		std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";
//
//		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword
//			, "italian");
//
//		REQUIRE(masterWallet != nullptr);
//		REQUIRE(!masterWallet->GetPublicKey().empty());
//
//		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
//		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
//		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
//
//		std::string mnemonicExport = walletFactory->ExportWalletWithMnemonic(masterWallet, payPassword);
//		mnemonicExport = mnemonicExport.substr(0, mnemonicExport.length() - 1); ;
//		REQUIRE(mnemonicExport == mnemonic);
//
//		dumyMasterWallet.DestroyWallet(subWallet);
//		walletFactory->DestroyWallet(masterWallet);
//	}
//}

