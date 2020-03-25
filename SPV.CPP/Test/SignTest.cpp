// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <Account/Account.h>
#include <Account/SubAccount.h>
#include <Common/Utils.h>
#include <Common/Log.h>
#include <Plugin/Transaction/Program.h>
#include <Plugin/Transaction/Transaction.h>
#include <WalletCore/BIP39.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/Key.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Sign transaction test", "[SignTransaction]") {
	Log::registerMultiLogger();

	nlohmann::json content = "{\"Attributes\":[{\"Data\":\"353634383333303934\",\"Usage\":0}],\"BlockHeight\":2147483647,\"Fee\":10000,\"Inputs\":[{\"Address\":\"8Gqrkk876Kc1HUjeG9evyFsc91RGYWyQj4\",\"Amount\":200000000,\"Index\":0,\"Script\":\"76a914134a742f7782c295d3ea18cb59cd0101b21b1a2f88ac\",\"Sequence\":4294967295,\"Signature\":\"\",\"TxHash\":\"e77c3bea963d124311076d4737372cbb23aef8d63d5eadaad578455d481cc025\"}],\"IsRegistered\":false,\"LockTime\":0,\"Outputs\":[{\"FixedIndex\":0,\"Address\":\"Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ\",\"Amount\":\"10000000\",\"AssetId\":\"b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3\",\"OutputLock\":0,\"ProgramHash\":\"21db215de2758b7d743f66e4c66cfcc35dc54ccbcb\",\"OutputType\":0,\"Payload\":null},{\"FixedIndex\":1,\"Address\":\"8Gqrkk876Kc1HUjeG9evyFsc91RGYWyQj4\",\"Amount\":\"189990000\",\"AssetId\":\"b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3\",\"OutputLock\":0,\"ProgramHash\":\"12134a742f7782c295d3ea18cb59cd0101b21b1a2f\",\"OutputType\":0,\"Payload\":null}],\"PayLoad\":null,\"PayloadVersion\":0,\"Programs\":[],\"Remark\":\"\",\"Timestamp\":0,\"TxHash\":\"80a0eb3c6bbce2c21d542c7ce9d248fe013fc1c757addd7fcee04b14098d5fa7\",\"Type\":2,\"Version\":1}"_json;

	SECTION("Sign and Verify") {
		std::string mnemonic = "敌 宾 饰 详 贪 卷 剥 汇 层 富 怨 穷";
		uint512 seed = BIP39::DeriveSeed(mnemonic, "");

		HDSeed hdseed(seed.bytes());
		HDKeychain rootprv(hdseed.getExtendedKey(true));

		Key key1 = rootprv.getChild("1'/0");
		Key key2 = rootprv.getChild("2'/0");

		std::string msg = "hello world";

		bytes_t signature1 = key1.Sign(msg);
		REQUIRE(key1.Verify(msg, signature1));
		REQUIRE(!key2.Verify(msg, signature1));

		bytes_t signature2 = key2.Sign(msg);
		REQUIRE(key2.Verify(msg, signature2));
		REQUIRE(!key1.Verify(msg, signature2));
	}

	SECTION("BIP45") {

		SECTION("HD account sign multi sign tx test") {
			std::string mnemonic1 = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
			std::string mnemonic2 = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
			std::string mnemonic3 = "flat universe quantum uniform emerge blame lemon detail april sting aerobic disease";
			std::string mnemonic4 = "敌 宾 饰 详 贪 卷 剥 汇 层 富 怨 穷";
			std::string passphrase = "";
			std::string payPasswd = "12345678";
			uint32_t requiredSignCount = 3;
			uint32_t coinIndex = 0;

			AccountPtr account1(new Account("Data/1", mnemonic1, passphrase, payPasswd, false));
			SubAccountPtr subAccount1(new SubAccount(account1, coinIndex));
			subAccount1->Init();
			std::string multiSignPubKey1 = account1->MasterPubKeyHDPMString();
			bytes_t ownerPubKey1 = account1->OwnerPubKey();

			AccountPtr account2(new Account("Data/2", mnemonic2, passphrase, payPasswd, false));
			SubAccountPtr subAccount2(new SubAccount(account2, coinIndex));
			subAccount2->Init();
			std::string multiSignPubKey2 = account2->MasterPubKeyHDPMString();
			bytes_t ownerPubKey2 = account2->OwnerPubKey();

			AccountPtr account3(new Account("Data/3", mnemonic3, passphrase, payPasswd, false));
			SubAccountPtr subAccount3(new SubAccount(account3, coinIndex));
			subAccount3->Init();
			std::string multiSignPubKey3 = account3->MasterPubKeyHDPMString();
			bytes_t ownerPubKey3 = account3->OwnerPubKey();

			AccountPtr account4(new Account("Data/4", mnemonic4, passphrase, payPasswd, false));
			SubAccountPtr subAccount4(new SubAccount(account4, coinIndex));
			subAccount4->Init();
			std::string multiSignPubKey4 = account4->MasterPubKeyHDPMString();
			bytes_t votePubKey4 = account4->OwnerPubKey();

			SECTION("Standard address sign test") {
				AddressArray addresses;
				subAccount1->GetAllAddresses(addresses, 0, 100, false);

				REQUIRE(!addresses.empty());
				bytes_t redeemScript;
				std::string path;
				REQUIRE(subAccount1->GetCodeAndPath(addresses.back(), redeemScript, path));

				TransactionPtr tx(new Transaction);
				tx->FromJson(content);
				tx->AddProgram(ProgramPtr(new Program(path, redeemScript, bytes_t())));


				REQUIRE_THROWS(subAccount3->SignTransaction(tx, payPasswd));
				REQUIRE_NOTHROW(subAccount1->SignTransaction(tx, payPasswd));
				REQUIRE(tx->IsSigned());


				subAccount2->GetAllAddresses(addresses, 0, 100, false);

				tx->ClearPrograms();
				REQUIRE(subAccount2->GetCodeAndPath(addresses.front(), redeemScript, path));
				tx->AddProgram(ProgramPtr(new Program(path, redeemScript, bytes_t())));
				REQUIRE_THROWS(subAccount1->SignTransaction(tx, payPasswd));
				REQUIRE_THROWS(subAccount3->SignTransaction(tx, payPasswd));

				REQUIRE_NOTHROW(subAccount2->SignTransaction(tx, payPasswd));

				REQUIRE_THROWS(subAccount2->SignTransaction(tx, payPasswd));

				REQUIRE(tx->IsSigned());
			}

			SECTION("Owner standard address sign test") {
				AddressPtr addr(new Address(PrefixStandard, ownerPubKey1));
				bytes_t redeemScript;
				std::string path;
				REQUIRE(subAccount1->GetCodeAndPath(addr, redeemScript, path));

				TransactionPtr tx(new Transaction());
				tx->FromJson(content);

				tx->AddProgram(ProgramPtr(new Program(path, redeemScript, bytes_t())));

				REQUIRE_THROWS(subAccount2->SignTransaction(tx, payPasswd));
				REQUIRE(!tx->IsSigned());
				REQUIRE_NOTHROW(subAccount1->SignTransaction(tx, payPasswd));
				REQUIRE(tx->IsSigned());

			}

			SECTION("Owner deposit address sign test") {
				AddressPtr addr(new Address(PrefixDeposit, ownerPubKey1));
				bytes_t redeemScript;
				std::string path;
				REQUIRE(subAccount1->GetCodeAndPath(addr, redeemScript, path));

				TransactionPtr tx(new Transaction);
				tx->FromJson(content);

				tx->AddProgram(ProgramPtr(new Program(path, redeemScript, bytes_t())));


				REQUIRE_THROWS(subAccount2->SignTransaction(tx, payPasswd));
				REQUIRE(!tx->IsSigned());
				REQUIRE_NOTHROW(subAccount1->SignTransaction(tx, payPasswd));
				REQUIRE(tx->IsSigned());

			}

			SECTION("Readonly multi sign account test") {
				TransactionPtr tx(new Transaction);
				tx->FromJson(content);

				std::vector<PublicKeyRing> cosigners;
				cosigners.push_back(PublicKeyRing(account1->RequestPubKey().getHex(), multiSignPubKey1));
				cosigners.push_back(PublicKeyRing(account2->RequestPubKey().getHex(), multiSignPubKey2));
				cosigners.push_back(PublicKeyRing(account3->RequestPubKey().getHex(), multiSignPubKey3));
				cosigners.push_back(PublicKeyRing(account4->RequestPubKey().getHex(), multiSignPubKey4));

				AccountPtr multiSignAccount(new Account("Data/multisign", cosigners, requiredSignCount, false, false));
				SubAccountPtr multiSignSubAccount(new SubAccount(multiSignAccount, coinIndex));
				multiSignSubAccount->Init();
				AddressArray addresses;
				multiSignSubAccount->GetAllAddresses(addresses, 0, 1, false);
				REQUIRE(!addresses.empty());
				bytes_t redeemScript;
				std::string path;
				REQUIRE(multiSignSubAccount->GetCodeAndPath(addresses.front(), redeemScript, path));
				tx->AddProgram(ProgramPtr(new Program(path, redeemScript, bytes_t())));

				REQUIRE_NOTHROW(subAccount1->SignTransaction(tx, payPasswd));
				REQUIRE(!tx->IsSigned());

				REQUIRE_NOTHROW(subAccount2->SignTransaction(tx, payPasswd));
				REQUIRE(!tx->IsSigned());

				REQUIRE_NOTHROW(subAccount3->SignTransaction(tx, payPasswd));
				REQUIRE(tx->IsSigned());

				REQUIRE_THROWS(subAccount4->SignTransaction(tx, payPasswd));
				REQUIRE(tx->IsSigned());
			}

			SECTION("Multi sign account with private key test") {
				TransactionPtr tx(new Transaction);
				tx->FromJson(content);

				std::vector<PublicKeyRing> cosigners;
				cosigners.push_back(PublicKeyRing(account2->RequestPubKey().getHex(), multiSignPubKey2));
				cosigners.push_back(PublicKeyRing(account3->RequestPubKey().getHex(), multiSignPubKey3));
				cosigners.push_back(PublicKeyRing(account4->RequestPubKey().getHex(), multiSignPubKey4));

				AccountPtr multiSignAccount1(
					new Account("Data/m1", mnemonic1, passphrase, payPasswd, cosigners, requiredSignCount, false,
								false));
				SubAccountPtr ms1(new SubAccount(multiSignAccount1, coinIndex));
				ms1->Init();
				AddressArray addresses1;
				ms1->GetAllAddresses(addresses1, 0, 10, false);

				cosigners.clear();
				cosigners.push_back(PublicKeyRing(account1->RequestPubKey().getHex(), multiSignPubKey1));
				cosigners.push_back(PublicKeyRing(account3->RequestPubKey().getHex(), multiSignPubKey3));
				cosigners.push_back(PublicKeyRing(account4->RequestPubKey().getHex(), multiSignPubKey4));

				AccountPtr multiSignAccount2(
					new Account("Data/m2", mnemonic2, passphrase, payPasswd, cosigners, requiredSignCount, false,
								false));
				SubAccountPtr ms2(new SubAccount(multiSignAccount2, coinIndex));
				ms2->Init();
				AddressArray addresses2;
				ms2->GetAllAddresses(addresses2, 0, 10, false);

				cosigners.clear();
				cosigners.push_back(PublicKeyRing(account1->RequestPubKey().getHex(), multiSignPubKey1));
				cosigners.push_back(PublicKeyRing(account2->RequestPubKey().getHex(), multiSignPubKey2));
				cosigners.push_back(PublicKeyRing(account4->RequestPubKey().getHex(), multiSignPubKey4));

				AccountPtr multiSignAccount3(
					new Account("Data/m3", mnemonic3, passphrase, payPasswd, cosigners, requiredSignCount, false,
								false));
				SubAccountPtr ms3(new SubAccount(multiSignAccount3, coinIndex));
				ms3->Init();
				AddressArray addresses3;
				ms3->GetAllAddresses(addresses3, 0, 10, false);

				cosigners.clear();
				cosigners.push_back(PublicKeyRing(account1->RequestPubKey().getHex(), multiSignPubKey1));
				cosigners.push_back(PublicKeyRing(account2->RequestPubKey().getHex(), multiSignPubKey2));
				cosigners.push_back(PublicKeyRing(account3->RequestPubKey().getHex(), multiSignPubKey3));

				AccountPtr multiSignAccount4(
					new Account("Data/m4", mnemonic4, passphrase, payPasswd, cosigners, requiredSignCount, false,
								false));
				SubAccountPtr ms4(new SubAccount(multiSignAccount4, coinIndex));
				ms4->Init();
				AddressArray addresses4;
				ms4->GetAllAddresses(addresses4, 0, 10, false);

				cosigners.clear();
				cosigners.push_back(PublicKeyRing(account1->RequestPubKey().getHex(), multiSignPubKey1));
				cosigners.push_back(PublicKeyRing(account2->RequestPubKey().getHex(), multiSignPubKey2));
				cosigners.push_back(PublicKeyRing(account3->RequestPubKey().getHex(), multiSignPubKey3));
				cosigners.push_back(PublicKeyRing(account4->RequestPubKey().getHex(), multiSignPubKey4));

				AccountPtr multiSignAccount5(
					new Account("Data/multisign-readonly", cosigners, requiredSignCount, false, false));

				SubAccountPtr ms5(new SubAccount(multiSignAccount5, coinIndex));
				ms5->Init();
				AddressArray addresses5;
				ms5->GetAllAddresses(addresses5, 0, 10, false);

				REQUIRE(!addresses1.empty());
				REQUIRE(addresses1.size() == addresses2.size());
				REQUIRE(addresses1.size() == addresses3.size());
				REQUIRE(addresses1.size() == addresses4.size());
				REQUIRE(addresses1.size() == addresses5.size());
				bytes_t redeemScript;
				std::string path;
				REQUIRE(ms1->GetCodeAndPath(addresses1[0], redeemScript, path));

				tx->AddProgram(ProgramPtr(new Program(path, redeemScript, bytes_t())));


				REQUIRE(!tx->IsSigned());
				REQUIRE_NOTHROW(ms1->SignTransaction(tx, payPasswd));

				REQUIRE(!tx->IsSigned());
				REQUIRE_NOTHROW(ms2->SignTransaction(tx, payPasswd));

				REQUIRE(!tx->IsSigned());
				REQUIRE_NOTHROW(ms4->SignTransaction(tx, payPasswd));

				REQUIRE(tx->IsSigned());
				REQUIRE_THROWS(ms3->SignTransaction(tx, payPasswd));

				REQUIRE(tx->IsSigned());
			}
		}
	}

	SECTION("BIP44") {
		TransactionPtr tx(new Transaction);
		tx->FromJson(content);

		const std::string backupPasswd = "11111111";
		const std::string payPasswd = "payPasswd";
		nlohmann::json keystoreJson = R"(
			{"iv":"hm0lwrO1A8F7rel/0B0rWQ==","v":1,"iter":10000,"ks":128,"ts":64,"mode":"ccm","adata":"","cipher":"aes","salt":"dDXsCNh45Yw=","ct":"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"}
		)"_json;
		KeyStore ks;
		REQUIRE_NOTHROW(ks.Import(keystoreJson, backupPasswd));
		AccountPtr account1(new Account("Data/AccountTestMultiSignFromWeb", ks, payPasswd));
		SubAccountPtr ms1(new SubAccount(account1, 0));
		ms1->Init();

		AddressArray addr1;
		size_t count1 = ms1->GetAllAddresses(addr1, 0, 1, false);
		REQUIRE(count1 >= 1);
		REQUIRE(addr1[0]->String() == "8W6TRf4ZxyTaDZdJs4Gd8dwFkvb62dVN1r");

		std::vector<PublicKeyRing> cosigners = account1->MasterPubKeyRing();


		AccountPtr account2(new Account("Data/MultiSignRO", cosigners, account1->GetM(), false, true));
		SubAccountPtr ms2(new SubAccount(account2, 0));
		ms2->Init();
		AddressArray addr2;
		size_t count2 = ms2->GetAllAddresses(addr2, 0, 1, false);
		REQUIRE_THROWS(account2->MasterPubKey() == nullptr);
		REQUIRE_THROWS(account2->MultiSignSigner() == nullptr);
		REQUIRE(count2 >= 1);
		REQUIRE(addr2[0]->String() == "8W6TRf4ZxyTaDZdJs4Gd8dwFkvb62dVN1r");

		bytes_t redeemScript;
		std::string path;
		REQUIRE(ms2->GetCodeAndPath(addr2[0], redeemScript, path));

		tx->AddProgram(ProgramPtr(new Program(path, redeemScript, bytes_t())));

		REQUIRE(!tx->IsSigned());
		REQUIRE_NOTHROW(ms1->SignTransaction(tx, payPasswd));


		nlohmann::json readonlyJSON = account1->ExportReadonlyWallet();

		AccountPtr account3;
		SubAccountPtr ms3;
		REQUIRE_NOTHROW(account3 = AccountPtr(new Account("Data/ReadOnly", readonlyJSON)));
		REQUIRE_NOTHROW(ms3 = SubAccountPtr(new SubAccount(account3, 0)));
		ms3->Init();
		REQUIRE(account3->GetM() == account1->GetM());
		REQUIRE(account3->GetN() == account1->GetN());
		REQUIRE(account3->GetSignType() == account1->GetSignType());
		REQUIRE(account3->DerivationStrategy() == account1->DerivationStrategy());
		REQUIRE(account3->SingleAddress() == account1->SingleAddress());
		REQUIRE_THROWS(account3->ExportMnemonic(payPasswd));
		REQUIRE(account3->SubWalletInfoList().size() == account1->SubWalletInfoList().size());
		REQUIRE_THROWS(account3->RootKey(payPasswd));
		REQUIRE(account3->RequestPubKey().empty());
		REQUIRE_THROWS(account3->OwnerPubKey());
		REQUIRE_THROWS(account3->MasterPubKey());
		REQUIRE_THROWS(account3->MultiSignSigner());
		REQUIRE(account3->CosignerIndex() == -1);
		REQUIRE(account3->Readonly() == true);
		AddressArray addr3;
		size_t count3 = ms3->GetAllAddresses(addr3, 0, 1, false);
		REQUIRE(count3 == count2);
		REQUIRE(addr3[0]->String() == "8W6TRf4ZxyTaDZdJs4Gd8dwFkvb62dVN1r");
	}
}
