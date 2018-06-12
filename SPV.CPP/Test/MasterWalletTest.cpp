// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>
#include <Interface/Enviroment.h>

#include "catch.hpp"

#include "MasterWallet.h"
#include "MainchainSubWallet.h"
#include "SidechainSubWallet.h"
#include "IdChainSubWallet.h"

using namespace Elastos::SDK;

class TestMasterWallet : public MasterWallet {
public:
	TestMasterWallet(const std::string &language) :
			MasterWallet("MasterWalletTest", language) {
	}

	TestMasterWallet(const std::string &phrasePassword,
					 const std::string &payPassword, const std::string language) :
			MasterWallet("MasterWalletTest", language) {
		std::string mnemonic = GenerateMnemonic();
		importFromMnemonic(mnemonic, phrasePassword, payPassword);
	}

	bool importFromMnemonicWraper(const std::string &mnemonic,
								  const std::string &phrasePassword,
								  const std::string &payPassword) {
		return importFromMnemonic(mnemonic, phrasePassword, payPassword);
	}

	ISubWallet *CreateSubWalletEx(
			const std::string &chainID,
			const std::string &payPassword,
			bool singleAddress,
			SubWalletType subWalletType,
			int coinTypeIndex,
			uint64_t feePerKb = 0) {
		CoinInfo info;
		info.setEaliestPeerTime(0);
		info.setWalletType(subWalletType);
		info.setIndex(coinTypeIndex);
		info.setSingleAddress(singleAddress);
		info.setUsedMaxAddressIndex(0);
		info.setChainId(chainID);
		info.setFeePerKb(feePerKb);
		info.setEncryptedKey("4c336d6e4d6b724b55676f666f6e716e464b79394a5532533657724b314350555955356f35577a4d6e4a5054715963526370734700");
		info.setChainCode("0000000000000000000000000000000000000000000000000000000000000000");
		std::vector<CoinInfo> coinInfoList = {info};
		restoreSubWallets(coinInfoList);

		return _createdWallets[chainID];
	}

protected:
	virtual void startPeerManager(SubWallet *wallet) {
	}

	virtual void stopPeerManager(SubWallet *wallet) {
	}
};

TEST_CASE("Master wallet constructor with language only", "[Constructor1]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	std::string chainId = "chainid";

	Enviroment::InitializeRootPath("Data");

	SECTION("Class public methods should throw when master wallet is not initialized") {
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(language));
		REQUIRE_FALSE(masterWallet->Initialized());

		CHECK_THROWS_AS(masterWallet->CreateSubWallet(chainId, payPassword, false), std::logic_error);
		CHECK_THROWS_AS(masterWallet->RecoverSubWallet(chainId, payPassword, false, 1), std::logic_error);
		CHECK_THROWS_AS(masterWallet->DestroyWallet(nullptr), std::logic_error);
		CHECK_THROWS_AS(masterWallet->GetPublicKey(), std::logic_error);
		CHECK_THROWS_AS(masterWallet->Sign("mymessage", payPassword), std::logic_error);
		CHECK_THROWS_AS(masterWallet->CheckSign("ilegal pubKey", "mymessage", "ilegal signature"), std::logic_error);
		std::string id;
		std::string key;
		CHECK_THROWS_AS(masterWallet->DeriveIdAndKeyForPurpose(44, 0, payPassword), std::logic_error);
	}
	SECTION("Class public methods will not throw after mater initialized by importFromMnemonic") {
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(language));
		REQUIRE_FALSE(masterWallet->Initialized());

		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		masterWallet->importFromMnemonicWraper(mnemonic, phrasePassword, payPassword);

		REQUIRE(masterWallet->Initialized());

		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, payPassword, false);
		REQUIRE(subWallet != nullptr);

		ISubWallet *subWallet1 = masterWallet->RecoverSubWallet(chainId, payPassword, false, 1);
		REQUIRE(subWallet1 != nullptr);

		std::string message = "mymessage";
		std::string signedMessage = masterWallet->Sign(message, payPassword);

		REQUIRE_FALSE(signedMessage.empty());

		nlohmann::json j = masterWallet->CheckSign(masterWallet->GetPublicKey(), message, signedMessage);
		REQUIRE(j["Result"].get<bool>());

		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet));
	}
	SECTION("Language should not be null") {
		CHECK_THROWS_AS(TestMasterWallet(""), std::invalid_argument);
	}
}

