// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>
#include <SDK/Common/Utils.h>
#include <Core/BRTransaction.h>
#include <SDK/Common/Log.h>
#include <Interface/Enviroment.h>

#include "catch.hpp"

#include "MasterWallet.h"
#include "SubWallet.h"

using namespace Elastos::SDK;

const std::vector<std::string> DefaultAddress = {"EZuWALdKM92U89NYAN5DDP5ynqMuyqG5i3",
												 "EgSMqA8v4RJYyHareuXcFULKFjx2jNK9Zs",
												 "ERbn5LLwuhA3v7kpYBV7cH4vh99uhGZgtR",
												 "EenCqPwkAzwE3yKFQqJxkT64zBYDSMmT9j",
												 "EfD3zSQUtA6zqzukFvUSbN8WWbHZXSjxvZ",
												 "ELR8gBDqJoF3CxyqTT1qxPHqgJUUcd6o8V",
												 "ENZbHPvhXwTePRv4hB8mKCVnqXbgKYUYAv",
												 "EU2s3Xar8n9cccKv6jLJ2dbu4QFiEm5xys",
												 "EKhvgt41SDcf61u4CGzPvakoKK3ErBXr8x",
												 "EX4X6ZxkrS7Drm4mDUS1NXxBWapmBhNTWF",
												 "EMLnMNpgWmdnhn6r91wYisHaEdNrw6v57j",
												 "EXGtGgwYWEFkgPs8b7gCRTKbf9Ufts1Yio",
												 "EJ98iA2ooCRZ6piM3v3ECqNAUsdQczuZ4w",
												 "EfWxKV4JBbRt657u3ejHJL7YB6orDGVpN5",
												 "EX51M5fNG5P9NkKTtN49FYnvY8xxprbsaH"};

class TestMasterWallet : public MasterWallet {
public:
	TestMasterWallet() : MasterWallet("MasterWalletTest", "english") {
		std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
		std::string phrasePassword = "";
		std::string payPassword = "payPassword";
		initFromPhrase(mnemonic, phrasePassword, payPassword);
	}

protected:
	virtual void startPeerManager(SubWallet *wallet) {
	}

	virtual void stopPeerManager(SubWallet *wallet) {
	}
};

class TestSubWallet : public SubWallet {
public:
	TestSubWallet(const CoinInfo &info,
				  const std::string &payPassword,
				  MasterWallet *parent) :
			SubWallet(info, ChainParams::mainNet(), payPassword, parent) {
	}
};

#define BASIC_UINT 100000000ULL

class TestWalletManager : public WalletManager {
public:
	TestWalletManager(const WalletManager &parent) :
		WalletManager(parent) {
	}

protected:
	virtual SharedWrapperList<Transaction, BRTransaction *> loadTransactions() {
		SharedWrapperList<Transaction, BRTransaction *> txList;

		UInt168 u168Address = UINT168_ZERO;
		Utils::UInt168FromAddress(u168Address, DefaultAddress[0]);

		ELATransaction *elaTransaction = ELATransactionNew();
		TransactionPtr tx(new Transaction(elaTransaction, false));
		elaTransaction->type = ELATransaction::CoinBase;
		TransactionOutputPtr out(new TransactionOutput());
		out->setAddress(DefaultAddress[0]);
		out->setAmount(150 * BASIC_UINT);
		out->setProgramHash(u168Address);
		elaTransaction->outputs.push_back(out);
		elaTransaction->raw.txHash = UINT256_ZERO;
		// FIXME cheat TransactionIsSign(), fix this after signTransaction works fine
		CMBlock code(10);
		CMBlock parameter(10);
		ProgramPtr program(new Program(code, parameter));
		elaTransaction->programs.push_back(program);
		txList.push_back(tx);

		ELATransaction *elaTransaction1 = ELATransactionNew();
		TransactionPtr tx1(new Transaction(elaTransaction1, false));
		elaTransaction1->type = ELATransaction::CoinBase;
		TransactionOutputPtr out1(new TransactionOutput());
		out1->setAddress(DefaultAddress[0]);
		out1->setAmount(250 * BASIC_UINT);
		out1->setProgramHash(u168Address);
		elaTransaction1->outputs.push_back(out1);
		elaTransaction1->raw.txHash = Utils::UInt256FromString(
			"000000000000000002df2dd9d4fe0578392e519610e341dd09025469f101cfa1");
		// FIXME cheat TransactionIsSign(), fix this after signTransaction works fine
		CMBlock code1(10);
		CMBlock parameter1(10);
		ProgramPtr program1(new Program(code1, parameter1));
		elaTransaction1->programs.push_back(program1);
		txList.push_back(tx1);

		return txList;
	}
};

