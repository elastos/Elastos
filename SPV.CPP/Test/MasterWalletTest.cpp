// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>

#include "catch.hpp"

#include <SDK/Implement/MasterWallet.h>
#include <SDK/Implement/MainchainSubWallet.h>
#include <SDK/Implement/SidechainSubWallet.h>
#include <SDK/Implement/IDChainSubWallet.h>
#include <SDK/Common/Utils.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Block/SidechainMerkleBlock.h>
#include <SDK/Plugin/Block/MerkleBlock.h>
#include <SDK/Plugin/ELAPlugin.h>
#include <SDK/Plugin/IDPlugin.h>

using namespace Elastos::ElaWallet;

#define MasterWalletTestID "MasterWalletTest"

class TestMasterWallet : public MasterWallet {
public:
	TestMasterWallet() :

		MasterWallet(MasterWalletTestID,
					 "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
					 "phrasePassword",
					 "payPassword",
					 false,
					 false,
					 "Data",
					 "Data",
					 0,
					 ImportFromMnemonic) {
	}

	explicit TestMasterWallet(const std::string &phrasePassword,
					 const std::string &payPassword) :
			MasterWallet(MasterWalletTestID,
						 "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
						 phrasePassword,
						 payPassword,
						 false,
						 false,
						 "Data",
						 "Data",
						 0,
						 ImportFromMnemonic) {
	}

	TestMasterWallet(const std::string &mnemonic,
					 const std::string &phrasePassword,
					 const std::string &payPassword) :
			MasterWallet(MasterWalletTestID, mnemonic, phrasePassword, payPassword, false, false, "Data", "Data", 0, ImportFromMnemonic) {
	}

	TestMasterWallet(const std::string &mnemonic,
					 const std::string &phrasePassword,
					 const std::string &payPassword,
					 bool singleAddress) :
			MasterWallet(MasterWalletTestID, mnemonic, phrasePassword, payPassword, singleAddress, false, "Data", "Data", 0, ImportFromMnemonic) {
	}

	TestMasterWallet(const std::string &id) :
			MasterWallet(id, "Data", "Data", false, ImportFromMnemonic) {
	}

	TestMasterWallet(const nlohmann::json &keystore, const std::string &backupPassword, const std::string &payPasswd) :
		MasterWallet(MasterWalletTestID, keystore, backupPassword, payPasswd, "Data", "Data", false, ImportFromKeyStore) {

	}

	nlohmann::json ExportKeystore(const std::string &backupPassword,
										  const std::string &payPassword) {
		return ExportKeyStore(backupPassword, payPassword);
	}

	std::string GetxPubKey() {
		return _account->MasterPubKeyString();
	}
	
	

};

TEST_CASE("Master wallet CreateSubWallet method test", "[CreateSubWallet]") {
	Log::registerMultiLogger();

#ifndef BUILD_SHARED_LIBS
	Log::info("Registering plugin ...");
	REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
	REGISTER_MERKLEBLOCKPLUGIN(IDChain, getIDPluginComponent);
#endif
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	std::string chainID = "IDChain";

	boost::filesystem::remove_all(boost::filesystem::path(std::string("Data/") + MasterWalletTestID));
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

	SECTION("Create mainchain sub wallet") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA");

		SubWallet *normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
		REQUIRE(normalSubWallet != nullptr);

		MainchainSubWallet *mainchainSubWallet = dynamic_cast<MainchainSubWallet *>(subWallet);
		REQUIRE(mainchainSubWallet != nullptr);

		SidechainSubWallet *sidechainSubWallet = dynamic_cast<SidechainSubWallet *>(subWallet);
		REQUIRE(sidechainSubWallet == nullptr);

		masterWallet->DestroyWallet(subWallet);
	}

	SECTION("Create idchain sub wallet") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet("IDChain");

		SubWallet *normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
		REQUIRE(normalSubWallet != nullptr);

		IDChainSubWallet *idChainSubWallet = dynamic_cast<IDChainSubWallet *>(subWallet);
		REQUIRE(idChainSubWallet != nullptr);

		MainchainSubWallet *mainchainSubWallet = dynamic_cast<MainchainSubWallet *>(subWallet);
		REQUIRE(mainchainSubWallet == nullptr);

		masterWallet->DestroyWallet(subWallet);
	}

	SECTION("Create all sub wallets in list") {
		std::vector<std::string> chainIDs = masterWallet->GetSupportedChains();
		for (int i = 0; i < chainIDs.size(); ++i) {
			ISubWallet *subWallet = masterWallet->CreateSubWallet(chainIDs[i]);
			REQUIRE(subWallet != nullptr);
		}
	}

	SECTION("Return exist sub wallet with same id") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainID);
		REQUIRE(subWallet != nullptr);

		//Return same sub wallet with same chain id
		ISubWallet *subWallet1 = masterWallet->CreateSubWallet(chainID);
		REQUIRE(subWallet == subWallet1);

		//Throw param exception when chain id is exist
		REQUIRE_NOTHROW(masterWallet->CreateSubWallet(chainID));

		//Create another sub wallet
		ISubWallet *subWallet3 = masterWallet->CreateSubWallet("ELA");
		REQUIRE(subWallet3 != nullptr);
		REQUIRE(subWallet != subWallet3);

		masterWallet->DestroyWallet(subWallet);
		masterWallet->DestroyWallet(subWallet3);
	}
	SECTION("Create sub wallet with empty chain id") {
		REQUIRE_THROWS(masterWallet->CreateSubWallet(""));
	}
	SECTION("Create sub wallet with chain id that is more than 128") {
		REQUIRE_THROWS(masterWallet->CreateSubWallet(
				"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"));
	}
}

