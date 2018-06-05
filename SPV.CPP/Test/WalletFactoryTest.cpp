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

//////////////////////this case is to generate mnemonic of support language (english  chinese italian japanese spanish french)
TEST_CASE("Wallet factory generate french mnemonic ", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword, "french"));

	std::string mnemonic;

	//vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat
	SECTION("generate french mnemonic") {
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		//std::cout<<" generate french mnemonic ----->" <<std::endl;
		//std::cout<< " <<<<<<< "<< mnemonic << ">>>>>>>>" <<std::endl;

		REQUIRE(!mnemonic.empty());
	}
}

/////generate spanish mnemonic
TEST_CASE("Wallet factory generate spanish mnemonic ", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword, "spanish"));

	std::string mnemonic;

	//separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina
	SECTION("generate spanish mnemonic") {
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		//std::cout<<" generate spanish mnemonic ----->" <<std::endl;
		//std::cout<< " <<<<<<< "<< mnemonic << ">>>>>>>>" <<std::endl;

		REQUIRE(!mnemonic.empty());
	}
}

/////generate japanese mnemonic
TEST_CASE("Wallet factory generate japanese mnemonic ", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword, "japanese"));

	std::string mnemonic;

	//separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina
	SECTION("generate japanese mnemonic") {
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		//std::cout<<" generate japanese mnemonic ----->" <<std::endl;
		//std::cout<< " <<<<<<< "<< mnemonic << ">>>>>>>>" <<std::endl;

		REQUIRE(!mnemonic.empty());
	}
}

/////generate italian mnemonic
TEST_CASE("Wallet factory generate italian mnemonic ", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword, "italian"));

	std::string mnemonic;

	//separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina
	SECTION("generate italian mnemonic") {
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		//std::cout<<" generate italian mnemonic ----->" <<std::endl;
		//std::cout<< " <<<<<<< "<< mnemonic << ">>>>>>>>" <<std::endl;

		REQUIRE(!mnemonic.empty());
	}
}

//

/////generate chinese mnemonic
TEST_CASE("Wallet factory generate chinese mnemonic ", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword, "chinese"));

	std::string mnemonic;

	//separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina
	SECTION("generate chinese mnemonic") {
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		//std::cout<<" generate chinese mnemonic ----->" <<std::endl;
		//std::cout<< " <<<<<<< "<< mnemonic << ">>>>>>>>" <<std::endl;

		REQUIRE(!mnemonic.empty());
	}
}
/////generate english mnemonic
TEST_CASE("Wallet factory generate english mnemonic ", "[WalletFactory]") {

	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(
		walletFactory->CreateMasterWallet(phrasePassword, payPassword, "english"));

	std::string mnemonic;

	//separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina
	SECTION("generate english mnemonic") {
		mnemonic = walletFactory->ExportWalletWithMnemonic(masterWallet.get(), payPassword);
		//std::cout<<" generate english mnemonic ----->" <<std::endl;
		//std::cout<< " <<<<<<< "<< mnemonic << ">>>>>>>>" <<std::endl;

		REQUIRE(!mnemonic.empty());
	}
}
////////////////////////////////


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
////////////////////////////////

////////////////////////////////ImportWalletWithMnemonic mnemonic can be english   chinese french,  italian, japanese, spanish and mix(all of this)
TEST_CASE("Wallet factory ImportWalletWithMnemonic mnemonic english", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";


	SECTION("Create master wallet mnemonic english ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword);

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		TestMasterWallet dumyMasterWallet(*static_cast<MasterWallet *>(masterWallet));
		ISubWallet *subWallet = dumyMasterWallet.CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		dumyMasterWallet.DestroyWallet(subWallet);
		walletFactory->DestroyWallet(masterWallet);
	}
}

TEST_CASE("Wallet factory ImportWalletWithMnemonic mnemonic chinese", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";


	//this is invalid mnemonic
	//sstd::string mnemonic = "的 一 是 在 不 了 有 和 人 这 中 大";


	SECTION("Create master wallet mnemonic chinese ") {
		//IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword, "chinese");
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "chinese");
		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		REQUIRE(subWallet != nullptr);

		//nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);


		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
	}
}


TEST_CASE("Wallet factory ImportWalletWithMnemonic mnemonic french", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";


	SECTION("Create master wallet mnemonic french ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "french");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);


		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
	}
}

TEST_CASE("Wallet factory ImportWalletWithMnemonic mnemonic spanish", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";


	SECTION("Create master wallet mnemonic spanish ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "spanish");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);



		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
	}
}

TEST_CASE("Wallet factory ImportWalletWithMnemonic mnemonic japanese", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";


	SECTION("Create master wallet mnemonic japanese ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "japanese");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);

		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
	}
}

TEST_CASE("Wallet factory ImportWalletWithMnemonic mnemonic italian", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";


	SECTION("Create master wallet mnemonic spanish ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "italian");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());

		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);


		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
	}
}

TEST_CASE("Wallet factory ImportWalletWithMnemonic mnemonic english2", "[WalletFactory]") {
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

		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
	}
}

////////////////////////////////////////////

//////////////////////////////////////////////////export & import  WithKeystore  spanish, chinese, french,  italian, japanese
TEST_CASE("Wallet factory export & import  WithKeystore mnemonic spanish", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";


	SECTION("Create master wallet mnemonic spanish ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "spanish");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		//nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);


		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testSpanishMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));
		//, "spanish"
		boost::scoped_ptr<IMasterWallet> masterWallet2(
			walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword));

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		boost::filesystem::remove(keystorePath);
		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
		boost::filesystem::remove(keystorePath);
	}
}

