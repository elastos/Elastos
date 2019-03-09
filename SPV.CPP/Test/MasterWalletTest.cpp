// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>

#include "catch.hpp"

#include <SDK/Implement/MasterWallet.h>
#include <SDK/Implement/MainchainSubWallet.h>
#include <SDK/Implement/SidechainSubWallet.h>
#include <SDK/Implement/IdChainSubWallet.h>
#include <SDK/Common/Utils.h>

using namespace Elastos::ElaWallet;

class TestMasterWallet : public MasterWallet {
public:
	TestMasterWallet() :

		MasterWallet("MasterWalletTest",
					 "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
					 "phrasePassword",
					 "payPassword",
					 false,
					 false,
					 "Data",
					 ImportFromMnemonic) {
	}

	TestMasterWallet(const std::string &phrasePassword,
					 const std::string &payPassword) :
			MasterWallet("MasterWalletTest",
						 "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
						 phrasePassword,
						 payPassword,
						 false,
						 false,
						 "Data",
						 ImportFromMnemonic) {
	}

	TestMasterWallet(const std::string &mnemonic,
					 const std::string &phrasePassword,
					 const std::string &payPassword) :
			MasterWallet("MasterWalletTest", mnemonic, phrasePassword, payPassword, false, false, "Data", ImportFromMnemonic) {
	}

	TestMasterWallet(const std::string &mnemonic,
					 const std::string &phrasePassword,
					 const std::string &payPassword,
					 bool singleAddress) :
			MasterWallet("MasterWalletTest", mnemonic, phrasePassword, payPassword, singleAddress, false, "Data", ImportFromMnemonic) {
	}

	TestMasterWallet(const boost::filesystem::path &localStore) :
			MasterWallet(localStore, "Data", false, ImportFromMnemonic) {
	}

	void restoreLocalStoreWrapper() {
		restoreLocalStore();
	}

	const MasterWalletStore &getMasterWalletStore() { return _localStore; }

	void initFromKeyStoreWrapper(const KeyStore &keyStore, const std::string &payPassword) {
		initFromKeyStore(keyStore, payPassword);
	}

	void restoreKeyStoreWrapper(KeyStore &keyStore, const std::string &payPassword) {
		restoreKeyStore(keyStore, payPassword);
	}

	std::string getEncryptedPhrasePassword() {
		return _localStore.Account()->GetEncryptedPhrasePassword();
	}

	ISubWallet *CreateSubWalletEx(
			const std::string &chainID,
			bool singleAddress,
			SubWalletType subWalletType,
			int coinTypeIndex,
			uint64_t feePerKb = 0) {

		tryInitCoinConfig();
		CoinInfo info;
		info.SetEaliestPeerTime(0);
		info.SetWalletType(subWalletType);
		info.SetIndex(coinTypeIndex);
		info.SetSingleAddress(singleAddress);
		info.SetUsedMaxAddressIndex(0);
		info.SetChainId(chainID);
		info.SetFeePerKb(feePerKb);
		info.SetChainCode("0000000000000000000000000000000000000000000000000000000000000000");
		info.SetPublicKey("038bc7fbfa77b10b85cdf9ef75c0d994924adab73a7a7486e9c392398899faac33");
		std::vector<CoinInfo> coinInfoList = {info};
		restoreSubWallets(coinInfoList);

		return _createdWallets[chainID];
	}
};

TEST_CASE("Master wallet constructor with language only", "[Constructor1]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainId = "IdChain";
	uint64_t feePerKB = 10000;

	SECTION("Class public methods will not throw after mater initialized by importFromMnemonic") {
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

		//override from IMasterWallet
		ISubWallet *subWallet = nullptr;
		REQUIRE_NOTHROW(subWallet = masterWallet->CreateSubWallet(chainId, feePerKB));
		REQUIRE(subWallet != nullptr);

		std::string message = "mymessage";
		std::string signedMessage = masterWallet->Sign(message, payPassword);
		REQUIRE_FALSE(signedMessage.empty());
		REQUIRE(masterWallet->CheckSign(masterWallet->GetPublicKey(), message, signedMessage));

		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet));
	}
}

TEST_CASE("Master wallet constructor with phrase password and pay password", "[Constructor2]") {

	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainId = "IdChain";
	uint64_t feePerKB = 10000;

	SECTION("Class public methods behave well when construct with phrase password and pay password") {
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, feePerKB);
		REQUIRE(subWallet != nullptr);

		std::string message = "mymessage";
		std::string signedMessage = masterWallet->Sign(message, payPassword);
		REQUIRE_FALSE(signedMessage.empty());
		REQUIRE(masterWallet->CheckSign(masterWallet->GetPublicKey(), message, signedMessage));

		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet));
	}
}

