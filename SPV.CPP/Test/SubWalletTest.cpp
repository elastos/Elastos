// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>
#include <SDK/Common/Utils.h>
#include <Core/BRTransaction.h>
#include <SDK/Common/Log.h>

#include "catch.hpp"

#include "MasterWallet.h"
#include "SubWallet.h"

using namespace Elastos::ElaWallet;

const std::vector<std::string> DefaultAddress = {"EZcvtcsT8wXSXBTeijCdSXvT2sk62yPii5",
												 "ETjMGz4GAGxMjMaoNwhJR1ynjrEGEUDpQQ",
												 "ESAQdQkT5PKbNtpfwQmQRMkYRmUvgXp691",
												 "EeUmo1rGzS3QYvJwN5JcvDhEWnWrWHeKh4",
												 "EKWns42jGDcyywWcyW3Q6sZASabcnMeJ2b",
												 "ETcSvQZcnnSouBzTu2bVsLaxUmNAiQVVHt",
												 "EYJivg6CgYZahvSciWyMeq1x1XkkWzK4kr",
												 "EJp7CJjNHhPnJF73w2EKBcbdwHZ4apKFQ6",
												 "EVQPDnDdhZMEzT8z6xVQ2EUMtwrUXFzw9f",
												 "Eg6Yzkk2V1YgSdK4qzQiTuZohAYo28zPG8",
												 "EWijTKFpy74Z8kqjvrBF4Ku6qb952zVpzM",
												 "EM6b5VuftZWPd2kwQtZFRfuJXu1wHNSTA9",
												 "EgUPP1SsvdmH1epbPSy7ZNDtTqFyfUQrSX",
												 "EKpkNcPf11ssfk1cNcUgBx44pg7sKZ32gU",
												 "ET8H6JMCDQiKShGjshmJzTdbLYbfYkQRJ9"};

class TestMasterWallet : public MasterWallet {
public:
	TestMasterWallet() : MasterWallet(
			"MasterWalletTest",
			"abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
			"",
			"payPassword",
			"english",
			false,
			"Data") {
	}
};

class TestSubWallet : public SubWallet {
public:
	TestSubWallet(const CoinInfo &info,
				  const std::string &payPassword,
				  const ChainParams &chainParams,
				  MasterWallet *parent) :
			SubWallet(info, chainParams, payPassword, PluginType("ELA"), parent) {
	}
};

ChainParams createChainParams() {
	CoinConfig coinConfig;
	coinConfig.Type = Mainchain;
	coinConfig.NetType = "TestNet";
	return ChainParams(coinConfig);
}

#define BASIC_UINT 100000000ULL

class TestWalletManager : public SpvService {
public:
	TestWalletManager(const SpvService &parent) :
			SpvService(parent) {
	}

protected:
	virtual SharedWrapperList<Transaction, BRTransaction *> loadTransactions() {
		SharedWrapperList<Transaction, BRTransaction *> txList;

		UInt168 u168Address = UINT168_ZERO;
		Utils::UInt168FromAddress(u168Address, DefaultAddress[0]);

		ELATransaction *elaTransaction = ELATransactionNew();
		TransactionPtr tx(new Transaction(elaTransaction, false));
		elaTransaction->type = ELATransaction::CoinBase;
		elaTransaction->raw.blockHeight = 1;
		TransactionOutput *out(new TransactionOutput());
		out->setAddress(DefaultAddress[0]);
		out->setAmount(150 * BASIC_UINT);
		out->setProgramHash(u168Address);
		out->setAssetId(Key::getSystemAssetId());
		elaTransaction->outputs.push_back(out);
		elaTransaction->raw.txHash = Utils::UInt256FromString(
				"0000000000000000011111111111111111111111111111111111111111111111", true);
		// FIXME cheat TransactionIsSign(), fix this after signTransaction works fine
		CMBlock code(10);
		CMBlock parameter(10);
		Program *program = new Program(code, parameter);
		elaTransaction->programs.push_back(program);
		txList.push_back(tx);

		ELATransaction *elaTransaction1 = ELATransactionNew();
		TransactionPtr tx1(new Transaction(elaTransaction1, false));
		elaTransaction1->type = ELATransaction::CoinBase;
		elaTransaction1->raw.blockHeight = 1;
		TransactionOutput *out1 = new TransactionOutput();
		out1->setAddress(DefaultAddress[0]);
		out1->setAmount(250 * BASIC_UINT);
		out1->setProgramHash(u168Address);
		out1->setAssetId(Key::getSystemAssetId());
		elaTransaction1->outputs.push_back(out1);
		elaTransaction1->raw.txHash = Utils::UInt256FromString(
				"000000000000000002df2dd9d4fe0578392e519610e341dd09025469f101cfa1", true);
		// FIXME cheat TransactionIsSign(), fix this after signTransaction works fine
		CMBlock code1(10);
		CMBlock parameter1(10);
		Program *program1 = new Program(code1, parameter1);
		elaTransaction1->programs.push_back(program1);
		txList.push_back(tx1);

		return txList;
	}
};

