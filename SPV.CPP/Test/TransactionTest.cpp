// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/Payload/PayloadCoinBase.h>
#include <SDK/Plugin/Transaction/Attribute.h>
#include <SDK/Base/Address.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <Core/BRTransaction.h>
#include <Core/BRTransaction.h>

using namespace Elastos::ElaWallet;


TEST_CASE("Transaction Serialize and Deserialize", "[Transaction]") {
	srand(time(nullptr));

	SECTION("transaction Serialize test") {
		Transaction tx1;
		initTransaction(tx1, Transaction::TxVersion::Default);

		ByteStream stream;
		tx1.Serialize(stream);

		Transaction tx2;
		stream.setPosition(0);
		REQUIRE(tx2.Deserialize(stream));

		verifyTransaction(tx1, tx2, false);

		tx2 = tx1;

		verifyTransaction(tx1, tx2, true);
	}

}

TEST_CASE("Convert to and from json", "[Transaction]") {
	srand(time(nullptr));

	SECTION("to and from json") {
		Transaction tx1;

		initTransaction(tx1, Transaction::TxVersion::V09);

		nlohmann::json txJson = tx1.toJson();

		Transaction tx2;
		tx2.fromJson(txJson);

		verifyTransaction(tx1, tx2, true);
	}
}

TEST_CASE("public function test", "[Transaction]") {
	srand(time(nullptr));

	SECTION("removeDuplicatePrograms test") {
		SECTION("situation 1") {
			Transaction tx;
			for (size_t i = 0; i < 20; ++i) {
				CMBlock code = getRandCMBlock(25);
				CMBlock parameter = getRandCMBlock(25);
				tx.addProgram(Program(code, parameter));
			}

			REQUIRE(tx.getPrograms().size() == 20);

			tx.addProgram(tx.getPrograms()[0]);
			tx.addProgram(tx.getPrograms()[1]);
			tx.addProgram(tx.getPrograms()[1]);
			tx.addProgram(tx.getPrograms()[3]);
			tx.addProgram(tx.getPrograms()[5]);
			tx.addProgram(tx.getPrograms()[7]);
			tx.addProgram(tx.getPrograms()[19]);

			REQUIRE(tx.getPrograms().size() == 27);

			for (size_t i = 0; i < 20; ++i) {
				CMBlock code = getRandCMBlock(25);
				CMBlock parameter = getRandCMBlock(25);
				tx.addProgram(Program(code, parameter));
			}

			REQUIRE(tx.getPrograms().size() == 47);

			tx.removeDuplicatePrograms();

			REQUIRE(tx.getPrograms().size() == 40);
		}

		SECTION("situation 2") {
			Transaction tx;
			for (size_t i = 0; i < 20; ++i) {
				CMBlock code = getRandCMBlock(25);
				CMBlock parameter = getRandCMBlock(25);
				tx.addProgram(Program(code, parameter));
			}

			REQUIRE(tx.getPrograms().size() == 20);

			for (size_t i = 0; i < 20; ++i) {
				tx.addProgram(tx.getPrograms()[i]);
			}

			REQUIRE(tx.getPrograms().size() == 40);

			tx.removeDuplicatePrograms();

			REQUIRE(tx.getPrograms().size() == 20);
		}

		SECTION("situation 3") {
			Transaction tx;
			for (size_t i = 0; i < 20; ++i) {
				CMBlock code = getRandCMBlock(25);
				CMBlock parameter = getRandCMBlock(25);
				tx.addProgram(Program(code, parameter));
			}

			REQUIRE(tx.getPrograms().size() == 20);

			tx.removeDuplicatePrograms();

			REQUIRE(tx.getPrograms().size() == 20);
		}

	}
}