TEST_CASE("Master wallet CreateSubWallet method test", "[CreateSubWallet]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string language = "english";
	std::string chainId = "IdChain";
	uint64_t feePerKB = 10000;

	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

	SECTION("Create normal sub wallet") {
		ISubWallet *subWallet = masterWallet->CreateSubWalletEx(chainId, false, Normal, 0);

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
		ISubWallet *subWallet = masterWallet->CreateSubWalletEx(chainId, false, Mainchain, 0);

		SubWallet *normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
		REQUIRE(normalSubWallet != nullptr);

		MainchainSubWallet *mainchainSubWallet = dynamic_cast<MainchainSubWallet *>(subWallet);
		REQUIRE(mainchainSubWallet != nullptr);

		masterWallet->DestroyWallet(subWallet);
	}
	SECTION("Create mainchain sub wallet") {
		ISubWallet *subWallet = masterWallet->CreateSubWalletEx(chainId, false, Sidechain, 0);

		SubWallet *normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
		REQUIRE(normalSubWallet != nullptr);

		SidechainSubWallet *sidechainSubWallet = dynamic_cast<SidechainSubWallet *>(subWallet);
		REQUIRE(sidechainSubWallet != nullptr);

		masterWallet->DestroyWallet(subWallet);
	}
	SECTION("Create idchain sub wallet") {
		ISubWallet *subWallet = masterWallet->CreateSubWalletEx(chainId, false, Idchain, 0);

		SubWallet *normalSubWallet = dynamic_cast<SubWallet *>(subWallet);
		REQUIRE(normalSubWallet != nullptr);

		IdChainSubWallet *idChainSubWallet = dynamic_cast<IdChainSubWallet *>(subWallet);
		REQUIRE(idChainSubWallet != nullptr);

		masterWallet->DestroyWallet(subWallet);
	}
	SECTION("Create all sub wallets in list") {
		std::vector<std::string> chainIds = masterWallet->GetSupportedChains();
		for (int i = 0; i < chainIds.size(); ++i) {
			ISubWallet *subWallet = masterWallet->CreateSubWallet(chainIds[i], feePerKB);
			REQUIRE(subWallet != nullptr);
		}
	}
	SECTION("Return exist sub wallet with same id") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, feePerKB);
		REQUIRE(subWallet != nullptr);

		//Return same sub wallet with same chain id
		ISubWallet *subWallet1 = masterWallet->CreateSubWallet(chainId, feePerKB);
		REQUIRE(subWallet == subWallet1);

		//Throw param exception when chain id is exist
		REQUIRE_NOTHROW(masterWallet->CreateSubWallet(chainId, feePerKB));

		//Create another sub wallet
		ISubWallet *subWallet3 = masterWallet->CreateSubWallet("ELA", feePerKB);
		REQUIRE(subWallet3 != nullptr);
		REQUIRE(subWallet != subWallet3);

		masterWallet->DestroyWallet(subWallet);
		masterWallet->DestroyWallet(subWallet3);
	}
	SECTION("Create sub wallet with empty chain id") {
		REQUIRE_THROWS_AS(masterWallet->CreateSubWallet("", feePerKB), std::invalid_argument);
	}
	SECTION("Create sub wallet with chain id that is more than 128") {
		REQUIRE_THROWS_AS(masterWallet->CreateSubWallet(
				"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
				feePerKB), std::invalid_argument);
	}
}

TEST_CASE("Master wallet GenerateMnemonic method test", "[GenerateMnemonic]") {
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
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainId = "IdChain";
	uint64_t feePerKB = 10000;

	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

	SECTION("Normal destroy sub wallets") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, feePerKB);
		ISubWallet *subWallet1 = masterWallet->CreateSubWallet("ELA", feePerKB);

		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet));
		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet1));
	}
	SECTION("Destroy sub wallet multi-times") {
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, feePerKB);

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
		ISubWallet *subWallet1 = masterWallet1->CreateSubWallet(chainId, feePerKB);
		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, feePerKB);

		REQUIRE_THROWS_AS(masterWallet->DestroyWallet(subWallet1), std::logic_error);

		REQUIRE_NOTHROW(masterWallet1->DestroyWallet(subWallet1));
		REQUIRE_NOTHROW(masterWallet->DestroyWallet(subWallet));
	}
}

