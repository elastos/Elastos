// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>

#include <catch.hpp>

#include <Implement/MasterWallet.h>
#include <Implement/MainchainSubWallet.h>
#include <Implement/SidechainSubWallet.h>
#include <Implement/IDChainSubWallet.h>
#include <Plugin/Registry.h>
#include <Plugin/ELAPlugin.h>
#include <Plugin/IDPlugin.h>
#include <Plugin/TokenPlugin.h>
#include <SpvService/Config.h>

using namespace Elastos::ElaWallet;

static const std::string rootPath = ".";
#define MasterWalletTestID "MasterWalletID"
#define PASSPHRASE         "passphrase"
#define PAY_PASSWORD       "payPassword"

class TestMasterWallet : public MasterWallet {
public:
	TestMasterWallet(const ConfigPtr &config) :

		MasterWallet(MasterWalletTestID,
					 "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
					 "phrasePassword",
					 "payPassword",
					 false,
					 false,
					 config,
					 rootPath,
					 0,
					 ImportFromMnemonic) {
	}

	explicit TestMasterWallet(const std::string &passphrase,
							  const std::string &payPasswd,
							  const ConfigPtr &config) :
		MasterWallet(MasterWalletTestID,
					 "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
					 passphrase,
					 payPasswd,
					 false,
					 false,
					 config,
					 rootPath,
					 0,
					 ImportFromMnemonic) {
	}

	TestMasterWallet(const std::string &mnemonic,
					 const std::string &passphrase,
					 const std::string &payPasswd,
					 const ConfigPtr &config) :
		MasterWallet(MasterWalletTestID, mnemonic, passphrase, payPasswd, false, false, config, rootPath, 0,
					 ImportFromMnemonic) {
	}

	TestMasterWallet(const std::string &mnemonic,
					 const std::string &passphrase,
					 const std::string &payPasswd,
					 bool singleAddress,
					 const ConfigPtr &config) :
		MasterWallet(MasterWalletTestID, mnemonic, passphrase, payPasswd, singleAddress, false, config, rootPath, 0,
					 ImportFromMnemonic) {
	}

	TestMasterWallet(const std::string &id, const ConfigPtr &config) :
		MasterWallet(id, config, rootPath, false, ImportFromMnemonic) {
	}

	TestMasterWallet(const nlohmann::json &keystore, const std::string &backupPassword, const std::string &payPasswd,
					 const ConfigPtr &config) :
		MasterWallet(MasterWalletTestID, keystore, backupPassword, payPasswd, config, rootPath, false,
					 ImportFromKeyStore) {
	}

	std::string GetxPubKey() {
		return _account->MasterPubKeyString();
	}


};

