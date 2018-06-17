// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <iostream>

#include <catch.hpp>
#include <Core/BRTransaction.h>

#include "Utils.h"
#include "BRBIP39Mnemonic.h"
#include "SingleAddressWallet.h"

using namespace Elastos::SDK;

class TestListener : public Wallet::Listener {
public:
	virtual void balanceChanged(uint64_t balance) {
		std::cout << balance << std::endl;
	}

	virtual void onTxAdded(const TransactionPtr &transaction) {
		std::cout << "Added: " << Utils::UInt256ToString(transaction->getHash()) << std::endl;
	}

	virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
		std::cout << "Updated: " << hash << std::endl;
	}

	virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
		std::cout << "Deleted: " << hash << std::endl;
	}
};

MasterPubKeyPtr createDummyPublicKey() {
	UInt512 seed;
	std::string phrase = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	BRBIP39DeriveKey(seed.u8, phrase.c_str(), "");

	BRKey key;
	UInt256 chainCode;
	Key::deriveKeyAndChain(&key, chainCode, &seed, sizeof(seed), 3, 44, 0, 0);

	MasterPubKeyPtr masterPubKey(new MasterPubKey(key, chainCode));
	return masterPubKey;
}

TEST_CASE("Single address wallet Constructor method", "[Constructor]") {

	boost::shared_ptr<Wallet::Listener> listener(new TestListener);

	SECTION("Normal procedure") {
		SharedWrapperList<Transaction, BRTransaction *> transactions;

		SingleAddressWallet singleAddressWallet(transactions, createDummyPublicKey(), listener);

		std::vector<std::string> addresses = singleAddressWallet.getAllAddresses();
		REQUIRE(addresses.size() == 1);
		REQUIRE(addresses[0] == "ELR8gBDqJoF3CxyqTT1qxPHqgJUUcd6o8V");

		REQUIRE(singleAddressWallet.getBalance() == 0);
	}
	SECTION("Initialize with transactions") {
		SharedWrapperList<Transaction, BRTransaction *> transactions;

		ELATransaction *transaction = new ELATransaction;
		TransactionOutputPtr output(new TransactionOutput);
		output->setAmount(100000000);
		output->setAddress("ELR8gBDqJoF3CxyqTT1qxPHqgJUUcd6o8V");
		transaction->outputs.push_back(output);
		transaction->raw.txHash = UINT256_ZERO;
		transactions.push_back(TransactionPtr(new Transaction(transaction, false)));

		ELATransaction *transaction2 = new ELATransaction;
		TransactionOutputPtr output2(new TransactionOutput);
		output2->setAmount(200000000);
		output2->setAddress("ELR8gBDqJoF3CxyqTT1qxPHqgJUUcd6o8V");
		transaction2->outputs.push_back(output2);
		transaction2->raw.txHash = Utils::UInt256FromString(
				"000000000000000002df2dd9d4fe0578392e519610e341dd09025469f101cfa1");
		transactions.push_back(TransactionPtr(new Transaction(transaction2, false)));

		SingleAddressWallet singleAddressWallet(transactions, createDummyPublicKey(), listener);

		std::vector<std::string> addresses = singleAddressWallet.getAllAddresses();
		REQUIRE(addresses.size() == 1);
		REQUIRE(addresses[0] == "ELR8gBDqJoF3CxyqTT1qxPHqgJUUcd6o8V");

		REQUIRE(singleAddressWallet.getBalance() == 300000000);
	}
}

TEST_CASE("Single address wallet transaction related method", "[register,]") {

	boost::shared_ptr<Wallet::Listener> listener(new TestListener);
	SharedWrapperList<Transaction, BRTransaction *> transactions;
	SingleAddressWallet singleAddressWallet(transactions, createDummyPublicKey(), listener);

	ELATransaction *transaction = new ELATransaction;
	TransactionOutputPtr output(new TransactionOutput);
	output->setAmount(100000000);
	output->setAddress("ELR8gBDqJoF3CxyqTT1qxPHqgJUUcd6o8V");
	transaction->outputs.push_back(output);
	transaction->raw.txHash = UINT256_ZERO;
	REQUIRE(singleAddressWallet.registerTransaction(TransactionPtr(new Transaction(transaction, false))));

	REQUIRE(singleAddressWallet.getAllAddresses().size() == 1);
	REQUIRE(singleAddressWallet.getBalance() == 100000000);

	REQUIRE(singleAddressWallet.registerTransaction(TransactionPtr(new Transaction(transaction)))); //register the same transaction again
	REQUIRE(singleAddressWallet.getAllAddresses().size() == 1);

	//register the second transaction
	ELATransaction *transaction2 = new ELATransaction;
	TransactionOutputPtr output2(new TransactionOutput);
	output2->setAmount(200000000);
	output2->setAddress("ELR8gBDqJoF3CxyqTT1qxPHqgJUUcd6o8V");
	transaction2->outputs.push_back(output2);
	transaction2->raw.txHash = Utils::UInt256FromString(
			"000000000000000002df2dd9d4fe0578392e519610e341dd09025469f101cfa1");
	REQUIRE(singleAddressWallet.registerTransaction(TransactionPtr(new Transaction(transaction2))));

	REQUIRE(singleAddressWallet.getBalance() == 300000000);
}