TEST_CASE("Master wallet Sign method test", "[Sign]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainId = "IdChain";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

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

TEST_CASE("Master wallet ChangePassword method test", "[ChangePassword]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainId = "IdChain";
	uint64_t feePerKB = 10000;
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

	SECTION("Normal change") {
		REQUIRE_FALSE(masterWallet->Sign("mymessage", payPassword).empty());

		std::string newPayPassword = "newPayPassword";
		masterWallet->ChangePassword(payPassword, newPayPassword);

		REQUIRE_THROWS_AS(masterWallet->Sign("mymessage", payPassword), std::logic_error);
		REQUIRE_FALSE(masterWallet->Sign("mymessage", newPayPassword).empty());
	}
	SECTION("Check change with wrong pay password") {
		REQUIRE_THROWS_AS(masterWallet->ChangePassword("wrongPassword", "newPayPassword"), std::logic_error);
	}
	SECTION("Change with old pay password that is empty or less than 8") {
		CHECK_THROWS_AS(masterWallet->ChangePassword("", "newPayPassword"), std::invalid_argument);
		CHECK_THROWS_AS(masterWallet->ChangePassword("ilegal", "newPayPassword"), std::invalid_argument);
	}
	SECTION("Change with old pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWallet->ChangePassword(
				"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
				"newPayPassword"), std::invalid_argument);
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
        boost::scoped_ptr<TestMasterWallet> masterWallet2(new TestMasterWallet("", payPassword));
        std::string newPayPassword = "newPayPassword";
        REQUIRE_NOTHROW(masterWallet->ChangePassword(payPassword, newPayPassword));
	}
	SECTION("Change password should be effective for sub wallets") {
		masterWallet->CreateSubWallet("ELA", feePerKB);
		std::string newPayPassword = "newPayPassword";

		REQUIRE_NOTHROW(masterWallet->GetAllSubWallets()[0]->Sign("MyMessage", payPassword));
		REQUIRE_THROWS(masterWallet->GetAllSubWallets()[0]->Sign("MyMessage", newPayPassword));

		masterWallet->ChangePassword(payPassword, newPayPassword);

		REQUIRE_THROWS(masterWallet->GetAllSubWallets()[0]->Sign("MyMessage", payPassword));
		REQUIRE_NOTHROW(masterWallet->GetAllSubWallets()[0]->Sign("MyMessage", newPayPassword));
	}
}

TEST_CASE("Master wallet CheckSign method test", "[CheckSign]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainId = "ELA";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));
	std::string message = "mymessage";
	std::string signedData = masterWallet->Sign(message, payPassword);
	SECTION("Normal check sign") {
		REQUIRE(masterWallet->CheckSign(masterWallet->GetPublicKey(), message, signedData));
	}
	SECTION("Check sign with wrong message") {
		REQUIRE_FALSE(masterWallet->CheckSign(masterWallet->GetPublicKey(), "wrongMessage", signedData));
	}
	SECTION("Check sign with wrong signed data") {
		REQUIRE_FALSE(masterWallet->CheckSign(masterWallet->GetPublicKey(), message, "wrangData"));
	}
}

TEST_CASE("Master wallet GetPublicKey method test", "[GetPublicKey]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

	SECTION("Normal test") {
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(phrasePassword, payPassword));

		REQUIRE(masterWallet->GetPublicKey() == "02dbac277ae44dc791320d571341e55177ebe6df6d2e55badb0cbbc31e6f179230");
	}
}

TEST_CASE("Master wallet IsAddressValid method test", "[IsAddressValid]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";

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

TEST_CASE("Master wallet DeriveIdAndKeyForPurpose method test", "[DeriveIdAndKeyForPurpose]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "you train view salon cancel impulse phrase oxygen sport crack peasant observe";
	std::string language = "english";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(
			mnemonic,
			phrasePassword,
			payPassword,
			false));

	std::string id;

	SECTION("Normal derive") {
		id = masterWallet->DeriveIdAndKeyForPurpose(1, 1);
		REQUIRE(id == "inht4LxCUCUi5N8dWfk2NYPJB6uHzFrrin");
		std::string id2 = masterWallet->DeriveIdAndKeyForPurpose(1, 2);
		REQUIRE(id != id2);
	}
	SECTION("Derive reserved purpose") {
		REQUIRE_THROWS_AS(id = masterWallet->DeriveIdAndKeyForPurpose(44, 1), std::invalid_argument);
		//todo add other reserved purpose test in future
	}
}