TEST_CASE("Master wallet constructor with phrase password and pay password", "[Constructor2]") {

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	std::string chainId = "chainid";

	Enviroment::InitializeRootPath("Data");

	SECTION("Class public methods behave well when construct with phrase password and pay password") {
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword, language));
		REQUIRE(masterWallet->Initialized());

		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, payPassword, false);
		REQUIRE(subWallet != nullptr);

		ISubWallet *subWallet1 = masterWallet->RecoverSubWallet("anotherid", payPassword, false, 1);
		REQUIRE(subWallet1 != nullptr);

		std::string message = "mymessage";
		std::string signedMessage = masterWallet->Sign(message, payPassword);
		REQUIRE_FALSE(signedMessage.empty());

		nlohmann::json j = masterWallet->CheckSign(masterWallet->GetPublicKey(), message, signedMessage);
		REQUIRE(j["Result"].get<bool>());

		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet));
		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet1));
	}
	SECTION("Master key and public key should generate randomly") {
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword, language));
		boost::scoped_ptr<TestMasterWallet> masterWallet1(new TestMasterWallet(phrasePassword, payPassword, language));

		REQUIRE(masterWallet->GetPublicKey() != masterWallet1->GetPublicKey());
	}
	SECTION("Create with phrase password can be empty") {
		CHECK_NOTHROW(TestMasterWallet("", payPassword, language));
	}
	SECTION("Create with phrase password that is less than 8") {
		CHECK_THROWS_AS(TestMasterWallet("ilegal", payPassword, language), std::invalid_argument);
	}
	SECTION("Create with phrase password that is more than 128") {
		REQUIRE_THROWS_AS(TestMasterWallet(
								  "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
										  payPassword, language), std::invalid_argument);
	}
	SECTION("Create with pay password that is empty or less than 8") {
		CHECK_THROWS_AS(TestMasterWallet(phrasePassword, "", language), std::invalid_argument);
		CHECK_THROWS_AS(TestMasterWallet(phrasePassword, "ilegal", language), std::invalid_argument);
	}
	SECTION("Create with pay password that is more than 128") {
		REQUIRE_THROWS_AS(TestMasterWallet(phrasePassword,
										   "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
										   language), std::invalid_argument);
	}
	SECTION("Language should not be null") {
		CHECK_THROWS_AS(TestMasterWallet(phrasePassword, payPassword, ""), std::invalid_argument);
	}
}

TEST_CASE("Master wallet CreateSubWallet method test", "[CreateSubWallet]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	std::string chainId = "chainid";

	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword, language));

	SECTION("Create normal sub wallet") {
		ISubWallet *subWallet = masterWallet->CreateSubWalletEx(chainId, payPassword, false, Normal, 0);

		SubWallet *normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
		REQUIRE(normalSubWallet != nullptr);

		MainchainSubWallet *mainchainSubWallet = dynamic_cast<MainchainSubWallet *>(subWallet);
		REQUIRE_FALSE(mainchainSubWallet != nullptr);

		SidechainSubWallet *sidechainSubWallet = dynamic_cast<SidechainSubWallet *>(subWallet);
		REQUIRE_FALSE(sidechainSubWallet != nullptr);

		IdChainSubWallet *idChainSubWallet = dynamic_cast<IdChainSubWallet *>(subWallet);
		REQUIRE_FALSE(idChainSubWallet != nullptr);

		masterWallet->DestroyWallet(subWallet);
	}
	SECTION("Create mainchain sub wallet") {
		ISubWallet *subWallet = masterWallet->CreateSubWalletEx(chainId, payPassword, false, Mainchain, 0);

		SubWallet *normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
		REQUIRE(normalSubWallet != nullptr);

		MainchainSubWallet *mainchainSubWallet = dynamic_cast<MainchainSubWallet *>(subWallet);
		REQUIRE(mainchainSubWallet != nullptr);

		masterWallet->DestroyWallet(subWallet);
	}
	SECTION("Create mainchain sub wallet") {
		ISubWallet *subWallet = masterWallet->CreateSubWalletEx(chainId, payPassword, false, Sidechain, 0);

		SubWallet *normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
		REQUIRE(normalSubWallet != nullptr);

		SidechainSubWallet *sidechainSubWallet = dynamic_cast<SidechainSubWallet *>(subWallet);
		REQUIRE(sidechainSubWallet != nullptr);

		masterWallet->DestroyWallet(subWallet);
	}
	SECTION("Create idchain sub wallet") {
		ISubWallet *subWallet = masterWallet->CreateSubWalletEx(chainId, payPassword, false, Idchain, 0);

		SubWallet *normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
		REQUIRE(normalSubWallet != nullptr);

		IdChainSubWallet *idChainSubWallet = dynamic_cast<IdChainSubWallet *>(subWallet);
		REQUIRE(idChainSubWallet != nullptr);

		masterWallet->DestroyWallet(subWallet);
	}
	SECTION("Return exist sub wallet with same id") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, payPassword, false);
		REQUIRE(subWallet != nullptr);

		//Return same sub wallet with same chain id
		ISubWallet *subWallet1 = masterWallet->CreateSubWallet(chainId, payPassword, false);
		REQUIRE(subWallet == subWallet1);

		//Throw param exception when chain id is exist
		REQUIRE_THROWS_AS(masterWallet->CreateSubWallet(chainId, "other password", true),
						  std::invalid_argument);

		//Create another sub wallet
		ISubWallet *subWallet3 = masterWallet->CreateSubWallet("chain2", payPassword, false);
		REQUIRE(subWallet3 != nullptr);
		REQUIRE(subWallet != subWallet3);

		masterWallet->DestroyWallet(subWallet);
		masterWallet->DestroyWallet(subWallet3);
	}
	SECTION("Create sub wallet with empty chain id") {
		REQUIRE_THROWS_AS(masterWallet->CreateSubWallet("", payPassword, false), std::invalid_argument);
	}
	SECTION("Create sub wallet with chain id that is more than 128") {
		REQUIRE_THROWS_AS(masterWallet->CreateSubWallet(
				"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
				payPassword, false), std::invalid_argument);
	}
	SECTION("Create sub wallet with pay password that is empty or less than 8") {
		REQUIRE_THROWS_AS(masterWallet->CreateSubWallet(chainId, "", false), std::invalid_argument);
		REQUIRE_THROWS_AS(masterWallet->CreateSubWallet(chainId, "invalid", false), std::invalid_argument);
	}
	SECTION("Create sub wallet with pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWallet->CreateSubWallet(chainId,
														"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
														false), std::invalid_argument);
	}
}

