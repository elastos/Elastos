// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include <SDK/Account/MultiSignAccount.h>
#include <SDK/Account/SimpleAccount.h>
#include <SDK/Account/MultiSignSubAccount.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Account/StandardAccount.h>
#include <SDK/Account/HDSubAccount.h>
#include <SDK/Account/SubAccountGenerator.h>
#include <SDK/Account/StandardSingleSubAccount.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Sign transaction test", "[SignTransaction]") {

	SECTION("Multi sign account sign multi sign tx test") {
		TransactionPtr tx(new Transaction);
		nlohmann::json content = "{\"Attributes\":[{\"Data\":\"353634383333303934\",\"Usage\":0}],\"BlockHeight\":2147483647,\"Fee\":10000,\"Inputs\":[{\"Address\":\"8Gqrkk876Kc1HUjeG9evyFsc91RGYWyQj4\",\"Amount\":200000000,\"Index\":0,\"Script\":\"76a914134a742f7782c295d3ea18cb59cd0101b21b1a2f88ac\",\"Sequence\":4294967295,\"Signature\":\"\",\"TxHash\":\"e77c3bea963d124311076d4737372cbb23aef8d63d5eadaad578455d481cc025\"}],\"IsRegistered\":false,\"LockTime\":0,\"Outputs\":[{\"Address\":\"Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ\",\"Amount\":10000000,\"AssetId\":\"b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3\",\"OutputLock\":0,\"ProgramHash\":\"21db215de2758b7d743f66e4c66cfcc35dc54ccbcb\",\"Script\":\"76a914db215de2758b7d743f66e4c66cfcc35dc54ccbcb88ac\",\"ScriptLen\":25,\"SignType\":172},{\"Address\":\"8Gqrkk876Kc1HUjeG9evyFsc91RGYWyQj4\",\"Amount\":189990000,\"AssetId\":\"b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3\",\"OutputLock\":0,\"ProgramHash\":\"12134a742f7782c295d3ea18cb59cd0101b21b1a2f\",\"Script\":\"76a914134a742f7782c295d3ea18cb59cd0101b21b1a2f88ac\",\"ScriptLen\":25,\"SignType\":174}],\"PayLoad\":null,\"PayloadVersion\":0,\"Programs\":[],\"Remark\":\"\",\"Timestamp\":0,\"TxHash\":\"80a0eb3c6bbce2c21d542c7ce9d248fe013fc1c757addd7fcee04b14098d5fa7\",\"Type\":2,\"Version\":1}"_json;
		tx->FromJson(content);

		bytes_t key;
		std::string payPassword = "payPassword";
		std::vector<std::string> coSigners;

		std::string pubKey1 = "03b73a64f50c142c1f08710e04b928553508c3028e045dfdfdc5489434df13275e";
		std::string prvKey1 = "6e7910da9c066524273be2b493616ef0d4a848a0696829141a90458a9cf160af";
		std::string pubKey2 = "02f925e82f4482a9aa853a35203ab8965439c9db6aee8ef1783d2e1a491c28a482";
		std::string prvKey2 = "2c7c9180792e49a624b02ac2adff2f994ecc28044ee9889d6054159189da03a5";

		MultiSignAccount *account1 = new MultiSignAccount(new SimpleAccount(prvKey1, payPassword), {pubKey2}, 2);
		MultiSignSubAccount *subAccount1 = new MultiSignSubAccount(account1);

		MultiSignAccount *account2 = new MultiSignAccount(new SimpleAccount(prvKey2, payPassword), {pubKey1}, 2);
		MultiSignSubAccount *subAccount2 = new MultiSignSubAccount(account2);

		std::vector<Address> addrs;
		subAccount1->GetAllAddresses(addrs, 0, 100, true);
		bytes_t code = subAccount1->GetRedeemScript(addrs[0]);
		tx->AddProgram(Program(code, bytes_t()));

		REQUIRE_NOTHROW(subAccount1->SignTransaction(tx, payPassword));
		REQUIRE_THROWS(subAccount1->SignTransaction(tx, payPassword));

		nlohmann::json signedInfo = tx->GetSignedInfo();


		REQUIRE(signedInfo.size() == 1);
		REQUIRE((signedInfo[0]["SignType"] == "MultiSign"));
		REQUIRE((signedInfo[0]["Signers"][0] == pubKey1));
		REQUIRE(!tx->IsSigned());

		REQUIRE_NOTHROW(subAccount2->SignTransaction(tx, payPassword));
		REQUIRE_THROWS(subAccount2->SignTransaction(tx, payPassword));
		signedInfo = tx->GetSignedInfo();
		REQUIRE(signedInfo.size() == 1);
		REQUIRE((signedInfo[0]["SignType"] == "MultiSign"));
		REQUIRE((signedInfo[0]["Signers"][1] == pubKey2));
		REQUIRE(tx->IsSigned());

		SECTION("Simple account should not support vote") {
			bytes_t votePubKey = subAccount1->GetVotePublicKey();
			REQUIRE(votePubKey.size() == 0);
			REQUIRE_THROWS(subAccount1->DeriveVoteKey(payPassword));
		}
	}

	SECTION("HD account sign multi sign tx test") {
		nlohmann::json content = "{\"Attributes\":[{\"Data\":\"353634383333303934\",\"Usage\":0}],\"BlockHeight\":2147483647,\"Fee\":10000,\"Inputs\":[{\"Address\":\"8Gqrkk876Kc1HUjeG9evyFsc91RGYWyQj4\",\"Amount\":200000000,\"Index\":0,\"Script\":\"76a914134a742f7782c295d3ea18cb59cd0101b21b1a2f88ac\",\"Sequence\":4294967295,\"Signature\":\"\",\"TxHash\":\"e77c3bea963d124311076d4737372cbb23aef8d63d5eadaad578455d481cc025\"}],\"IsRegistered\":false,\"LockTime\":0,\"Outputs\":[{\"Address\":\"Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ\",\"Amount\":10000000,\"AssetId\":\"b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3\",\"OutputLock\":0,\"ProgramHash\":\"21db215de2758b7d743f66e4c66cfcc35dc54ccbcb\",\"Script\":\"76a914db215de2758b7d743f66e4c66cfcc35dc54ccbcb88ac\",\"ScriptLen\":25,\"SignType\":172},{\"Address\":\"8Gqrkk876Kc1HUjeG9evyFsc91RGYWyQj4\",\"Amount\":189990000,\"AssetId\":\"b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3\",\"OutputLock\":0,\"ProgramHash\":\"12134a742f7782c295d3ea18cb59cd0101b21b1a2f\",\"Script\":\"76a914134a742f7782c295d3ea18cb59cd0101b21b1a2f88ac\",\"ScriptLen\":25,\"SignType\":174}],\"PayLoad\":null,\"PayloadVersion\":0,\"Programs\":[],\"Remark\":\"\",\"Timestamp\":0,\"TxHash\":\"80a0eb3c6bbce2c21d542c7ce9d248fe013fc1c757addd7fcee04b14098d5fa7\",\"Type\":2,\"Version\":1}"_json;

		std::string phrase1 = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		std::string phrase2 = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
		std::string phrase3 = "flat universe quantum uniform emerge blame lemon detail april sting aerobic disease";
		std::string phrase4 = "敌 宾 饰 详 贪 卷 剥 汇 层 富 怨 穷";
		std::string phrasePassword = "";
		std::string payPassword = "12345678";
		uint32_t requiredSignCount = 3;
		uint32_t coinIndex = 0;
		Lockable lock;

		StandardAccount *account1 = new StandardAccount("./Data", phrase1, phrasePassword, payPassword);
		HDKeychain mpk1 = SubAccountGenerator::GenerateMasterPubKey(account1, coinIndex, payPassword);
		bytes_t votePubKey1 = SubAccountGenerator::GenerateVotePubKey(account1, coinIndex, payPassword);
		HDSubAccount hd1(mpk1, votePubKey1, account1, coinIndex);
		hd1.InitAccount({}, &lock);
		bytes_t multiSignPubKey1 = account1->GetMultiSignPublicKey();

		StandardAccount *account2 = new StandardAccount("./Data", phrase2, phrasePassword, payPassword);
		HDKeychain mpk2 = SubAccountGenerator::GenerateMasterPubKey(account2, coinIndex, payPassword);
		bytes_t votePubKey2 = SubAccountGenerator::GenerateVotePubKey(account2, coinIndex, payPassword);
		HDSubAccount hd2(mpk2, votePubKey2, account2, coinIndex);
		hd2.InitAccount({}, &lock);
		bytes_t multiSignPubKey2 = account2->GetMultiSignPublicKey();

		StandardSingleSubAccount ss2(mpk2, votePubKey2, account2, coinIndex);
		REQUIRE(multiSignPubKey2 == ss2.GetMultiSignPublicKey());

		StandardAccount *account3 = new StandardAccount("./Data", phrase3, phrasePassword, payPassword);
		HDKeychain mpk3 = SubAccountGenerator::GenerateMasterPubKey(account3, coinIndex, payPassword);
		bytes_t votePubKey3 = SubAccountGenerator::GenerateVotePubKey(account3, coinIndex, payPassword);
		HDSubAccount hd3(mpk3, votePubKey3, account3, coinIndex);
		hd3.InitAccount({}, &lock);
		bytes_t multiSignPubKey3 = account3->GetMultiSignPublicKey();

		StandardAccount *account4 = new StandardAccount("./Data", phrase4, phrasePassword, payPassword);
		HDKeychain mpk4 = SubAccountGenerator::GenerateMasterPubKey(account4, coinIndex, payPassword);
		bytes_t votePubKey4 = SubAccountGenerator::GenerateVotePubKey(account4, coinIndex, payPassword);
		HDSubAccount hd4(mpk4, votePubKey4, account4, coinIndex);
		hd4.InitAccount({}, &lock);
		bytes_t multiSignPubKey4 = account4->GetMultiSignPublicKey();

		SECTION("Standard address sign test") {
			std::vector<Address> addresses;
			hd1.GetAllAddresses(addresses, 0, 100, true);

			REQUIRE(!addresses.empty());
			bytes_t redeemScript = hd1.GetRedeemScript(addresses[addresses.size() - 1]);

			TransactionPtr tx(new Transaction);
			tx->FromJson(content);
			tx->AddProgram(Program(redeemScript, bytes_t()));


			REQUIRE_THROWS(hd3.SignTransaction(tx, payPassword));
			REQUIRE_NOTHROW(hd1.SignTransaction(tx, payPassword));
			REQUIRE(tx->IsSigned());


			std::vector<Address> ssAddresses;
			ss2.GetAllAddresses(ssAddresses, 0, 100, true);
			REQUIRE(ssAddresses.size() == 1);

			tx->GetPrograms().clear();
			redeemScript = hd2.GetRedeemScript(ssAddresses[0]);
			tx->AddProgram(Program(redeemScript, bytes_t()));
			REQUIRE_THROWS(hd1.SignTransaction(tx, payPassword));
			REQUIRE_THROWS(hd3.SignTransaction(tx, payPassword));

			REQUIRE_NOTHROW(ss2.SignTransaction(tx, payPassword));

			REQUIRE_THROWS(hd2.SignTransaction(tx, payPassword));

			REQUIRE(tx->IsSigned());
		}

		SECTION("Vote deposit address sign test") {
			std::string addr = Address(PrefixDeposit, votePubKey1).String();
			bytes_t redeemScript = hd1.GetRedeemScript(addr);

			TransactionPtr tx(new Transaction);
			tx->FromJson(content);

			tx->AddProgram(Program(redeemScript, bytes_t()));


			REQUIRE_THROWS(hd2.SignTransaction(tx, payPassword));
			REQUIRE(!tx->IsSigned());
			REQUIRE_NOTHROW(hd1.SignTransaction(tx, payPassword));
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
			MultiSignAccount *multiSignAccount = new MultiSignAccount(nullptr, coSigners, requiredSignCount);
			MultiSignSubAccount ms(multiSignAccount);
			std::vector<Address> addresses;
			ms.GetAllAddresses(addresses, 0, 1, true);
			REQUIRE(!addresses.empty());
			bytes_t redeemScript = ms.GetRedeemScript(addresses[0]);;
			tx->AddProgram(Program(redeemScript, bytes_t()));

			REQUIRE_NOTHROW(hd1.SignTransaction(tx, payPassword));
			REQUIRE(!tx->IsSigned());

			REQUIRE_NOTHROW(hd2.SignTransaction(tx, payPassword));
			REQUIRE(!tx->IsSigned());

			REQUIRE_NOTHROW(hd3.SignTransaction(tx, payPassword));
			REQUIRE(tx->IsSigned());

			REQUIRE_THROWS(hd4.SignTransaction(tx, payPassword));
			REQUIRE(tx->IsSigned());
		}

		SECTION("Multi sign account with private key test") {
			TransactionPtr tx(new Transaction);
			tx->FromJson(content);

			MultiSignAccount *multiSignAccount1 = new MultiSignAccount(account1,
				{
					multiSignPubKey2.getHex(),
					multiSignPubKey3.getHex(),
					multiSignPubKey4.getHex()
				}, requiredSignCount);
			MultiSignSubAccount ms1(multiSignAccount1);
			std::vector<Address> addresses1;
			ms1.GetAllAddresses(addresses1, 0, 10, true);

			MultiSignAccount *multiSignAccount2 = new MultiSignAccount(account2,
				{
					multiSignPubKey1.getHex(),
					multiSignPubKey3.getHex(),
					multiSignPubKey4.getHex()
				}, requiredSignCount);
			MultiSignSubAccount ms2(multiSignAccount2);
			std::vector<Address> addresses2;
			ms2.GetAllAddresses(addresses2, 0, 10, true);

			MultiSignAccount *multiSignAccount3 = new MultiSignAccount(account3,
				{
					multiSignPubKey1.getHex(),
					multiSignPubKey2.getHex(),
					multiSignPubKey4.getHex()
				}, requiredSignCount);
			MultiSignSubAccount ms3(multiSignAccount3);
			std::vector<Address> addresses3;
			ms3.GetAllAddresses(addresses3, 0, 10, true);

			MultiSignAccount *multiSignAccount4 = new MultiSignAccount(account4,
				{
					multiSignPubKey1.getHex(),
					multiSignPubKey2.getHex(),
					multiSignPubKey3.getHex()
				}, requiredSignCount);
			MultiSignSubAccount ms4(multiSignAccount4);
			std::vector<Address> addresses4;
			ms4.GetAllAddresses(addresses4, 0, 10, true);

			std::vector<std::string> coSigners;
			coSigners.push_back(multiSignPubKey1.getHex());
			coSigners.push_back(multiSignPubKey2.getHex());
			coSigners.push_back(multiSignPubKey3.getHex());
			coSigners.push_back(multiSignPubKey4.getHex());
			MultiSignAccount *multiSignAccount5 = new MultiSignAccount(nullptr, coSigners, requiredSignCount);
			MultiSignSubAccount ms5(multiSignAccount5);
			std::vector<Address> addresses5;
			ms5.GetAllAddresses(addresses5, 0, 10, true);

			REQUIRE(!addresses1.empty());
			REQUIRE(addresses1.size() == addresses2.size());
			REQUIRE(addresses1.size() == addresses3.size());
			REQUIRE(addresses1.size() == addresses4.size());
			REQUIRE(addresses1.size() == addresses5.size());
			bytes_t redeemScript = ms1.GetRedeemScript(addresses1[0]);

			tx->AddProgram(Program(redeemScript, bytes_t()));


			REQUIRE(!tx->IsSigned());
			REQUIRE_NOTHROW(ms1.SignTransaction(tx, payPassword));

			REQUIRE(!tx->IsSigned());
			REQUIRE_NOTHROW(ms2.SignTransaction(tx, payPassword));

			REQUIRE(!tx->IsSigned());
			REQUIRE_NOTHROW(ms4.SignTransaction(tx, payPassword));

			REQUIRE(tx->IsSigned());
			REQUIRE_THROWS(ms3.SignTransaction(tx, payPassword));

			REQUIRE(tx->IsSigned());
		}
	}

}