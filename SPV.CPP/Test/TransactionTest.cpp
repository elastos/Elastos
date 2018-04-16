// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <BRTransaction.h>
#include "catch.hpp"
#include "Transaction.h"
#include "Address.h"
#include "Key.h"

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
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		ByteData script = myaddress.getPubKeyScript();
		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");

		TransactionInput transactionInput(hash, 0, 1000, script, ByteData(nullptr, 0), 1);
		Transaction transaction;
		transaction.addInput(transactionInput);

		ByteData byteData = transaction.serialize();
		REQUIRE(byteData.length > 0);
		REQUIRE(byteData.data != nullptr);

		Transaction transaction1(byteData);
		REQUIRE(transaction1.getRaw() != nullptr);
		ByteData byteData1 = transaction1.serialize();
		REQUIRE(byteData1.length == byteData.length);
		for (int i = 0; i < byteData.length; i++) {
			REQUIRE(byteData.data[i] == byteData1.data[i]);
		}
	}

	SECTION("Constructor init with buffer and 'blockHeight + timeStamp'") {
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		ByteData script = myaddress.getPubKeyScript();
		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		TransactionInput transactionInput(hash, 0, 1000, script, ByteData(nullptr, 0), 1);
		Transaction transaction;
		transaction.addInput(transactionInput);

		ByteData byteData = transaction.serialize();
		uint32_t blockHeight = 888;
		uint32_t timeStamp = 1523608889;

		Transaction transaction1(byteData, blockHeight, timeStamp);
		REQUIRE(transaction1.getRaw() != nullptr);
		REQUIRE(transaction1.getBlockHeight() == blockHeight);
		REQUIRE(transaction1.getTimestamp() == timeStamp);

		ByteData byteData1 = transaction1.serialize();
		REQUIRE(byteData1.length == byteData.length);
		for (int i = 0; i < byteData.length; i++) {
			REQUIRE(byteData.data[i] == byteData1.data[i]);
		}
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

TEST_CASE("transaction with inpus and outputs", "[Transaction]") {

	SECTION("transaction with inputs") {
		Transaction transaction;
		uint32_t index = 8000;
		uint64_t amount = 10000;
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		uint32_t sequence = 8888;
		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		ByteData script = myaddress.getPubKeyScript();
		std::vector<TransactionInput> inputList;
		for (int i = 0; i < 10; i++) {
			TransactionInput input(hash, i + 1, amount + i, myaddress.getPubKeyScript(), ByteData(), sequence + i);
			transaction.addInput(input);
			inputList.push_back(input);
		}

		SharedWrapperList<TransactionInput, BRTxInput *> inputs = transaction.getInputs();
		REQUIRE(inputs.size() == inputList.size());
		for (int i = 0; i < inputList.size(); i++) {
			boost::shared_ptr<TransactionInput> input = inputs[i];
			REQUIRE(input->getIndex() == inputList[i].getIndex());
			REQUIRE(input->getAmount() == inputList[i].getAmount());
			UInt256 temp = input->getHash();
			int result = UInt256Eq(&hash, &temp);
			REQUIRE(result == 1);
			REQUIRE(input->getSequence() == inputList[i].getSequence());
			ByteData tempScript1 = input->getScript();
			ByteData tempScript2 = inputList[i].getScript();
			REQUIRE(tempScript1.length == tempScript2.length);
			for (int j = 0; j < tempScript1.length; j++) {
				REQUIRE(tempScript1.data[j] == tempScript2.data[j]);
			}
		}

		std::vector<std::string> addressList = transaction.getInputAddresses();
		for (int i = 0; i < addressList.size(); i++) {
			REQUIRE(addressList[i] == content);
		}
	}

	SECTION("transaction with outputs") {
		Transaction transaction;
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		std::vector<TransactionOutput> outputsList;
		for (int i = 0; i < 10; i++) {
			TransactionOutput output(8888 + i, myaddress.getPubKeyScript());
			transaction.addOutput(output);
			outputsList.push_back(output);
		}

		SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction.getOutputs();
		REQUIRE(outputs.size() == outputsList.size());
		for (int i = 0; i < outputsList.size(); i++) {
			boost::shared_ptr<TransactionOutput> output = outputs[i];
			ByteData tempScript1 = output->getScript();
			ByteData tempScript2 = outputsList[i].getScript();
			REQUIRE(tempScript1.length == tempScript2.length);
			for (int j = 0; j < tempScript1.length; j++) {
				REQUIRE(tempScript1.data[j] == tempScript2.data[j]);
			}
			REQUIRE(output->getAmount() == outputsList[i].getAmount());
		}

		std::vector<std::string> addressList = transaction.getOutputAddresses();
		for (int i = 0; i < addressList.size(); i++) {
			REQUIRE(addressList[i] == content);
		}
	}
}

TEST_CASE("transaction public method test", "[Transaction]") {

	Transaction transaction;
	SECTION("transaction getHash test") {
		//contructor transaction is no hash, if send a transaction then must sign ,then has a hash;
		UInt256 hash = transaction.getHash();
		UInt256 zero = UINT256_ZERO;
		int result = UInt256Eq(&hash, &zero);
		REQUIRE(result == 1);

		Key key("0000000000000000000000000000000000000000000000000000000000000001");
		transaction.sign(key, 0);
		UInt256 tempHash = transaction.getHash();
		result = UInt256Eq(&tempHash, &zero);
		REQUIRE(result == 0);
	}

	SECTION("transaction getVersion test") {
		REQUIRE(transaction.getVersion() == 0x00000001);
	}

	SECTION("transaction getLockTime test") {
		REQUIRE(transaction.getLockTime() == 0);

		transaction.setLockTime(0x00002345);
		REQUIRE(transaction.getLockTime() == 0x00002345);
	}

	SECTION("transaction setTimestamp test") {
		REQUIRE(transaction.getTimestamp() == 0);

		transaction.setTimestamp(1523863152);
		REQUIRE(transaction.getTimestamp() == 1523863152);
	}

	SECTION("transaction shuffleOutputs test") {
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		std::vector<TransactionOutput> outputsList;
		for (int i = 0; i < 10; i++) {
			TransactionOutput output(8888 + i, myaddress.getPubKeyScript());
			transaction.addOutput(output);
			outputsList.push_back(output);
		}

		transaction.shuffleOutputs();

		SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction.getOutputs();
		int noSameCount = 0;
		for (int i = 0; i < outputsList.size(); i++) {
			if (outputs[i]->getAmount() != outputsList[i].getAmount()) {
				noSameCount++;
			}
		}
		REQUIRE(noSameCount > 0);
	}

	SECTION("transaction getSize test") {
		size_t size = transaction.getSize();
		REQUIRE(size == 10);

		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		uint32_t sequence = 8888;
		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		std::vector<TransactionInput> inputList;
		for (int i = 0; i < 10; i++) {
			TransactionInput input(hash, i + 1, 1000 + i, myaddress.getPubKeyScript(), ByteData(), i + 1);
			transaction.addInput(input);
		}
		size_t inputSize = transaction.getSize();
		REQUIRE(inputSize > size);

		for (int i = 0; i < 10; i++) {
			TransactionOutput output(8888 + i, myaddress.getPubKeyScript());
			transaction.addOutput(output);
		}
		size = transaction.getSize();
		REQUIRE(size > inputSize);
	}

	SECTION("transaction getStandardFee test") {
		uint64_t fee = transaction.getStandardFee();
		REQUIRE(fee > 0);
	}

	SECTION("transaction isSigned test") {
		bool res = transaction.isSigned();
		REQUIRE(res == true);

		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		Key key("0000000000000000000000000000000000000000000000000000000000000001");
		transaction.addInput(TransactionInput(hash, 1, 1000, myaddress.getPubKeyScript(), key.sign(hash), 1));
		res = transaction.isSigned();
		REQUIRE(res == true);

		transaction.addInput(TransactionInput(hash, 1, 1000, myaddress.getPubKeyScript(), ByteData(), 1));
		res = transaction.isSigned();
		REQUIRE(res == false);
	}

	SECTION("transaction mulity sign and getReverseHash test") {
		WrapperList<Key, BRKey> keys(3);
		keys[0] = Key("0000000000000000000000000000000000000000000000000000000000000001");
		keys[1] = Key("0000000000000000000000000000000000000000000000000000000000000010");
		keys[2] = Key("0000000000000000000000000000000000000000000000000000000000000011");
		transaction.sign(keys, 0);

		UInt256 zero = UINT256_ZERO;
		UInt256 tempHash = transaction.getHash();
		int result = UInt256Eq(&tempHash, &zero);
		REQUIRE(result == 0);

		UInt256 reverseHash = transaction.getReverseHash();
		ssize_t size = sizeof(reverseHash.u8);
		REQUIRE(size == 32);
		for (int i = 0; i < size; i++) {
			REQUIRE(reverseHash.u8[i] == tempHash.u8[size - 1 - i]);
		}
	}

	SECTION("transaction getMinOutputAmount test") {
		uint64_t value = transaction.getMinOutputAmount();
		REQUIRE(value == TX_MIN_OUTPUT_AMOUNT);
	}

	SECTION("transaction isStandard test") {
		//todo result is always true,it's didn't has transaction type judge
		bool result = transaction.isStandard();
		REQUIRE(result == true);
	}
}