TEST_CASE("Master wallet GetPublicKey method of id agent", "[GetPublicKey-IdAgent]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "you train view salon cancel impulse phrase oxygen sport crack peasant observe";
	bool singleAddress = false;
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, phrasePassword, payPassword, singleAddress));

	std::string id;

	SECTION("Normal get") {
		id = masterWallet->DeriveIdAndKeyForPurpose(1, 1);
		std::string pubKey = masterWallet->GetPublicKey(id);
		REQUIRE(pubKey == "0266d9b14a1f20b723f2ef5f2b6edcec6fe8681cfd1dc4f1a42486d3164d023833");
	}
	SECTION("Should throw with wrong id") {
		id = masterWallet->DeriveIdAndKeyForPurpose(1, 1);
		REQUIRE_THROWS_AS(masterWallet->GetPublicKey("wrongid"), std::logic_error);
	}
}

TEST_CASE("Master wallet Sign method of id agent", "[Sign-IdAgent]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string mnemonic = "you train view salon cancel impulse phrase oxygen sport crack peasant observe";
	bool singleAddress = false;
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, phrasePassword, payPassword, singleAddress));

	std::string id = masterWallet->DeriveIdAndKeyForPurpose(1, 1);

	SECTION("Normal sign") {
		std::string signedMsg = masterWallet->Sign(id, "mymessage", payPassword);
		REQUIRE_FALSE(signedMsg.empty());

		REQUIRE(masterWallet->CheckSign(masterWallet->GetPublicKey(id), "mymessage", signedMsg));
	}
	SECTION("Sign empty message") {
		REQUIRE_THROWS_AS(masterWallet->Sign(id, "", payPassword), std::invalid_argument);
	}
	SECTION("Sign with pay password that is empty or less than 8") {
		REQUIRE_THROWS_AS(masterWallet->Sign(id, "mymessage", ""), std::invalid_argument);
		REQUIRE_THROWS_AS(masterWallet->Sign(id, "mymessage", "invalid"), std::invalid_argument);
	}
	SECTION("Sign with pay password that is more than 128") {
		REQUIRE_THROWS_AS(masterWallet->Sign(id, "mymessage",
											 "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"),
						  std::invalid_argument);
	}
	SECTION("Sign with wrong password") {
		REQUIRE_THROWS_AS(masterWallet->Sign(id, "mymessage", "wrongpassword"), std::logic_error);
	}
	SECTION("Sign with wrong id") {
		REQUIRE_THROWS_AS(masterWallet->Sign("wrongid", "mymessage", payPassword), std::logic_error);
	}
}

TEST_CASE("Master wallet IsIdValid method test", "[IsIdValid]") {
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet());

	SECTION("Normal test") {
		REQUIRE(masterWallet->IsIdValid("im1NmKj6QKGmFToknsNP8cJyfCoU5sS26Y"));
	}
	SECTION("Invalid id with not base58 character") {
		REQUIRE_FALSE(masterWallet->IsIdValid("im1NmKj6QKGmFToknsNP8cJyfCoU5sS26I"));
	}
	SECTION("Invalid id with wrong length") {
		REQUIRE_FALSE(masterWallet->IsIdValid("im1NmKj6QKGmFToknsNP8cJyfCoU5sS26"));
	}
	SECTION("Invalid id with wrong prefix") {
		REQUIRE_FALSE(masterWallet->IsIdValid("Ym1NmKj6QKGmFToknsNP8cJyfCoU5sS26Y"));
	}
	SECTION("Invalid even is a valid normal address") {
		REQUIRE(masterWallet->IsAddressValid("EZuWALdKM92U89NYAN5DDP5ynqMuyqG5i3"));
		REQUIRE_FALSE(masterWallet->IsIdValid("EZuWALdKM92U89NYAN5DDP5ynqMuyqG5i3"));
	}
}

TEST_CASE("Master wallet GetSupportedChains method test", "[GetSupportedChains]") {
	//todo update me when CoinConfig.json file changed
	SECTION("Normal test") {
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet());
		std::vector<std::string> chainIdList = masterWallet->GetSupportedChains();
		REQUIRE(chainIdList.size() == 3);
	}
}