class TestTransactionSubWallet : public SubWallet {
public:
	TestTransactionSubWallet(const CoinInfo &info,
							 const std::string &payPassword,
							 const ChainParams &chainParams,
							 MasterWallet *parent) :
			SubWallet(info, chainParams, payPassword, PluginType("ELA"), parent) {
		_walletManager.reset(new TestWalletManager(*_walletManager));
	}

protected:
	virtual void publishTransaction(const TransactionPtr &transaction) {

	}
};

TEST_CASE("Sub wallet basic", "[SubWallet]") {

	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet);

	CoinInfo info;
	info.setChainId("chainId");
	std::string payPassword = "payPassword";
	boost::scoped_ptr<TestSubWallet> subWallet(new TestSubWallet(info, payPassword, createChainParams(), masterWallet.get()));

	SECTION("Address related") {
		//fixme
//		nlohmann::json j = subWallet->GetAllAddress(0, INT_MAX);
//		std::vector<std::string> addresses = j["Addresses"].get<std::vector<std::string>>();
//		REQUIRE(addresses.size() == DefaultAddress.size());
//		for (int i = 0; i < addresses.size(); ++i) {
//			REQUIRE(addresses[i] == DefaultAddress[i]);
//		}
//
//		std::string newAddress = subWallet->CreateAddress(); //we did't create address actually because current addresses have not used
//		REQUIRE(!newAddress.empty());
//		REQUIRE(newAddress == DefaultAddress[SEQUENCE_GAP_LIMIT_INTERNAL]);
//
//		nlohmann::json j2 = subWallet->GetAllAddress(0, INT_MAX);
//		std::vector<std::string> addresses2 = j2["Addresses"].get<std::vector<std::string>>();
//		REQUIRE(addresses.size() == addresses2.size());
	}
	SECTION("Balance related") {
		REQUIRE(subWallet->GetBalance() == 0);

		nlohmann::json balanceInfo = subWallet->GetBalanceInfo();
		std::vector<nlohmann::json> balanceList = balanceInfo["Balances"];
		REQUIRE(balanceList.empty());

		std::string newAddress = subWallet->CreateAddress();
		REQUIRE(!newAddress.empty());
		REQUIRE(subWallet->GetBalanceWithAddress(newAddress) == 0);
	}
}