TEST_CASE("Master wallet RecoverSubWallet method test", "[RecoverSubWallet]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	std::string chainId = "chainid";

	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword, language));

	SECTION("Return exist sub wallet with same id") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, payPassword, false);
		REQUIRE(subWallet != nullptr);

		//Return same sub wallet with same chain id
		ISubWallet *subWallet1 = masterWallet->RecoverSubWallet(chainId, payPassword, false, 1);
		REQUIRE(subWallet == subWallet1);

		//Return same sub wallet even if parameter of others are different
		ISubWallet *subWallet2 = masterWallet->RecoverSubWallet(chainId, "other password", true, 1);
		REQUIRE(subWallet == subWallet2);

		//Create another sub wallet
		ISubWallet *subWallet3 = masterWallet->RecoverSubWallet("chain2", payPassword, false, 1);
		REQUIRE(subWallet3 != nullptr);
		REQUIRE(subWallet != subWallet3);

		masterWallet->DestroyWallet(subWallet);
		masterWallet->DestroyWallet(subWallet3);
	}
	SECTION("Limit gap should less than or equal 10") {
		REQUIRE_THROWS_AS(masterWallet->RecoverSubWallet(chainId, payPassword, false, 11),
						  std::invalid_argument);
	}
	SECTION("Recover wallet if not exist") {
		//todo complete me
	}
}

TEST_CASE("Master wallet DestroyWallet method test", "[DestroyWallet]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	std::string chainId = "chainid";

	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword, language));

	SECTION("Normal destroy sub wallets") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, payPassword, false);
		ISubWallet *subWallet1 = masterWallet->CreateSubWallet("anotherId", payPassword, false);

		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet));
		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet1));
	}
	SECTION("Destroy sub wallet multi-times") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, payPassword, false);

		masterWallet->DestroyWallet(subWallet);
		try {
			masterWallet->DestroyWallet(subWallet);
		}
		catch (const std::logic_error &e) {
			REQUIRE(strcmp(e.what(), "There is no sub wallet in this wallet.") == 0);
		}
	}
	SECTION("Destroy empty sub wallet") {
		REQUIRE_THROWS_AS(masterWallet->DestroyWallet(nullptr), std::invalid_argument);
	}
	SECTION("Destroy sub wallet that is not belong to current master wallet") {
		boost::scoped_ptr<TestMasterWallet> masterWallet1(new TestMasterWallet(phrasePassword, payPassword, language));
		ISubWallet *subWallet1 = masterWallet1->CreateSubWallet(chainId, payPassword, false);
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, payPassword, false);

		try {
			masterWallet->DestroyWallet(subWallet1);
		}
		catch (const std::logic_error &e) {
			REQUIRE(strcmp(e.what(), "Specified sub wallet is not belong to current master wallet.") == 0);
		}

		REQUIRE_NOTHROW(masterWallet1->DestroyWallet(subWallet1));
		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet));
	}
}