class TestTransactionSubWallet : public SubWallet {
public:
	TestTransactionSubWallet(const CoinInfo &info,
				  const std::string &payPassword,
				  MasterWallet *parent) :
			SubWallet(info, ChainParams::mainNet(), payPassword, parent) {
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
	boost::scoped_ptr<TestSubWallet> subWallet(new TestSubWallet(info, payPassword, masterWallet.get()));

	SECTION("Address related") {
		nlohmann::json j = subWallet->GetAllAddress(0, INT_MAX);
		std::vector<std::string> addresses = j["Addresses"].get<std::vector<std::string>>();

		REQUIRE(addresses.size() == DefaultAddress.size());
		for (int i = 0; i < addresses.size(); ++i) {
			REQUIRE(addresses[i] == DefaultAddress[i]);
		}

		std::string newAddress = subWallet->CreateAddress(); //we did't create address actually because current addresses have not used
		REQUIRE(!newAddress.empty());
		REQUIRE(newAddress == DefaultAddress[SEQUENCE_GAP_LIMIT_INTERNAL]);

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

TEST_CASE("Sub wallet with single address", "[SubWallet]") {
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet);

	CoinInfo info;
	info.setChainId("chainId");
	info.setSingleAddress(true);
	std::string payPassword = "payPassword";
	boost::scoped_ptr<TestSubWallet> subWallet(new TestSubWallet(info, payPassword, masterWallet.get()));

	SECTION("Address related") {
		nlohmann::json j = subWallet->GetAllAddress(0, INT_MAX);
		std::vector<std::string> addresses = j["Addresses"].get<std::vector<std::string>>();

		REQUIRE(addresses.size() == 1);
		REQUIRE(addresses[0] == "ELR8gBDqJoF3CxyqTT1qxPHqgJUUcd6o8V");

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

	Enviroment::InitializeRootPath("Data");
	CoinInfo info;
	info.setChainId("chainId");
	info.setSingleAddress(false);
	std::string payPassword = "payPassword";
	boost::scoped_ptr<TestTransactionSubWallet> subWallet(new TestTransactionSubWallet(info, payPassword, masterWallet.get()));

	SECTION("Send transaction") {
		REQUIRE(subWallet->GetBalance() == 400 * BASIC_UINT);

		nlohmann::json result;
		std::string emptyHash = Utils::UInt256ToString(UINT256_ZERO);

		CHECK_THROWS_AS(subWallet->SendTransaction("", "ERcEon7MC8fUBZSadvCUTVYmdHyRK1Jork",
												   50 * BASIC_UINT, BASIC_UINT, payPassword, ""), std::logic_error);
		CHECK_THROWS_AS(subWallet->SendTransaction("EZuWALdKM92U89NYAN5DDP5ynqMuyqG5i3", "",
												   50 * BASIC_UINT, BASIC_UINT, payPassword, ""), std::logic_error);

		CHECK_NOTHROW(result = subWallet->SendTransaction("EZuWALdKM92U89NYAN5DDP5ynqMuyqG5i3", "ERcEon7MC8fUBZSadvCUTVYmdHyRK1Jork",
												   50 * BASIC_UINT, 0, payPassword, ""));
		REQUIRE(result["TxHash"].get<std::string>() != emptyHash);
		REQUIRE(result["Fee"].get<uint64_t>() != 0);

		CHECK_NOTHROW(result = subWallet->SendTransaction("EZuWALdKM92U89NYAN5DDP5ynqMuyqG5i3",
												   "ERcEon7MC8fUBZSadvCUTVYmdHyRK1Jork",
												   50 * BASIC_UINT, BASIC_UINT, payPassword, ""));

		REQUIRE(result["TxHash"].get<std::string>() != emptyHash);
		REQUIRE(result["Fee"].get<uint64_t>() == BASIC_UINT);
	}

	SECTION("send raw transaction") {

	}
}