TEST_CASE("Sub wallet with single address", "[SubWallet]") {
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet);

	CoinInfo info;
	info.setChainId("chainId");
	info.setSingleAddress(true);
	std::string payPassword = "payPassword";
	boost::scoped_ptr<TestSubWallet> subWallet(new TestSubWallet(info, payPassword, createChainParams(), masterWallet.get()));

	SECTION("Address related") {
		nlohmann::json j = subWallet->GetAllAddress(0, INT_MAX);
		std::vector<std::string> addresses = j["Addresses"].get<std::vector<std::string>>();
		REQUIRE(addresses.size() == 1);
		REQUIRE(addresses[0] == "ETcSvQZcnnSouBzTu2bVsLaxUmNAiQVVHt");

		std::string newAddress = subWallet->CreateAddress(); //we did't create address actually because current addresses have not used
		REQUIRE(!newAddress.empty());
		REQUIRE(newAddress == addresses[0]);

		nlohmann::json j2 = subWallet->GetAllAddress(0, INT_MAX);
		std::vector<std::string> addresses2 = j2["Addresses"].get<std::vector<std::string>>();
		REQUIRE(addresses.size() == addresses2.size());
	}
	SECTION("Balance related") {
		REQUIRE(subWallet->GetBalance() == 0);

		nlohmann::json balanceInfo = subWallet->GetBalanceInfo();
		std::vector<nlohmann::json> balanceList = balanceInfo["Balances"];
		REQUIRE(balanceList.empty());

		std::string newAddress = subWallet->CreateAddress();
		REQUIRE(!newAddress.empty());
		REQUIRE(subWallet->GetBalanceWithAddress(newAddress) == 0);
	}
}

TEST_CASE("Sub wallet send transaction", "SubWallet") {
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet);


	CoinInfo info;
	info.setChainId("chainId");
	info.setSingleAddress(false);
	info.setMinFee(BASIC_UINT);
	std::string payPassword = "payPassword";
	boost::scoped_ptr<TestTransactionSubWallet> subWallet(
			new TestTransactionSubWallet(info, payPassword, createChainParams(), masterWallet.get()));

	SECTION("Create and Send transaction") {
		REQUIRE(subWallet->GetBalance() == 400 * BASIC_UINT);

		nlohmann::json result;
		nlohmann::json txJson;
		std::string emptyHash = Utils::UInt256ToString(UINT256_ZERO, true);
		CHECK_NOTHROW(subWallet->CreateTransaction("", "ERcEon7MC8fUBZSadvCUTVYmdHyRK1Jork",
				50 * BASIC_UINT, "", ""));

		CHECK_THROWS_AS(subWallet->CreateTransaction("EZcvtcsT8wXSXBTeijCdSXvT2sk62yPii5", "",
		                                             50 * BASIC_UINT, "", ""), std::logic_error);

		CHECK_NOTHROW(txJson = subWallet->CreateTransaction("EZcvtcsT8wXSXBTeijCdSXvT2sk62yPii5",
				"ERcEon7MC8fUBZSadvCUTVYmdHyRK1Jork", 50 * BASIC_UINT, "", ""));

		REQUIRE(txJson["TxHash"].get<std::string>() != emptyHash);

		CHECK_NOTHROW(result = subWallet->SendRawTransaction(txJson, BASIC_UINT, payPassword));

		REQUIRE(result["TxHash"].get<std::string>() != emptyHash);
		REQUIRE(result["Fee"].get<uint64_t>() == BASIC_UINT);
	}

	SECTION("send raw transaction") {

	}
}

TEST_CASE("Sub wallet check sign", "[CheckSign]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet());
	std::string message = "mymessage";
	ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", payPassword, false);
	std::string signedData = subWallet->Sign(message, payPassword);

	SECTION("Normal check sign") {
		nlohmann::json j = subWallet->CheckSign(subWallet->GetPublicKey(), message, signedData);
		REQUIRE(j["Result"].get<bool>());
	}
	SECTION("Check sign with wrong message") {
		nlohmann::json j = subWallet->CheckSign(subWallet->GetPublicKey(), "wrongMessage", signedData);
		REQUIRE_FALSE(j["Result"].get<bool>());
	}
	SECTION("Check sign with wrong signed data") {
		REQUIRE_THROWS_AS(subWallet->CheckSign(subWallet->GetPublicKey(), message, "wrangData"), std::logic_error);
	}
}