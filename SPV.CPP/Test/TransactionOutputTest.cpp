// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "SDK/Transaction/TransactionOutput.h"
#include "BRAddress.h"
#include <SDK/Common/Utils.h>

using namespace Elastos::ElaWallet;

TEST_CASE("TransactionOutput test", "[TransactionOutput]") {

	SECTION("constructor with amount and script") {
		uint8_t dummyScript[] = {OP_DUP, OP_HASH160, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                         0, 0, 0, 0, 0, 0, 0, 0, 0, OP_EQUALVERIFY, OP_CHECKSIG};

		CMBlock mb;
		mb.SetMemFixed(dummyScript, sizeof(dummyScript));
		TransactionOutput transactionOutput(10000, mb, ELA_STANDARD);

		REQUIRE(transactionOutput.getAmount() == 10000);
		REQUIRE(transactionOutput.getScript().GetSize() == sizeof(dummyScript));

		CMBlock temp = transactionOutput.getScript();
		for (size_t i = 0; i < sizeof(dummyScript); i++) {
			uint8_t t = temp[i];
			REQUIRE(dummyScript[i] == t);
		}
	}

	SECTION("TransactionOutput address test", "") {
		TransactionOutput transactionOutput;

		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		transactionOutput.setAddress(content);
		std::string address = transactionOutput.getAddress();
		REQUIRE(address == content);
	}

	SECTION("Serialize and deserialize test", "") {
		TransactionOutput transactionOutput;

		UInt168 hash = *(UInt168 *) "\x21\xc2\xe2\x51\x72\xcb\x15\x19\x3c\xb1\xc6"
						"\xd4\x8f\x60\x7d\x42\xc1\xd2\xa2\x15\x16";
		transactionOutput.setAmount(11);
		transactionOutput.setOutputLock(33);
		transactionOutput.setProgramHash(hash);


		ByteStream s;
		transactionOutput.Serialize(s);

		s.setPosition(0);

		TransactionOutput transactionOutput1;

		transactionOutput1.Deserialize(s);

		REQUIRE(transactionOutput.getAmount() == transactionOutput1.getAmount());

		REQUIRE(transactionOutput.getOutputLock() == transactionOutput1.getOutputLock());

		UInt168 hash1 = transactionOutput1.getProgramHash();
		int r = UInt168Eq(&hash1, &hash);
		REQUIRE(r == 1);

	}

}