TEST_CASE("Wallet factory export & import  WithKeystore mnemonic chinese", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "脑 搅 墙 淀 式 移 协 分 地 欺 漫 被";


	SECTION("Create master wallet mnemonic chinese ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "chinese");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		//nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);


		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testchineseMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));
		//, "spanish"
		boost::scoped_ptr<IMasterWallet> masterWallet2(
			walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword));

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
		boost::filesystem::remove(keystorePath);
	}
}

TEST_CASE("Wallet factory export & import  WithKeystore mnemonic french", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "vexer lumière palourde séquence nuancer surface dioxyde paradoxe batterie hilarant subvenir grenat";


	SECTION("Create master wallet mnemonic french ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "french");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		//nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);


		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testFrenchMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));
		//, "spanish"
		boost::scoped_ptr<IMasterWallet> masterWallet2(
			walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword));

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
		boost::filesystem::remove(keystorePath);
	}
}

TEST_CASE("Wallet factory export & import  WithKeystore mnemonic japanese", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "たたみ そこそこ ひそか ほうこく そんぞく したぎ のぼる うちがわ せきにん つける してき ひさい";


	SECTION("Create master wallet mnemonic japanese ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "japanese");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		//nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);


		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testJapaneseMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));
		//, "spanish"
		boost::scoped_ptr<IMasterWallet> masterWallet2(
			walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword));

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
		boost::filesystem::remove(keystorePath);
	}
}

TEST_CASE("Wallet factory export & import  WithKeystore mnemonic italian", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "casaccio sfilato bisturi onice pestifero acido profumo spuntino busta bibita angolare inalare";


	SECTION("Create master wallet mnemonic italian ") {
		IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(mnemonic, phrasePassword, payPassword,
																			  "italian");

		REQUIRE(masterWallet != nullptr);
		REQUIRE(!masterWallet->GetPublicKey().empty());


		ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
		//nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);


		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testItalianMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));
		//, "spanish"
		boost::scoped_ptr<IMasterWallet> masterWallet2(
			walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword));

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
		boost::filesystem::remove(keystorePath);
	}
}

TEST_CASE("Wallet factory export & import  WithKeystore mnemonic english", "[WalletFactory]") {
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
		//nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);


		std::string backupPassword = "backupPassword";
		std::string keystorePath = "testEnglishMnemonic.json";
		walletFactory->ExportWalletWithKeystore(masterWallet, backupPassword, keystorePath);
		REQUIRE(boost::filesystem::exists(keystorePath));
		//, "spanish"
		boost::scoped_ptr<IMasterWallet> masterWallet2(
			walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword));

		REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());
		//masterWallet->DestroyWallet(subWallet);
		//walletFactory->DestroyWallet(masterWallet);
		boost::filesystem::remove(keystorePath);
	}
}
///////////////////////////////////////////////////


TEST_CASE("Wallet factory key store export & import english mnemonic", "[WalletFactory]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<IMasterWallet> masterWallet(walletFactory->CreateMasterWallet(phrasePassword, payPassword));
	/////////////////
	ISubWallet *subWallet = masterWallet->CreateSubWallet(Normal, "ELA", 0, payPassword, false);
	//nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
	/////////////////

	std::string backupPassword = "backupPassword";
	std::string keystorePath = "test.json";
	walletFactory->ExportWalletWithKeystore(masterWallet.get(), backupPassword, keystorePath);
	REQUIRE(boost::filesystem::exists(keystorePath));

	boost::scoped_ptr<IMasterWallet> masterWallet2(
		walletFactory->ImportWalletWithKeystore(keystorePath, backupPassword, payPassword));
	REQUIRE(masterWallet->GetPublicKey() == masterWallet2->GetPublicKey());

	boost::filesystem::remove(keystorePath);
}

TEST_CASE("Wallet CreateMasterWallet method", "[CreateMasterWallet]") {
	boost::scoped_ptr<WalletFactory> walletFactory(new WalletFactory());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	SECTION("Normal creation") {
		IMasterWallet *masterWallet = walletFactory->CreateMasterWallet(phrasePassword, payPassword);
		REQUIRE(masterWallet != nullptr);
	}
	SECTION("Create with phrase password that is empty or less than 8") {
		CHECK_THROWS_AS(walletFactory->CreateMasterWallet("", payPassword), std::invalid_argument);
		CHECK_THROWS_AS(walletFactory->CreateMasterWallet("ilegal", payPassword), std::invalid_argument);
	}
	SECTION("Create with phrase password that is more than 128") {
		REQUIRE_THROWS_AS(walletFactory->CreateMasterWallet("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
														 payPassword), std::invalid_argument);
	}
	SECTION("Create with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(walletFactory->CreateMasterWallet(phrasePassword, ""), std::invalid_argument);
		CHECK_THROWS_AS(walletFactory->CreateMasterWallet(phrasePassword, "ilegal"), std::invalid_argument);
	}
	SECTION("Create with phrase password that is more than 128") {
		REQUIRE_THROWS_AS(walletFactory->CreateMasterWallet(phrasePassword, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
	//mnemonic related test is in mnemonic special test suit
}

TEST_CASE("Mnemonic i18n test", "[WalletFactory]") {

}