TEST_CASE("Master wallet CreateSubWallet method test", "[CreateSubWallet]") {
	Log::registerMultiLogger();

#ifdef SPV_ENABLE_STATIC
	Log::info("Registering plugin ...");
	REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
	REGISTER_MERKLEBLOCKPLUGIN(IDChain, getIDPluginComponent);
	REGISTER_MERKLEBLOCKPLUGIN(TokenChain, getTokenPluginComponent);
#endif

	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
	ConfigPtr config(new Config(rootPath, CONFIG_MAINNET));
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(PASSPHRASE, PAY_PASSWORD, config));

	ISubWallet *subWallet = masterWallet->CreateSubWallet(CHAINID_MAINCHAIN);
	SubWallet *normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
	REQUIRE(normalSubWallet != nullptr);
	MainchainSubWallet *mainchainSubWallet = dynamic_cast<MainchainSubWallet *>(subWallet);
	REQUIRE(mainchainSubWallet != nullptr);
	SidechainSubWallet *sidechainSubWallet = dynamic_cast<SidechainSubWallet *>(subWallet);
	REQUIRE(sidechainSubWallet == nullptr);
	REQUIRE_NOTHROW(masterWallet->DestroyWallet(CHAINID_MAINCHAIN));

	subWallet = masterWallet->CreateSubWallet(CHAINID_IDCHAIN);
	normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
	REQUIRE(normalSubWallet != nullptr);
	IDChainSubWallet *idChainSubWallet = dynamic_cast<IDChainSubWallet *>(subWallet);
	REQUIRE(idChainSubWallet != nullptr);
	mainchainSubWallet = dynamic_cast<MainchainSubWallet *>(subWallet);
	REQUIRE(mainchainSubWallet == nullptr);
	REQUIRE_NOTHROW(masterWallet->DestroyWallet(CHAINID_IDCHAIN));
	REQUIRE_THROWS(masterWallet->DestroyWallet(CHAINID_IDCHAIN));

	std::vector<std::string> chainIDs = masterWallet->GetSupportedChains();
	for (int i = 0; i < chainIDs.size(); ++i) {
		if (chainIDs[i] == CHAINID_ESC)
			continue;
		subWallet = masterWallet->CreateSubWallet(chainIDs[i]);
		REQUIRE(subWallet != nullptr);
	}

	// Return exist sub wallet with same id
	subWallet = masterWallet->CreateSubWallet(CHAINID_IDCHAIN);
	REQUIRE(subWallet != nullptr);
	//Return same sub wallet with same chain id
	ISubWallet *subWallet1 = masterWallet->CreateSubWallet(CHAINID_IDCHAIN);
	REQUIRE(subWallet == subWallet1);
	ISubWallet *subWallet2 = masterWallet->CreateSubWallet(CHAINID_MAINCHAIN);
	REQUIRE(subWallet2 != nullptr);
	REQUIRE(subWallet != subWallet2);
	REQUIRE_NOTHROW(masterWallet->DestroyWallet(CHAINID_IDCHAIN));
	REQUIRE_NOTHROW(masterWallet->DestroyWallet(CHAINID_MAINCHAIN));

	REQUIRE_THROWS(masterWallet->CreateSubWallet(""));
	REQUIRE_THROWS(masterWallet->CreateSubWallet(
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"));
	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
}

TEST_CASE("Master wallet ChangePassword method test", "[ChangePassword]") {
	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
	ConfigPtr config(new Config(rootPath, CONFIG_MAINNET));
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(PASSPHRASE, PAY_PASSWORD, config));

	REQUIRE_THROWS(masterWallet->ChangePassword("wrongPassword", "newPayPassword"));
	CHECK_THROWS(masterWallet->ChangePassword("", "newPayPassword"));
	CHECK_THROWS(masterWallet->ChangePassword("ilegal", "newPayPassword"));
	REQUIRE_THROWS(masterWallet->ChangePassword(
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"newPayPassword"));
	REQUIRE_THROWS(masterWallet->ChangePassword(PAY_PASSWORD, ""));
	REQUIRE_THROWS(masterWallet->ChangePassword(PAY_PASSWORD, "ilegal"));
	REQUIRE_THROWS(masterWallet->ChangePassword(PAY_PASSWORD,
												"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"));

	std::string newPayPassword = "newPayPassword";
	REQUIRE_NOTHROW(masterWallet->ChangePassword(PAY_PASSWORD, newPayPassword));
	REQUIRE_NOTHROW(masterWallet->ExportMnemonic(newPayPassword));
	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
}

TEST_CASE("Master wallet IsAddressValid method test", "[IsAddressValid]") {
	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
	ConfigPtr config(new Config(rootPath, CONFIG_MAINNET));
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(PASSPHRASE, PAY_PASSWORD, config));

	REQUIRE(masterWallet->IsAddressValid("EZuWALdKM92U89NYAN5DDP5ynqMuyqG5i3")); //normal
	REQUIRE(masterWallet->IsAddressValid("im1NmKj6QKGmFToknsNP8cJyfCoU5sS26Y")); //id
	REQUIRE(masterWallet->IsAddressValid("XFjTcbZ9sN8CAmUhNTjf67AFFC3RBYoCRB")); //cross chain
	REQUIRE(masterWallet->IsAddressValid("8FQZdRrN8bSJuzSJh4im2teMqZoenmeJ4u")); //multi-sign
	REQUIRE_FALSE(masterWallet->IsAddressValid("im1NmKj6QKGmFToknsNP8cJyfCoU5sS26I"));
	REQUIRE_FALSE(masterWallet->IsAddressValid("im1NmKj6QKGmFToknsNP8cJyfCoU5sS26"));
	REQUIRE_FALSE(masterWallet->IsAddressValid("Ym1NmKj6QKGmFToknsNP8cJyfCoU5sS26Y"));

	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
}

