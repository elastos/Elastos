// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <BRTransaction.h>
#include "catch.hpp"
#include "Transaction.h"

using namespace Elastos::SDK;

TEST_CASE( "Transaction constructor test", "[Transaction]" ) {

	SECTION("Default constructor") {

		Transaction transaction;
		REQUIRE(transaction.getRaw() != nullptr);
	}

	SECTION("Constructor init with null pointer") {
		BRTransaction *raw = nullptr;
		Transaction transaction(raw);
		REQUIRE(transaction.getRaw() == nullptr);
		//todo should throw
	}

	SECTION("Constructor init with raw pointer") {
		BRTransaction *raw = BRTransactionNew();
		{
			Transaction transaction(raw);
			REQUIRE(transaction.getRaw() != nullptr);
			REQUIRE(transaction.getRaw() == raw);
		}
		//raw shall be invalid pointer here
	}

	SECTION("Constructor init with buffer") {
		//todo complete me
	}

	SECTION("Constructor init with buffer and 'blockHeight + timeStamp'") {
		//todo complete me
	}
}

TEST_CASE( "New empty transaction behavior", "[Transaction]" ) {

	Transaction transaction;

	SECTION("Input and related addresses") {
		REQUIRE(transaction.getInputs().empty());
		REQUIRE(transaction.getInputAddresses().empty());
	}

	SECTION("Output and related addresses") {
		REQUIRE(transaction.getOutputs().empty());
		REQUIRE(transaction.getOutputAddresses().empty());
	}

	SECTION("Should not registered") {
		REQUIRE(transaction.isRegistered() == false);
	}
}

//todo complete me