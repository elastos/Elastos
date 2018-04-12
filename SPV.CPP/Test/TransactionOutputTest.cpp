// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "TransactionOutput.h"
#include "BRAddress.h"

using namespace Elastos::SDK;

TEST_CASE( "TransactionOutput test", "[TransactionOutput]" )
{
	SECTION("constructor with null and object") {
		TransactionOutput transactionOutput(nullptr);
		REQUIRE(transactionOutput.getRaw() == nullptr);

		BRTxOutput* brTxOutput = new BRTxOutput();
		TransactionOutput transactionOutput1(brTxOutput);
		REQUIRE(transactionOutput1.getRaw() != nullptr);
	}

	SECTION("constructor with amount and script") {
		uint8_t dummyScript[] = { OP_DUP, OP_HASH160, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                                0, 0, 0, 0, 0, 0, 0, 0, 0, OP_EQUALVERIFY, OP_CHECKSIG };

		TransactionOutput transactionOutput(10000,ByteData(dummyScript,sizeof(dummyScript)));

		REQUIRE(transactionOutput.getRaw() != nullptr);
		REQUIRE(transactionOutput.getAmount() == 10000);
		REQUIRE(transactionOutput.getScript().length == sizeof(dummyScript));

		uint8_t *temp = transactionOutput.getScript().data;
		for(int i = 0; i < sizeof(dummyScript); i++) {
			REQUIRE(temp[i] == dummyScript[i]);
		}
	}

	SECTION("TransactionOutput address test", "") {
		BRTxOutput* brTxOutput = new BRTxOutput();
		TransactionOutput transactionOutput(brTxOutput);

		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		transactionOutput.setAddress(content);
		REQUIRE(transactionOutput.getAddress() == content);
	}
}