TEST_CASE("Master wallet GetAllSubWallets method test", "[GetAllSubWallets]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	uint64_t feePerKB = 10000;

	SECTION("Test normal adding") {
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, phrasePassword, payPassword));

		REQUIRE(masterWallet->GetAllSubWallets().size() == 0);

		std::string masterWalletId = masterWallet->GetId();

		masterWallet->CreateSubWallet("ELA", feePerKB);
		REQUIRE(masterWallet->GetAllSubWallets().size() == 1);

		masterWallet->CreateSubWallet("IdChain", feePerKB);
		REQUIRE(masterWallet->GetAllSubWallets().size() == 2);
	}
}

TEST_CASE("Master wallet manager restoreKeyStore method", "[restoreKeyStore]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "MasterWalletId";
	uint64_t feePerKB = 10000;

	std::string chainId1 = "ELA";
	std::string chainId2 = "IdChain";
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, phrasePassword, payPassword));
	masterWallet->CreateSubWallet(chainId1, feePerKB);
	masterWallet->CreateSubWallet(chainId2, feePerKB);

	KeyStore keyStore("Data");
	masterWallet->restoreKeyStoreWrapper(keyStore, payPassword);

	IAccount *account = keyStore.CreateAccountFromJson(payPassword);
	CHECK(keyStore.json().GetMnemonic() == mnemonic);
	CHECK(account->GetEncryptedPhrasePassword() == masterWallet->getEncryptedPhrasePassword());

	std::vector<CoinInfo> coinInfoList = keyStore.json().GetCoinInfoList();
	REQUIRE(coinInfoList.size() == 2);
}

TEST_CASE("Master wallet manager initFromKeyStore method", "[initFromKeyStore]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string masterWalletId = "MasterWalletId";
	uint64_t feePerKB = 10000;

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, phrasePassword, payPassword));
	masterWallet->CreateSubWallet("ELA", feePerKB);
	masterWallet->CreateSubWallet("IdChain", feePerKB);

	std::string publicKey = masterWallet->GetPublicKey();

	KeyStore keyStore("Data");
	masterWallet->restoreKeyStoreWrapper(keyStore, payPassword);

	masterWallet.reset(new TestMasterWallet());

	masterWallet->initFromKeyStoreWrapper(keyStore, payPassword);
	std::string publicKey1 = masterWallet->GetPublicKey();

	REQUIRE(publicKey == publicKey1);
	REQUIRE(masterWallet->GetAllSubWallets().size() == 2);
}

TEST_CASE("Master wallet save and restore", "[Save&Restore]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	uint64_t feePerKB = 10000;

	SECTION("Import from master wallet store should load all sub wallets") {
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet(mnemonic, phrasePassword, payPassword));
		ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", feePerKB);
		REQUIRE(subWallet != nullptr);
		REQUIRE(dynamic_cast<MainchainSubWallet *>(subWallet) != nullptr);
		subWallet = masterWallet->CreateSubWallet("IdChain", feePerKB);
		REQUIRE(subWallet != nullptr);
		REQUIRE(dynamic_cast<IdChainSubWallet *>(subWallet) != nullptr);

		std::string newPassword = "newPayPassword";
		masterWallet->ChangePassword(payPassword, newPassword);

		REQUIRE_NOTHROW(masterWallet->Sign("MyMessage", newPassword));
		REQUIRE_NOTHROW(subWallet->Sign("MyPassword", newPassword));

		boost::filesystem::path localStore = "Data";
		localStore /= "MasterWalletTest";
		localStore /= "MasterWalletStore.json";
		masterWallet->Save();
		masterWallet.reset(new TestMasterWallet(localStore)); //save and reload in this line
		REQUIRE_NOTHROW(masterWallet->Sign("MyMessage", newPassword));

		std::vector<ISubWallet *> subwallets = masterWallet->GetAllSubWallets();
		REQUIRE(subwallets.size() == 2);
		REQUIRE(subwallets[0] != nullptr);
		REQUIRE_NOTHROW(subwallets[0]->Sign("MyPassword", newPassword));
		REQUIRE(subwallets[1] != nullptr);
		REQUIRE_NOTHROW(subwallets[1]->Sign("MyPassword", newPassword));
		for (int i = 0; i < 2; ++i) {
			if (subwallets[i]->GetChainId() == "ELA")
				REQUIRE(dynamic_cast<MainchainSubWallet *>(subwallets[i]) != nullptr);
			else if (subwallets[i]->GetChainId() == "IdChain")
				REQUIRE(dynamic_cast<IdChainSubWallet *>(subwallets[i]) != nullptr);
		}
	}
}