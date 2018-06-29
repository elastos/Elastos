// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>

#include "catch.hpp"
#include "nlohmann/json.hpp"

#include "MasterWallet.h"
#include "WalletManager.h"
#include "MainchainSubWallet.h"

#define BASIC_UINT 100000000

using namespace Elastos::ElaWallet;

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
	TestMasterWallet() : MasterWallet(
			"MasterWalletTest",
			"abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
			"",
			"payPassword",
			"english",
			false,
			"Data") {
	}

protected:
	virtual void startPeerManager(SubWallet *wallet) {
	}

	virtual void stopPeerManager(SubWallet *wallet) {
	}
};

class TestWalletManager : public WalletManager {
public:
	TestWalletManager(const WalletManager &parent) :
			WalletManager(parent) {
	}

protected:
	virtual SharedWrapperList<Transaction, BRTransaction *> loadTransactions() {
		SharedWrapperList<Transaction, BRTransaction *> txList;

		TransactionPtr tx(new Transaction());
		ELATransaction *elaTransaction = (ELATransaction *) tx->getRaw();
		elaTransaction->type = ELATransaction::CoinBase;
		TransactionOutput *out = new TransactionOutput();
		out->setAddress(DefaultAddress[0]);
		out->setAmount(100 * BASIC_UINT);
		elaTransaction->outputs.push_back(out);
		txList.push_back(tx);

		TransactionPtr tx1(new Transaction());
		ELATransaction *elaTransaction1 = (ELATransaction *) tx->getRaw();
		elaTransaction1->type = ELATransaction::CoinBase;
		TransactionOutput *out1 = new TransactionOutput();
		out1->setAddress(DefaultAddress[0]);
		out1->setAmount(100 * BASIC_UINT);
		elaTransaction1->outputs.push_back(out1);
		txList.push_back(tx1);

		return txList;
	}
};

class TestMainchainSubWallet : public MainchainSubWallet {
public:
	TestMainchainSubWallet(const CoinInfo &info,
						   const std::string &payPassword,
						   const ChainParams &chainParams,
						   MasterWallet *parent) :
			MainchainSubWallet(info, chainParams, payPassword, PluginTypes("ELA"), parent) {
		_walletManager.reset(new TestWalletManager(*_walletManager));
	}

protected:
	virtual void publishTransaction(const TransactionPtr &transaction) {

	}
};

ChainParams createChainParams() {
	CoinConfig coinConfig;
	coinConfig.TargetTimeSpan = 86400;
	coinConfig.TargetTimePerBlock = 120;
	coinConfig.StandardPort = 20866;
	coinConfig.MagicNumber = 7630401;
	coinConfig.Services = 0;
	return ChainParams(coinConfig);
}

TEST_CASE("Mainchain sub wallet SendDepositTransaction method", "[SendDepositTransaction]") {
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet);

	std::string toAddress = "XQd1DCi6H62NQdWZQhJCRnrPn7sF9CTjaU";
	std::string sideChainAddress = "EKVtp6DxobFcUjvHVrDhnVgnpZVAacgv7s";

	CoinInfo info;
	std::string payPassword = "payPassword";

	SECTION("Normal send with null from address") {
		//todo complete me
//		boost::scoped_ptr<TestMainchainSubWallet> subWallet(
//				new TestMainchainSubWallet(info, payPassword, masterWallet.get()));
//
//		nlohmann::json sidechainAccounts;
//		sidechainAccounts.push_back(sideChainAddress);
//		nlohmann::json sidechainAmounts;
//		sidechainAmounts.push_back(9 * BASIC_UINT);
//		nlohmann::json sidechainIndexs;
//		sidechainIndexs.push_back(0);
//
//		nlohmann::json result = subWallet->SendDepositTransaction(
//				"", toAddress, 10 * BASIC_UINT, sidechainAccounts, sidechainAmounts, sidechainIndexs, BASIC_UINT,
//				payPassword, "");
//		REQUIRE(result["Fee"].get<uint64_t>() == BASIC_UINT);
	}
	SECTION("Normal send with specified address") {

	}
}

TEST_CASE("Mainchain sub wallet SendRawTransaction method", "[SendRawTransaction]") {
	boost::scoped_ptr<TestMasterWallet> masterWallet(new TestMasterWallet);

	CoinInfo info;
	std::string payPassword = "payPassword";

	SECTION("Normal send with null from address") {
		//todo complete me
//		nlohmann::json j = R"(
//		  {
//			"Version": 1,
//			"LockTime": 0,
//			"Outputs": [
//				{
//					"Address": "XQd1DCi6H62NQdWZQhJCRnrPn7sF9CTjaU",
//					"Amount": 1000000000
//				}
//			],
//			"PayLoad": [
//				{
//					"CrossChainAddress": "EKVtp6DxobFcUjvHVrDhnVgnpZVAacgv7s",
//					"CrossChainAmount": 900000000,
//					"OutputIndex": 0
//				}
//			]
//		  }
//		)"_json;
//
//		boost::scoped_ptr<TestMainchainSubWallet> subWallet(
//				new TestMainchainSubWallet(info, payPassword, masterWallet.get()));
//		nlohmann::json result = subWallet->SendRawTransaction(j, payPassword);
//		REQUIRE(result["Fee"].get<uint64_t>() == BASIC_UINT);
	}
	SECTION("Should throw if transaction type is not Mainchain") {
		//todo complete me
	}
}
