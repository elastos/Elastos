// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>
#include <catch.hpp>

#include "AddressRegisteringWallet.h"

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

class TestListener : public Wallet::Listener {
public:
	// func balanceChanged(_ balance: UInt64)
	virtual void balanceChanged(uint64_t balance) {}

	// func txAdded(_ tx: BRTxRef)
	virtual void onTxAdded(const TransactionPtr &transaction) {}

	// func txUpdated(_ txHashes: [UInt256], blockHeight: UInt32, timestamp: UInt32)
	virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {}

	// func txDeleted(_ txHash: UInt256, notifyUser: Bool, recommendRescan: Bool)
	virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {}
};

TEST_CASE("Basic test for register address", "[AddressRegisteringWallet]") {

	boost::shared_ptr<Wallet::Listener> listener(new TestListener);

	SECTION("Test empty address initializing") {

		std::vector<std::string> initialAddrs;
		boost::scoped_ptr<AddressRegisteringWallet> wallet(new AddressRegisteringWallet(
				listener, initialAddrs));

		REQUIRE(wallet->getAllAddresses().empty());
		REQUIRE(wallet->getReceiveAddress().empty());

		wallet->RegisterAddress(DefaultAddress[0]);
		REQUIRE(wallet->getAllAddresses().size() == 1);
		REQUIRE(wallet->getReceiveAddress() == DefaultAddress[0]);
	}
	SECTION("Test initialized by addresses") {
		boost::scoped_ptr<AddressRegisteringWallet> wallet(new AddressRegisteringWallet(
				listener, DefaultAddress));

		REQUIRE(wallet->getAllAddresses().size() == DefaultAddress.size());
		REQUIRE(std::find(DefaultAddress.cbegin(), DefaultAddress.cend(), wallet->getReceiveAddress()) !=
				DefaultAddress.cend());
	}
	SECTION("Ignore redundant address when registering") {
		boost::scoped_ptr<AddressRegisteringWallet> wallet(new AddressRegisteringWallet(
				listener, DefaultAddress));

		REQUIRE(wallet->getAllAddresses().size() == DefaultAddress.size());

		wallet->RegisterAddress(DefaultAddress[0]);

		REQUIRE(wallet->getAllAddresses().size() == DefaultAddress.size());
	}
	SECTION("Ignore redundant address when initializing") {
		std::vector<std::string> addressesWithRedundant = DefaultAddress;
		addressesWithRedundant.push_back(DefaultAddress[0]);
		boost::scoped_ptr<AddressRegisteringWallet> wallet(new AddressRegisteringWallet(
				listener, addressesWithRedundant));

		REQUIRE(wallet->getAllAddresses().size() == DefaultAddress.size());
	}
}