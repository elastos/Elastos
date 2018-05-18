// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TransactionInput.h"
#include "Address.h"

using namespace Elastos::SDK;

TEST_CASE( "TransactionInput test", "[TransactionInput]" )
{
	SECTION("Constructor with null and object") {
		BRTxInput *txInput = new BRTxInput();
		TransactionInput transactionInput1(txInput);
		REQUIRE(transactionInput1.getRaw() == txInput);
	}

	SECTION( "Contructor width input attributes", "[TransactionInput]" ) {
		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		uint32_t index = 8000;
		uint64_t amount = 10000;
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		CMBlock script = myaddress.getPubKeyScript();
		CMBlock signature;
		uint32_t sequence = 8888;
		TransactionInput transactionInput(hash, index, amount, script, signature, sequence);

		REQUIRE(transactionInput.getRaw() != nullptr);
		UInt256 tempHash = transactionInput.getHash();
		int result = UInt256Eq(&tempHash, &hash);
		REQUIRE(result == 1);
		REQUIRE(transactionInput.getIndex() == index);
		REQUIRE(transactionInput.getAmount() == amount);

		CMBlock tempScript = transactionInput.getScript();
		REQUIRE(tempScript.GetSize() == script.GetSize());
		for (int i = 0; i < script.GetSize(); i++) {
			REQUIRE(tempScript[i] == script[i]);
		}

		CMBlock tempSignature =  transactionInput.getSignature();
		REQUIRE(tempSignature == false);
		REQUIRE(tempSignature.GetSize() == 0);
		REQUIRE(transactionInput.getSequence() == sequence);
	}

	SECTION("TransactionInput address test", "") {
		BRTxInput *txInput = new BRTxInput();
		TransactionInput transactionInput1(txInput);

		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		transactionInput1.setAddress(content);
		REQUIRE(transactionInput1.getAddress() == content);
	}

	SECTION("TransactionInput Serialize Deserialize test") {
		BRTxInput *txInput = new BRTxInput();
		memset(txInput, 0, sizeof(BRTxInput));
		txInput->txHash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		txInput->amount = 89999;
		txInput->index = 567;
		txInput->sequence = 98888;
		TransactionInput transactionInput(txInput);

		ByteStream byteStream;
		transactionInput.Serialize(byteStream);
		byteStream.setPosition(0);
		UInt256 hash = transactionInput.getHash();
		int result = UInt256Eq(&hash, &txInput->txHash);
		REQUIRE(result == 1);
		REQUIRE(transactionInput.getAmount() == txInput->amount);
		REQUIRE(transactionInput.getSequence() == txInput->sequence);
		REQUIRE(transactionInput.getIndex() == txInput->index);

		TransactionInput transactionInput1;
		transactionInput1.Deserialize(byteStream);
		hash = transactionInput1.getHash();
		result = UInt256Eq(&hash, &txInput->txHash);
		REQUIRE(result == 1);
		REQUIRE(transactionInput1.getIndex() == txInput->index);
		REQUIRE(transactionInput1.getSequence() == txInput->sequence);
	}
}