TEST_CASE("Master wallet GetAllSubWallets method test", "[GetAllSubWallets]") {
	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
	ConfigPtr config(new Config(rootPath, CONFIG_MAINNET));
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, PASSPHRASE, PAY_PASSWORD, config));

	REQUIRE(masterWallet->GetAllSubWallets().size() == 0);
	REQUIRE(MasterWalletTestID == masterWallet->GetID());
	masterWallet->CreateSubWallet(CHAINID_MAINCHAIN);
	REQUIRE(masterWallet->GetAllSubWallets().size() == 1);
	masterWallet->CreateSubWallet(CHAINID_IDCHAIN);
	REQUIRE(masterWallet->GetAllSubWallets().size() == 2);

	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
}

TEST_CASE("Master wallet manager initFromKeyStore method", "[initFromKeyStore]") {
	Log::registerMultiLogger();

	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
	ConfigPtr config(new Config(rootPath, CONFIG_MAINNET));

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	boost::shared_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, PASSPHRASE, PAY_PASSWORD, config));
	masterWallet->CreateSubWallet(CHAINID_MAINCHAIN);
	masterWallet->CreateSubWallet(CHAINID_IDCHAIN);
	REQUIRE(masterWallet->GetAllSubWallets().size() == 2);

	std::string publicKey = masterWallet->GetxPubKey();

	std::string backupPasswd = "12345678";
	nlohmann::json keystore = masterWallet->ExportKeystore(backupPasswd, PAY_PASSWORD);
	TestMasterWallet masterWallet1(keystore, backupPasswd, PAY_PASSWORD, config);
	masterWallet1.InitSubWallets();
	std::string publicKey1 = masterWallet1.GetxPubKey();
	REQUIRE(publicKey == publicKey1);
	REQUIRE(masterWallet1.GetAllSubWallets().size() == 2);
	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
}

TEST_CASE("Master wallet save and restore", "[Save&Restore]") {
	Log::registerMultiLogger();

	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
	ConfigPtr config(new Config(rootPath, CONFIG_MAINNET));

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	boost::shared_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, PASSPHRASE, PAY_PASSWORD, config));
	ISubWallet *subWallet = masterWallet->CreateSubWallet(CHAINID_MAINCHAIN);
	REQUIRE(subWallet != nullptr);
	REQUIRE(dynamic_cast<MainchainSubWallet *>(subWallet) != nullptr);
	subWallet = masterWallet->CreateSubWallet(CHAINID_IDCHAIN);
	REQUIRE(subWallet != nullptr);
	REQUIRE(dynamic_cast<IDChainSubWallet *>(subWallet) != nullptr);

	std::string newPassword = "newPayPassword";
	masterWallet->ChangePassword(PAY_PASSWORD, newPassword);

	masterWallet.reset(new TestMasterWallet(MasterWalletTestID, config)); //save and reload in this line
	masterWallet->InitSubWallets();

	std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
	REQUIRE(subWallets.size() == 2);
	REQUIRE(subWallets[0] != nullptr);
	REQUIRE(subWallets[1] != nullptr);
	for (int i = 0; i < subWallets.size(); ++i) {
		if (subWallets[i]->GetChainID() == CHAINID_MAINCHAIN)
			REQUIRE(dynamic_cast<MainchainSubWallet *>(subWallets[i]) != nullptr);
		else if (subWallets[i]->GetChainID() == CHAINID_IDCHAIN)
			REQUIRE(dynamic_cast<IDChainSubWallet *>(subWallets[i]) != nullptr);
	}
	boost::filesystem::remove_all(boost::filesystem::path(rootPath + "/" + MasterWalletTestID));
}