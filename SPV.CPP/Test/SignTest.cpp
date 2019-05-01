// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include <SDK/Account/Account.h>
#include <SDK/Account/SubAccount.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Plugin/Transaction/Program.h>
#include <SDK/Plugin/Transaction/Transaction.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Sign transaction test", "[SignTransaction]") {
	Log::registerMultiLogger();

	SECTION("HD account sign multi sign tx test") {
		nlohmann::json content = "{\"Attributes\":[{\"Data\":\"353634383333303934\",\"Usage\":0}],\"BlockHeight\":2147483647,\"Fee\":10000,\"Inputs\":[{\"Address\":\"8Gqrkk876Kc1HUjeG9evyFsc91RGYWyQj4\",\"Amount\":200000000,\"Index\":0,\"Script\":\"76a914134a742f7782c295d3ea18cb59cd0101b21b1a2f88ac\",\"Sequence\":4294967295,\"Signature\":\"\",\"TxHash\":\"e77c3bea963d124311076d4737372cbb23aef8d63d5eadaad578455d481cc025\"}],\"IsRegistered\":false,\"LockTime\":0,\"Outputs\":[{\"Address\":\"Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ\",\"Amount\":10000000,\"AssetId\":\"b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3\",\"OutputLock\":0,\"ProgramHash\":\"21db215de2758b7d743f66e4c66cfcc35dc54ccbcb\",\"Script\":\"76a914db215de2758b7d743f66e4c66cfcc35dc54ccbcb88ac\",\"ScriptLen\":25,\"SignType\":172},{\"Address\":\"8Gqrkk876Kc1HUjeG9evyFsc91RGYWyQj4\",\"Amount\":189990000,\"AssetId\":\"b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3\",\"OutputLock\":0,\"ProgramHash\":\"12134a742f7782c295d3ea18cb59cd0101b21b1a2f\",\"Script\":\"76a914134a742f7782c295d3ea18cb59cd0101b21b1a2f88ac\",\"ScriptLen\":25,\"SignType\":174}],\"PayLoad\":null,\"PayloadVersion\":0,\"Programs\":[],\"Remark\":\"\",\"Timestamp\":0,\"TxHash\":\"80a0eb3c6bbce2c21d542c7ce9d248fe013fc1c757addd7fcee04b14098d5fa7\",\"Type\":2,\"Version\":1}"_json;

		std::string mnemonic1 = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		std::string mnemonic2 = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
		std::string mnemonic3 = "flat universe quantum uniform emerge blame lemon detail april sting aerobic disease";
		std::string mnemonic4 = "敌 宾 饰 详 贪 卷 剥 汇 层 富 怨 穷";
		std::string passphrase = "";
		std::string payPasswd = "12345678";
		uint32_t requiredSignCount = 3;
		uint32_t coinIndex = 0;
		Lockable lock;

		LocalStorePtr localstore1(new LocalStore("./Data/1", mnemonic1, passphrase, false, payPasswd));
		AccountPtr account1(new Account(localstore1, "Data"));
		SubAccountPtr subAccount1(new SubAccount(account1, coinIndex));
		subAccount1->Init({}, &lock);
		bytes_t multiSignPubKey1 = account1->RequestPubKey();
		bytes_t ownerPubKey1 = account1->OwnerPubKey();

		LocalStorePtr localStore2(new LocalStore("./Data/2", mnemonic2, passphrase, false, payPasswd));
		AccountPtr account2(new Account(localStore2, "Data"));
		SubAccountPtr subAccount2(new SubAccount(account2, coinIndex));
		subAccount2->Init({}, &lock);
		bytes_t multiSignPubKey2 = account2->RequestPubKey();
		bytes_t ownerPubKey2 = account2->OwnerPubKey();

		LocalStorePtr localstore3(new LocalStore("./Data/3", mnemonic3, passphrase, false, payPasswd));
		AccountPtr account3(new Account(localstore3, "Data"));
		SubAccountPtr subAccount3(new SubAccount(account3, coinIndex));
		subAccount3->Init({}, &lock);
		bytes_t multiSignPubKey3 = account3->RequestPubKey();
		bytes_t ownerPubKey3 = account3->OwnerPubKey();

		LocalStorePtr localstore4(new LocalStore("./Data/4", mnemonic4, passphrase, false, payPasswd));
		AccountPtr account4(new Account(localstore4, "Data"));
		SubAccountPtr subAccount4(new SubAccount(account4, coinIndex));
		subAccount4->Init({}, &lock);
		bytes_t multiSignPubKey4 = account4->RequestPubKey();
		bytes_t votePubKey4 = account4->OwnerPubKey();

		SECTION("Standard address sign test") {
			std::vector<Address> addresses;
			subAccount1->GetAllAddresses(addresses, 0, 100, true);

			REQUIRE(!addresses.empty());
			bytes_t redeemScript = subAccount1->GetRedeemScript(addresses[addresses.size() - 1]);

			TransactionPtr tx(new Transaction);
			tx->FromJson(content);
			tx->AddProgram(Program(redeemScript, bytes_t()));


			REQUIRE_THROWS(subAccount3->SignTransaction(tx, payPasswd));
			REQUIRE_NOTHROW(subAccount1->SignTransaction(tx, payPasswd));
			REQUIRE(tx->IsSigned());


			subAccount2->GetAllAddresses(addresses, 0, 100, true);

			tx->GetPrograms().clear();
			redeemScript = subAccount2->GetRedeemScript(addresses[0]);
			tx->AddProgram(Program(redeemScript, bytes_t()));
			REQUIRE_THROWS(subAccount1->SignTransaction(tx, payPasswd));
			REQUIRE_THROWS(subAccount3->SignTransaction(tx, payPasswd));

			REQUIRE_NOTHROW(subAccount2->SignTransaction(tx, payPasswd));

			REQUIRE_THROWS(subAccount2->SignTransaction(tx, payPasswd));

			REQUIRE(tx->IsSigned());
		}

		SECTION("Vote deposit address sign test") {
			std::string addr = Address(PrefixDeposit, ownerPubKey1).String();
			bytes_t redeemScript = subAccount1->GetRedeemScript(addr);

			TransactionPtr tx(new Transaction);
			tx->FromJson(content);

			tx->AddProgram(Program(redeemScript, bytes_t()));


			REQUIRE_THROWS(subAccount2->SignTransaction(tx, payPasswd));
			REQUIRE(!tx->IsSigned());
			REQUIRE_NOTHROW(subAccount1->SignTransaction(tx, payPasswd));
			REQUIRE(tx->IsSigned());

		}

		SECTION("Readonly multi sign account test") {
			TransactionPtr tx(new Transaction);
			tx->FromJson(content);

			std::vector<std::string> coSigners;
			coSigners.push_back(multiSignPubKey1.getHex());
			coSigners.push_back(multiSignPubKey2.getHex());
			coSigners.push_back(multiSignPubKey3.getHex());
			coSigners.push_back(multiSignPubKey4.getHex());
			LocalStorePtr localstore(new LocalStore("./Data/multisign", coSigners, requiredSignCount));

			AccountPtr multiSignAccount(new Account(localstore, "Data"));
			SubAccountPtr multiSignSubAccount(new SubAccount(multiSignAccount, coinIndex));
			multiSignSubAccount->Init({}, &lock);
			std::vector<Address> addresses;
			multiSignSubAccount->GetAllAddresses(addresses, 0, 1, true);
			REQUIRE(!addresses.empty());
			bytes_t redeemScript = multiSignSubAccount->GetRedeemScript(addresses[0]);;
			tx->AddProgram(Program(redeemScript, bytes_t()));

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

			LocalStorePtr localstore1(new LocalStore("./Data/m1", mnemonic1, passphrase, false, payPasswd));
			localstore1->AddPublicKeyRing(PublicKeyRing(account2->RequestPubKey().getHex()));
			localstore1->AddPublicKeyRing(PublicKeyRing(account3->RequestPubKey().getHex()));
			localstore1->AddPublicKeyRing(PublicKeyRing(account4->RequestPubKey().getHex()));
			localstore1->SetM(requiredSignCount);
			localstore1->SetN(localstore1->GetPublicKeyRing().size());
			AccountPtr multiSignAccount1(new Account(localstore1, "Data"));
			SubAccountPtr ms1(new SubAccount(multiSignAccount1, coinIndex));
			ms1->Init({}, &lock);
			std::vector<Address> addresses1;
			ms1->GetAllAddresses(addresses1, 0, 10, true);

			LocalStorePtr localstore2(new LocalStore("./Data/m2", mnemonic2, passphrase, false, payPasswd));
			localstore2->AddPublicKeyRing(PublicKeyRing(account1->RequestPubKey().getHex()));
			localstore2->AddPublicKeyRing(PublicKeyRing(account3->RequestPubKey().getHex()));
			localstore2->AddPublicKeyRing(PublicKeyRing(account4->RequestPubKey().getHex()));
			localstore2->SetM(requiredSignCount);
			localstore2->SetN(localstore2->GetPublicKeyRing().size());
			AccountPtr multiSignAccount2(new Account(localstore2, "Data"));
			SubAccountPtr ms2(new SubAccount(multiSignAccount2, coinIndex));
			ms2->Init({}, &lock);
			std::vector<Address> addresses2;
			ms2->GetAllAddresses(addresses2, 0, 10, true);

			LocalStorePtr localstore3(new LocalStore("./Data/m3", mnemonic3, passphrase, false, payPasswd));
			localstore3->AddPublicKeyRing(PublicKeyRing(account1->RequestPubKey().getHex()));
			localstore3->AddPublicKeyRing(PublicKeyRing(account2->RequestPubKey().getHex()));
			localstore3->AddPublicKeyRing(PublicKeyRing(account4->RequestPubKey().getHex()));
			localstore3->SetM(requiredSignCount);
			localstore3->SetN(localstore3->GetPublicKeyRing().size());
			AccountPtr multiSignAccount3(new Account(localstore3, "Data"));
			SubAccountPtr ms3(new SubAccount(multiSignAccount3, coinIndex));
			ms3->Init({}, &lock);
			std::vector<Address> addresses3;
			ms3->GetAllAddresses(addresses3, 0, 10, true);

			LocalStorePtr localstore4(new LocalStore("./Data/m4", mnemonic4, passphrase, false, payPasswd));
			localstore4->AddPublicKeyRing(PublicKeyRing(account1->RequestPubKey().getHex()));
			localstore4->AddPublicKeyRing(PublicKeyRing(account2->RequestPubKey().getHex()));
			localstore4->AddPublicKeyRing(PublicKeyRing(account3->RequestPubKey().getHex()));
			localstore4->SetM(requiredSignCount);
			localstore4->SetN(localstore4->GetPublicKeyRing().size());
			AccountPtr multiSignAccount4(new Account(localstore4, "Data"));
			SubAccountPtr ms4(new SubAccount(multiSignAccount4, coinIndex));
			ms4->Init({}, &lock);
			std::vector<Address> addresses4;
			ms4->GetAllAddresses(addresses4, 0, 10, true);

			std::vector<std::string> coSigners;
			coSigners.push_back(multiSignPubKey1.getHex());
			coSigners.push_back(multiSignPubKey2.getHex());
			coSigners.push_back(multiSignPubKey3.getHex());
			coSigners.push_back(multiSignPubKey4.getHex());
			LocalStorePtr localstore(new LocalStore("./Data/multisign", coSigners, requiredSignCount));

			AccountPtr multiSignAccount5(new Account(localstore, "Data"));

			SubAccountPtr ms5(new SubAccount(multiSignAccount5, coinIndex));
			ms5->Init({}, &lock);
			std::vector<Address> addresses5;
			ms5->GetAllAddresses(addresses5, 0, 10, true);

			REQUIRE(!addresses1.empty());
			REQUIRE(addresses1.size() == addresses2.size());
			REQUIRE(addresses1.size() == addresses3.size());
			REQUIRE(addresses1.size() == addresses4.size());
			REQUIRE(addresses1.size() == addresses5.size());
			bytes_t redeemScript = ms1->GetRedeemScript(addresses1[0]);

			tx->AddProgram(Program(redeemScript, bytes_t()));


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