TEST_CASE("Master wallet Sign method test", "[Sign]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	std::string chainId = "chainid";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword, language));

	SECTION("Normal sign") {
		REQUIRE_FALSE(masterWallet->Sign("mymessage", payPassword).empty());
	}
	SECTION("Sign empty message") {
		REQUIRE_THROWS_AS(masterWallet->Sign("", payPassword), std::invalid_argument);
	}
	SECTION("Sign with pay password that is empty or less than 8") {
		REQUIRE_THROWS_AS(masterWallet->Sign("mymessage", ""), std::invalid_argument);
		REQUIRE_THROWS_AS(masterWallet->Sign("mymessage", "invalid"), std::invalid_argument);
	}
	SECTION("Sign with pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWallet->Sign("mymessage",
											 "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
	SECTION("Sign with wrong password") {
		REQUIRE_THROWS_AS(masterWallet->Sign("mymessage", "wrongpassword"), std::logic_error);
	}
}

TEST_CASE("Master wallet CheckSign method test", "[CheckSign]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	std::string chainId = "chainid";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword, language));
	std::string message = "mymessage";
	std::string signedData = masterWallet->Sign(message, payPassword);

	SECTION("Normal check sign") {
		nlohmann::json j = masterWallet->CheckSign(masterWallet->GetPublicKey(), message, signedData);
		REQUIRE(j["Result"].get<bool>());
	}
	SECTION("Check sign with wrong message") {
		nlohmann::json j = masterWallet->CheckSign(masterWallet->GetPublicKey(), "wrongMessage", signedData);
		REQUIRE_FALSE(j["Result"].get<bool>());
	}
	SECTION("Check sign with wrong signed data") {
		nlohmann::json j = masterWallet->CheckSign(masterWallet->GetPublicKey(), message, "wrangData");
		REQUIRE_FALSE(j["Result"].get<bool>());
	}
}

TEST_CASE("Master wallet DeriveIdAndKeyForPurpose method test", "[DeriveIdAndKeyForPurpose]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(language));
	std::string mnemonic = "you train view salon cancel impulse phrase oxygen sport crack peasant observe";
	masterWallet->importFromMnemonicWraper(mnemonic, phrasePassword, payPassword);

	std::string id;
	std::string key;

	SECTION("Normal derive") {
		id = masterWallet->DeriveIdAndKeyForPurpose(1, 1, payPassword);
		REQUIRE(id == "im1NmKj6QKGmFToknsNP8cJyfCoU5sS26Y");
	}
	SECTION("Derive reserved purpose") {
		try {
			id = masterWallet->DeriveIdAndKeyForPurpose(44, 1, payPassword);
		}
		catch (const std::invalid_argument &e) {
			REQUIRE(strcmp(e.what(), "Can not use reserved purpose.") == 0);
		}
		//todo add other reserved purpose test in future
	}
	SECTION("Derive with pay password that is empty or less than 8") {
		REQUIRE_THROWS_AS(masterWallet->DeriveIdAndKeyForPurpose(1, 1, ""), std::invalid_argument);
		REQUIRE_THROWS_AS(masterWallet->DeriveIdAndKeyForPurpose(1, 1, "invalid"), std::invalid_argument);
	}
	SECTION("Derive with pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWallet->DeriveIdAndKeyForPurpose(1, 1,
																 "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
	SECTION("Derive by wrong password") {
		REQUIRE_THROWS_AS(masterWallet->DeriveIdAndKeyForPurpose(1, 1, "wrongPassword"), std::logic_error);
	}
}

TEST_CASE("Master wallet GetPublicKey method test", "[GetPublicKey]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";

	SECTION("Normal test") {
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(language));

		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		masterWallet->importFromMnemonicWraper(mnemonic, phrasePassword, payPassword);

		REQUIRE(masterWallet->GetPublicKey() == "02f7fc47a94041b776eed8894cfe90bb824297bd3e957fe10d1f408b25b5046bac");
	}
}

TEST_CASE("Master wallet IsIdValid method test", "[IsIdValid]") {
	//todo complete me
}

TEST_CASE("Master wallet GetSupportedChains method test", "[GetSupportedChains]") {
	//todo update me when CoinConfig.json file changed

	Enviroment::InitializeRootPath("Data");
	std::string language = "english";
	SECTION("Normal test") {
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(language));
		std::vector<std::string> chainIdList = masterWallet->GetSupportedChains();
		REQUIRE(chainIdList.size() == 2);
	}
}