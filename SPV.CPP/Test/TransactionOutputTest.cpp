// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "ELABRTxOutput.h"
#include "catch.hpp"
#include "TransactionOutput.h"
#include "BRAddress.h"

using namespace Elastos::SDK;

TEST_CASE("TransactionOutput test", "[TransactionOutput]") {
	SECTION("constructor with null and object") {
		BRTxOutput *brTxOutput = new BRTxOutput();
		TransactionOutput transactionOutput1(brTxOutput);
		REQUIRE(transactionOutput1.getRaw() != nullptr);
	}

	SECTION("constructor with amount and script") {
		uint8_t dummyScript[] = {OP_DUP, OP_HASH160, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                         0, 0, 0, 0, 0, 0, 0, 0, 0, OP_EQUALVERIFY, OP_CHECKSIG};

		CMBlock mb;
		mb.SetMemFixed(dummyScript, sizeof(dummyScript));
		TransactionOutput transactionOutput(10000, mb);

		REQUIRE(transactionOutput.getRaw() != nullptr);
		REQUIRE(transactionOutput.getAmount() == 10000);
		REQUIRE(transactionOutput.getScript().GetSize() == sizeof(dummyScript));

		CMBlock temp = transactionOutput.getScript();
		for (uint64_t i = 0; i < sizeof(dummyScript); i++) {
			uint8_t t = temp[i];
			REQUIRE(dummyScript[i] == t);
		}
	}

	SECTION("TransactionOutput address test", "") {
		BRTxOutput *brTxOutput = new BRTxOutput();
		TransactionOutput transactionOutput(brTxOutput);

		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		transactionOutput.setAddress(content);
		REQUIRE(transactionOutput.getAddress() == content);
	}

	SECTION("Serialize and deserialize test", "") {
//		uint8_t assetIdData[256 / 8];
//		UInt256Set(assetIdData, _assetId);
//		ostream << assetIdData;
//
//		uint8_t amountData[64 / 8];
//		UInt64SetLE(amountData, _output->amount);
//		ostream << amountData;
//
//		uint8_t outputLockData[32 / 8];
//		UInt32SetLE(outputLockData, _outputLock);
//		ostream << outputLockData;
//
//		uint8_t programHashData[168 / 8];
//		UInt168Set(programHashData, _programHash);
//		ostream << programHashData;
		BRTxOutput *brTxOutput = new BRTxOutput();
		TransactionOutput transactionOutput(brTxOutput);

		transactionOutput.setAmount(11);
		transactionOutput.setOutputLock(33);


		ByteStream s;
		transactionOutput.Serialize(s);

		s.setPosition(0);

		BRTxOutput *brTxOutput1 = new BRTxOutput();
		TransactionOutput transactionOutput1(brTxOutput1);

		transactionOutput1.Deserialize(s);

		REQUIRE(transactionOutput.getOutputLock() == transactionOutput1.getOutputLock());


	}

	SECTION("TransactionOutput convertToRaw test") {
		UInt168 hash = *(UInt168 *) "\x21\xc2\xe2\x51\x72\xcb\x15\x19\x3c\xb1\xc6"
				"\xd4\x8f\x60\x7d\x42\xc1\xd2\xa2\x15\x16";
		UInt256 assetHash = uint256("0000000000000000008e5d72027ef42ca050a0776b7184c96d0d4b300fa5da9e");
		uint8_t dummyScript[] = {OP_DUP, OP_HASH160, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                         0, 0, 0, 0, 0, 0, 0, 0, 0, OP_EQUALVERIFY, OP_CHECKSIG};
		ELABRTxOutput *elabrTxOutput = new ELABRTxOutput();
		for (int i = 0; i < 75; i++) {
			elabrTxOutput->raw.address[i] = i;
		}
		elabrTxOutput->programHash = hash;
		elabrTxOutput->outputLock = 666;
		elabrTxOutput->raw.amount = 888;
		elabrTxOutput->raw.script = dummyScript;
		elabrTxOutput->raw.scriptLen = sizeof(dummyScript);
		elabrTxOutput->assetId = assetHash;

		TransactionOutput transactionOutput((BRTxOutput *) elabrTxOutput);

		BRTxOutput *brTxOutput = transactionOutput.getRaw();
		REQUIRE(brTxOutput == (BRTxOutput *) elabrTxOutput);
		REQUIRE(brTxOutput->amount == elabrTxOutput->raw.amount);
		REQUIRE(elabrTxOutput->raw.scriptLen > 0);
		REQUIRE(brTxOutput->scriptLen == elabrTxOutput->raw.scriptLen);
		int result = memcmp(brTxOutput->script, elabrTxOutput->raw.script, brTxOutput->scriptLen);
		REQUIRE(result == 0);

		ELABRTxOutput *elabrTxOutput1 = (ELABRTxOutput *) brTxOutput;
		REQUIRE(elabrTxOutput1->outputLock == elabrTxOutput->outputLock);
		result = UInt256Eq(&elabrTxOutput1->assetId, &elabrTxOutput->assetId);
		REQUIRE(result == 1);
		result = UInt168Eq(&elabrTxOutput1->programHash, &elabrTxOutput->programHash);
		REQUIRE(result == 1);

		ELABRTxOutput *elabrTxOutput2 = (ELABRTxOutput *) transactionOutput.convertToRaw();
		REQUIRE(elabrTxOutput2->raw.amount == elabrTxOutput->raw.amount);
		REQUIRE(elabrTxOutput2->raw.scriptLen == elabrTxOutput->raw.scriptLen);
		result = memcmp(elabrTxOutput2->raw.script, elabrTxOutput->raw.script, elabrTxOutput->raw.scriptLen);
		REQUIRE(result == 0);
		REQUIRE(elabrTxOutput2->outputLock == elabrTxOutput->outputLock);
		result = UInt256Eq(&elabrTxOutput2->assetId, &elabrTxOutput->assetId);
		REQUIRE(result == 1);
		result = UInt168Eq(&elabrTxOutput2->programHash, &elabrTxOutput->programHash);
		REQUIRE(result == 1);
	}

}
