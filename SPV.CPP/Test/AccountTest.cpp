// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <Account/Account.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Account test", "[Account]") {
	Log::registerMultiLogger();
	SECTION("ExportKeystore test") {
		std::string mnemonic1 = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		std::string mnemonic2 = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
		std::string mnemonic3 = "flat universe quantum uniform emerge blame lemon detail april sting aerobic disease";
		std::string mnemonic4 = "敌 宾 饰 详 贪 卷 剥 汇 层 富 怨 穷";
		std::string passphrase = "";
		std::string payPasswd = "12345678";
		uint32_t requiredSignCount = 3;

		AccountPtr account1(new Account("Data/1", mnemonic1, passphrase, payPasswd, false));
		std::string multiSignPubKey1 = account1->MasterPubKeyHDPMString();

		AccountPtr account2(new Account("Data/2", mnemonic2, passphrase, payPasswd, false));
		std::string multiSignPubKey2 = account2->MasterPubKeyHDPMString();

		AccountPtr account3(new Account("Data/3", mnemonic3, passphrase, payPasswd, false));
		std::string multiSignPubKey3 = account3->MasterPubKeyHDPMString();

		SECTION("Readonly multi sign account test") {
			std::vector<PublicKeyRing> cosigners;
			cosigners.push_back(PublicKeyRing(account1->RequestPubKey().getHex(), multiSignPubKey1));
			cosigners.push_back(PublicKeyRing(account2->RequestPubKey().getHex(), multiSignPubKey2));
			cosigners.push_back(PublicKeyRing(account3->RequestPubKey().getHex(), multiSignPubKey3));

			AccountPtr multiSignAccount1(new Account("Data/multisign", cosigners, requiredSignCount, false, false));

			KeyStore keyStore1 = multiSignAccount1->ExportKeystore(payPasswd);
			REQUIRE(keyStore1.WalletJson().GetM() == requiredSignCount);
			REQUIRE(keyStore1.WalletJson().GetN() == cosigners.size());

			const std::vector<PublicKeyRing> pubkeys = keyStore1.WalletJson().GetPublicKeyRing();
			REQUIRE(pubkeys.size() == cosigners.size());
			for (size_t i = 0; i < pubkeys.size(); ++i) {
				REQUIRE(pubkeys[i].GetxPubKey() == cosigners[i].GetxPubKey());
				REQUIRE(pubkeys[i].GetRequestPubKey() == cosigners[i].GetRequestPubKey());
			}

			AccountPtr multiSignAccount2(new Account("Data/multisign2", keyStore1, payPasswd));
			REQUIRE(multiSignAccount2->Readonly());

			KeyStore keyStore2 = multiSignAccount2->ExportKeystore(payPasswd);
			REQUIRE(keyStore2.WalletJson().GetM() == requiredSignCount);
			REQUIRE(keyStore2.WalletJson().GetN() == cosigners.size());

			const std::vector<PublicKeyRing> pubkeyRings = keyStore2.WalletJson().GetPublicKeyRing();
			REQUIRE(pubkeyRings.size() == cosigners.size());
			for (size_t i = 0; i < pubkeyRings.size(); ++i) {
				REQUIRE(pubkeyRings[i].GetxPubKey() == cosigners[i].GetxPubKey());
				REQUIRE(pubkeyRings[i].GetRequestPubKey() == cosigners[i].GetRequestPubKey());
			}

		}

		SECTION("Multi sign account with private key test") {
			AccountPtr account4(new Account("Data/4", mnemonic4, passphrase, payPasswd, false));
			std::string multiSignPubKey4 = account4->MasterPubKeyHDPMString();

			std::vector<PublicKeyRing> cosigners;
			cosigners.push_back(PublicKeyRing(account2->RequestPubKey().getHex(), multiSignPubKey2));
			cosigners.push_back(PublicKeyRing(account3->RequestPubKey().getHex(), multiSignPubKey3));
			cosigners.push_back(PublicKeyRing(account4->RequestPubKey().getHex(), multiSignPubKey4));

			AccountPtr multiSignAccount1(
					new Account("Data/m1", mnemonic1, passphrase, payPasswd, cosigners, requiredSignCount, false,
					            false));

			KeyStore keyStore1 = multiSignAccount1->ExportKeystore(payPasswd);
			REQUIRE(keyStore1.WalletJson().GetM() == requiredSignCount);
			REQUIRE(keyStore1.WalletJson().GetN() == cosigners.size() + 1);

			const std::vector<PublicKeyRing> pubkeys = keyStore1.WalletJson().GetPublicKeyRing();
			REQUIRE(pubkeys.size() == cosigners.size() + 1);
			for (size_t i = 0; i < pubkeys.size(); ++i) {
				if (i == cosigners.size()) {
					REQUIRE(pubkeys[i].GetxPubKey() == account1->MasterPubKeyHDPMString());
					REQUIRE(pubkeys[i].GetRequestPubKey() == account1->RequestPubKey().getHex());
				} else {
					REQUIRE(pubkeys[i].GetxPubKey() == cosigners[i].GetxPubKey());
					REQUIRE(pubkeys[i].GetRequestPubKey() == cosigners[i].GetRequestPubKey());
				}
			}

			AccountPtr multiSignAccount2(new Account("Data/m2", keyStore1, payPasswd));
			REQUIRE(!multiSignAccount2->Readonly());

			KeyStore keyStore2 = multiSignAccount2->ExportKeystore(payPasswd);
			REQUIRE(keyStore2.WalletJson().GetM() == requiredSignCount);
			REQUIRE(keyStore2.WalletJson().GetN() == cosigners.size() + 1);

			const std::vector<PublicKeyRing> pubkeyRings = keyStore2.WalletJson().GetPublicKeyRing();
			REQUIRE(pubkeyRings.size() == cosigners.size() + 1);
			for (size_t i = 0; i < pubkeyRings.size(); ++i) {
				if (i == cosigners.size()) {
					REQUIRE(pubkeyRings[i].GetxPubKey() == account1->MasterPubKeyHDPMString());
					REQUIRE(pubkeyRings[i].GetRequestPubKey() == account1->RequestPubKey().getHex());
				} else {
					REQUIRE(pubkeyRings[i].GetxPubKey() == cosigners[i].GetxPubKey());
					REQUIRE(pubkeyRings[i].GetRequestPubKey() == cosigners[i].GetRequestPubKey());
				}
			}
		}
	}

}