TEST_CASE("Master wallet GenerateMnemonic method test", "[GenerateMnemonic]") {
	Log::registerMultiLogger();
	//mnemonic generating should be random
	SECTION("generate french mnemonic") {
		std::string mnemonic = MasterWallet::GenerateMnemonic("french", "Data");
		std::string mnemonic2 = MasterWallet::GenerateMnemonic("french", "Data");
		REQUIRE(mnemonic != mnemonic2);
	}
	SECTION("generate spanish mnemonic") {
		std::string mnemonic = MasterWallet::GenerateMnemonic("spanish", "Data");
		std::string mnemonic2 = MasterWallet::GenerateMnemonic("spanish", "Data");
		REQUIRE(mnemonic != mnemonic2);
	}

	SECTION("generate japanese mnemonic") {
		std::string mnemonic = MasterWallet::GenerateMnemonic("japanese", "Data");
		std::string mnemonic2 = MasterWallet::GenerateMnemonic("japanese", "Data");
		REQUIRE(mnemonic != mnemonic2);
	}

	SECTION("generate italian mnemonic") {
		std::string mnemonic = MasterWallet::GenerateMnemonic("italian", "Data");
		std::string mnemonic2 = MasterWallet::GenerateMnemonic("italian", "Data");
		REQUIRE(mnemonic != mnemonic2);
	}

	SECTION("generate chinese mnemonic") {
		std::string mnemonic = MasterWallet::GenerateMnemonic("chinese", "Data");
		std::string mnemonic2 = MasterWallet::GenerateMnemonic("chinese", "Data");
		REQUIRE(mnemonic != mnemonic2);
	}

	SECTION("generate english mnemonic") {
		std::string mnemonic = MasterWallet::GenerateMnemonic("english", "Data");
		std::string mnemonic2 = MasterWallet::GenerateMnemonic("english", "Data");
		REQUIRE(mnemonic != mnemonic2);
	}
}

TEST_CASE("Master wallet DestroyWallet method test", "[DestroyWallet]") {
	Log::registerMultiLogger();
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainID = "IDChain";

	boost::filesystem::remove_all(boost::filesystem::path(std::string("Data/") + MasterWalletTestID));
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

	SECTION("Normal destroy sub wallets") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainID);
		ISubWallet *subWallet1 = masterWallet->CreateSubWallet("ELA");

		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet));
		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet1));
	}
	SECTION("Destroy sub wallet multi-times") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainID);

		masterWallet->DestroyWallet(subWallet);
		std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
		REQUIRE(subWallets.size() == 0);
		REQUIRE_THROWS_AS(masterWallet->DestroyWallet(subWallet), std::invalid_argument);
	}
	SECTION("Destroy empty sub wallet") {
		REQUIRE_THROWS_AS(masterWallet->DestroyWallet(nullptr), std::invalid_argument);
	}
	SECTION("Destroy sub wallet that is not belong to current master wallet") {
		boost::scoped_ptr<TestMasterWallet> masterWallet1(new TestMasterWallet(phrasePassword, payPassword));
		ISubWallet *subWallet1 = masterWallet1->CreateSubWallet(chainID);
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainID);

		REQUIRE_THROWS(masterWallet->DestroyWallet(subWallet1));

		REQUIRE_NOTHROW(masterWallet1->DestroyWallet(subWallet1));
		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet));
	}
}

TEST_CASE("Master wallet ChangePassword method test", "[ChangePassword]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainID = "IDChain";

	boost::filesystem::remove_all(boost::filesystem::path(std::string("Data/") + MasterWalletTestID));
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

	SECTION("Check change with wrong pay password") {
		REQUIRE_THROWS_AS(masterWallet->ChangePassword("wrongPassword", "newPayPassword"), std::logic_error);
	}
	SECTION("Change with old pay password that is empty or less than 8") {
		CHECK_THROWS(masterWallet->ChangePassword("", "newPayPassword"));
		CHECK_THROWS(masterWallet->ChangePassword("ilegal", "newPayPassword"));
	}
	SECTION("Change with old pay password that is more than 128") {
		REQUIRE_THROWS(masterWallet->ChangePassword(
				"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
				"newPayPassword"));
	}
	SECTION("Change with new pay password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWallet->ChangePassword(payPassword, ""), std::invalid_argument);
		CHECK_THROWS_AS(masterWallet->ChangePassword(payPassword, "ilegal"), std::invalid_argument);
	}
	SECTION("Change with new pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWallet->ChangePassword(payPassword,
													   "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
	SECTION("Change password with phrase password not null") {
		boost::filesystem::remove_all(boost::filesystem::path(std::string("Data/") + MasterWalletTestID));
        boost::scoped_ptr<TestMasterWallet> masterWallet2(new TestMasterWallet("", payPassword));
        std::string newPayPassword = "newPayPassword";
        REQUIRE_NOTHROW(masterWallet->ChangePassword(payPassword, newPayPassword));
	}
}

TEST_CASE("Master wallet IsAddressValid method test", "[IsAddressValid]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	boost::filesystem::remove_all(boost::filesystem::path(std::string("Data/") + MasterWalletTestID));
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

	SECTION("Normal test") {
		REQUIRE(masterWallet->IsAddressValid("EZuWALdKM92U89NYAN5DDP5ynqMuyqG5i3")); //normal
		REQUIRE(masterWallet->IsAddressValid("im1NmKj6QKGmFToknsNP8cJyfCoU5sS26Y")); //id
		REQUIRE(masterWallet->IsAddressValid("XFjTcbZ9sN8CAmUhNTjf67AFFC3RBYoCRB")); //cross chain
		REQUIRE(masterWallet->IsAddressValid("8FQZdRrN8bSJuzSJh4im2teMqZoenmeJ4u")); //multi-sign
	}
	SECTION("Invalid id with not base58 character") {
		REQUIRE_FALSE(masterWallet->IsAddressValid("im1NmKj6QKGmFToknsNP8cJyfCoU5sS26I"));
	}
	SECTION("Invalid id with wrong length") {
		REQUIRE_FALSE(masterWallet->IsAddressValid("im1NmKj6QKGmFToknsNP8cJyfCoU5sS26"));
	}
	SECTION("Invalid id with wrong prefix") {
		REQUIRE_FALSE(masterWallet->IsAddressValid("Ym1NmKj6QKGmFToknsNP8cJyfCoU5sS26Y"));
	}
}

TEST_CASE("Master wallet GetSupportedChains method test", "[GetSupportedChains]") {
	//todo update me when CoinConfig.json file changed
	SECTION("Normal test") {
		boost::filesystem::remove_all(boost::filesystem::path(std::string("Data/") + MasterWalletTestID));
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet());
		std::vector<std::string> chainIDList = masterWallet->GetSupportedChains();
		REQUIRE(chainIDList.size() == 2);
	}
}

TEST_CASE("Master wallet GetAllSubWallets method test", "[GetAllSubWallets]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	SECTION("Test normal adding") {
		boost::filesystem::remove_all(boost::filesystem::path(std::string("Data/") + MasterWalletTestID));
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, phrasePassword, payPassword));

		REQUIRE(masterWallet->GetAllSubWallets().size() == 0);

		std::string masterWalletId = masterWallet->GetID();

		masterWallet->CreateSubWallet("ELA");
		REQUIRE(masterWallet->GetAllSubWallets().size() == 1);

		masterWallet->CreateSubWallet("IDChain");
		REQUIRE(masterWallet->GetAllSubWallets().size() == 2);
	}
}

TEST_CASE("Master wallet manager initFromKeyStore method", "[initFromKeyStore]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "MasterWalletId";

	boost::filesystem::remove_all(boost::filesystem::path(std::string("Data/") + MasterWalletTestID));
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, phrasePassword, payPassword));
	masterWallet->CreateSubWallet("ELA");
	masterWallet->CreateSubWallet("IDChain");

	std::string publicKey = masterWallet->GetxPubKey();

	nlohmann::json keystore = masterWallet->ExportKeystore("12345678", payPassword);

	TestMasterWallet masterWallet1(keystore, "12345678", payPassword);
	std::string publicKey1 = masterWallet1.GetxPubKey();

	REQUIRE(publicKey == publicKey1);
	REQUIRE(masterWallet->GetAllSubWallets().size() == 2);
}

TEST_CASE("Master wallet save and restore", "[Save&Restore]") {
	Log::registerMultiLogger();
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	SECTION("Import from master wallet store should load all sub wallets") {
		boost::filesystem::remove_all(boost::filesystem::path(std::string("Data/") + MasterWalletTestID));
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, phrasePassword, payPassword));
		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA");
		REQUIRE(subWallet != nullptr);
		REQUIRE(dynamic_cast<MainchainSubWallet *>(subWallet) != nullptr);
		subWallet = masterWallet->CreateSubWallet("IDChain");
		REQUIRE(subWallet != nullptr);
		REQUIRE(dynamic_cast<IDChainSubWallet *>(subWallet) != nullptr);

		std::string newPassword = "newPayPassword";
		masterWallet->ChangePassword(payPassword, newPassword);

		boost::filesystem::path localStore = "Data";
		localStore /= "MasterWalletTest";
		localStore /= "MasterWalletStore.json";
		masterWallet.reset(new TestMasterWallet("MasterWalletTest")); //save and reload in this line
		masterWallet->InitSubWallets();

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