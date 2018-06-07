// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "ELABRTxOutput.h"
#include "ELABRTransaction.h"
#include "catch.hpp"
#include "Transaction.h"
#include "Address.h"
#include "Payload/PayloadCoinBase.h"
#include "Utils.h"

using namespace Elastos::SDK;

TEST_CASE("Transaction constructor test", "[Transaction]") {

	SECTION("Default constructor") {

		Transaction transaction;
		REQUIRE(transaction.getRaw() != nullptr);
	}

	SECTION("Constructor init with raw pointer") {
		ELABRTransaction *raw = ELABRTransactionNew();
		{
			Transaction transaction((BRTransaction *) raw);
			REQUIRE(transaction.getRaw() != nullptr);
			REQUIRE(transaction.getRaw() == (BRTransaction *) raw);
		}
		//raw shall be invalid pointer here
	}
}

TEST_CASE("New empty transaction behavior", "[Transaction]") {

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
		CMBlock script = myaddress.getPubKeyScript();
		std::vector<TransactionInput> inputList;
		for (int i = 0; i < 10; i++) {
			TransactionInput input(hash, i + 1, amount + i, myaddress.getPubKeyScript(), CMBlock(), sequence + i);
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
			CMBlock tempScript1 = input->getScript();
			CMBlock tempScript2 = inputList[i].getScript();
			REQUIRE(tempScript1.GetSize() == tempScript2.GetSize());
			for (uint64_t j = 0; j < tempScript1.GetSize(); j++) {
				REQUIRE(tempScript1[j] == tempScript2[j]);
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
			CMBlock tempScript1 = output->getScript();
			CMBlock tempScript2 = outputsList[i].getScript();
			REQUIRE(tempScript1.GetSize() == tempScript2.GetSize());
			for (uint64_t j = 0; j < tempScript1.GetSize(); j++) {
				REQUIRE(tempScript1[j] == tempScript2[j]);
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

	SECTION("transaction getSize test") {
		size_t size = transaction.getSize();
		REQUIRE(size == 10);

		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		uint32_t sequence = 8888;
		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		std::vector<TransactionInput> inputList;
		for (int i = 0; i < 10; i++) {
			TransactionInput input(hash, i + 1, 1000 + i, myaddress.getPubKeyScript(), CMBlock(), i + 1);
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
		REQUIRE(res == false);

		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		Key key("0000000000000000000000000000000000000000000000000000000000000001");
		transaction.addInput(TransactionInput(hash, 1, 1000, myaddress.getPubKeyScript(), key.sign(hash), 1));
		res = transaction.isSigned();
		REQUIRE(res == true);
	}

	SECTION("transaction mulity sign and getReverseHash test") {
		WrapperList<Key, BRKey> keys(3);
		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		keys[0] = Key("0000000000000000000000000000000000000000000000000000000000000001");
		keys[1] = Key("0000000000000000000000000000000000000000000000000000000000000010");
		keys[2] = Key("0000000000000000000000000000000000000000000000000000000000000011");
		WrapperList<Address, BRAddress> address(3);
		for (int i = 0; i < 3; ++i) {
			std::string addr = keys[0].address();
			address[i] = Address(addr);
			TransactionInput input = TransactionInput(hash, 1, 1000, address[i].getPubKeyScript(), CMBlock(), 1);
			transaction.addInput(input);
		}
		bool r = transaction.sign(keys, 0);
		REQUIRE(r == true);

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

TEST_CASE("Transaction Serialize test", "[Transaction]") {

	SECTION("transaction Serialize test") {
		Transaction transaction;
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		for (int i = 0; i < 10; i++) {
			TransactionInput input(hash, i + 1, 10000 + i, myaddress.getPubKeyScript(), CMBlock(), i);
			transaction.addInput(input);
		}

		UInt256 outHash = uint256("0000000000000000008e5d72027ef42ca050a0776b7184c96d0d4b300fa5da9e");

		UInt168 programHash = *(UInt168 *) "\x21\xc2\xe2\x51\x72\xcb\x15\x19\x3c\xb1\xc6\xd4\x8f\x60\x7d\x42\xc1\xd2\xa2\x15\x16";

		for (int i = 0; i < 10; i++) {
			TransactionOutput output;
			output.setAssetId(outHash);
			output.setAmount(888);
			output.setOutputLock(i + 1);
			output.setProgramHash(programHash);
			transaction.addOutput(output);
		}
		transaction.setLockTime(1524231034);
		transaction.setTransactionType(Transaction::Type::CoinBase);

		ByteStream byteStream;
		transaction.Serialize(byteStream);
		byteStream.setPosition(0);

		Transaction transaction1;
		transaction1.Deserialize(byteStream);

		REQUIRE(transaction.getLockTime() == transaction1.getLockTime());

		SharedWrapperList<TransactionInput, BRTxInput *> inputs1 = transaction.getInputs();
		SharedWrapperList<TransactionInput, BRTxInput *> inputs2 = transaction1.getInputs();
		REQUIRE(inputs1.size() == inputs2.size());
		for (int i = 0; i < inputs1.size(); i++) {
			boost::shared_ptr<TransactionInput> input = inputs1[i];
			REQUIRE(input->getIndex() == inputs2[i]->getIndex());
			REQUIRE(input->getSequence() == inputs2[i]->getSequence());
			UInt256 temp = input->getHash();
			int result = UInt256Eq(&hash, &temp);
			REQUIRE(result == 1);
		}

		SharedWrapperList<TransactionOutput, BRTxOutput *> outputs1 = transaction.getOutputs();
		SharedWrapperList<TransactionOutput, BRTxOutput *> outputs2 = transaction1.getOutputs();
		REQUIRE(outputs1.size() == outputs2.size());
		for (int i = 0; i < outputs1.size(); i++) {
			boost::shared_ptr<TransactionOutput> outPut = outputs1[i];
			REQUIRE(outPut->getAmount() == outputs2[i]->getAmount());
			REQUIRE(outPut->getOutputLock() == outputs2[i]->getOutputLock());

			int result = UInt256Eq(&outPut->getAssetId(), &outputs2[i]->getAssetId());
			REQUIRE(result == 1);

			result = UInt168Eq(&outPut->getProgramHash(), &outputs2[i]->getProgramHash());
			REQUIRE(result == 1);
		}
	}

}

TEST_CASE("Transaction conver method test", "[Transaction]") {
	SECTION("transaction  BRTransaction convertToRaw test") {
		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		ELABRTransaction *raw = ELABRTransactionNew();
		raw->raw.txHash = hash;
		raw->raw.version = 1;
		raw->raw.blockHeight = 2;
		raw->raw.inCount = 5;
		raw->raw.outCount = 5;
		raw->raw.lockTime = 1524231034;
		raw->raw.timestamp = 1924231000;
		uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24, 60, 75, 8, 182, 57, 98};
		for (int i = 0; i < 5; ++i) {
			BRTransactionAddInput(&raw->raw, hash, i, i, s, 21, nullptr, 0, i);
		}
		UInt168 programHash = *(UInt168 *) "\x21\xc2\xe2\x51\x72\xcb\x15\x19\x3c\xb1\xc6\xd4\x8f\x60\x7d\x42\xc1\xd2"
											"\xa2\x15\x16";
		for (int i = 0; i < 5; ++i) {
			ELABRTransactionAddOutput(raw, i, s, 21);
			raw->outputLockList.push_back(i);
			raw->outputAssetIDList.push_back(hash);
			raw->outputProgramHashList.push_back(programHash);
		}

		Transaction transaction((BRTransaction *) raw);
		transaction.setTransactionType(Transaction::Type::CoinBase);

		UInt256 hash1 = transaction.getHash();
		int result = UInt256Eq(&hash1, &hash);
		REQUIRE(result == 1);
		REQUIRE(transaction.getVersion() == raw->raw.version);
		REQUIRE(transaction.getBlockHeight() == raw->raw.blockHeight);
		REQUIRE(transaction.getLockTime() == raw->raw.lockTime);
		REQUIRE(transaction.getTimestamp() == raw->raw.timestamp);

		SharedWrapperList<TransactionInput, BRTxInput *> inputs = transaction.getInputs();
		REQUIRE(inputs.size() == raw->raw.inCount);
		for (int i = 0; i < raw->raw.inCount; i++) {
			UInt256 hash2 = inputs[i]->getHash();
			result = UInt256Eq(&hash2, &hash);
			REQUIRE(result == 1);
			REQUIRE(inputs[i]->getAmount() == i);
			REQUIRE(inputs[i]->getIndex() == i);
			REQUIRE(inputs[i]->getSequence() == i);
			CMBlock tempScript1 = inputs[i]->getScript();
			REQUIRE(tempScript1.GetSize() == 21);
			result = memcmp(tempScript1, s, 21);
			REQUIRE(result == 0);
		}

		SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction.getOutputs();
		REQUIRE(outputs.size() == raw->raw.outCount);
		for (int i = 0; i < raw->raw.outCount; i++) {
			REQUIRE(outputs[i]->getAmount() == i);
			CMBlock byteData = outputs[i]->getScript();
			REQUIRE(byteData.GetSize() == 21);
			result = memcmp(byteData, s, 21);
			REQUIRE(result == 0);
		}

		BRTransaction *raw1 = transaction.convertToRaw();
		REQUIRE(raw1->inCount == raw->raw.inCount);
		REQUIRE(raw1->outCount == raw->raw.outCount);
		REQUIRE(raw1->blockHeight == raw->raw.blockHeight);
		REQUIRE(raw1->lockTime == raw->raw.lockTime);
		REQUIRE(raw1->timestamp == raw->raw.timestamp);
		REQUIRE(raw1->version == raw->raw.version);
		result = UInt256Eq(&raw1->txHash, &raw->raw.txHash);
		REQUIRE(result == 1);
		for (size_t i = 0; i < raw->raw.inCount; i++) {
			result = UInt256Eq(&raw1->inputs[i].txHash, &raw->raw.inputs[i].txHash);
			REQUIRE(result == 1);
			REQUIRE(raw1->inputs[i].amount == raw->raw.inputs[i].amount);
			REQUIRE(raw1->inputs[i].index == raw->raw.inputs[i].index);
			REQUIRE(raw1->inputs[i].sequence == raw->raw.inputs[i].sequence);
			REQUIRE(raw1->inputs[i].scriptLen == raw->raw.inputs[i].scriptLen);
			result = memcmp(raw1->inputs[i].script, raw->raw.inputs[i].script, raw->raw.inputs[i].scriptLen);
			REQUIRE(result == 0);
		}
		ELABRTransaction *elaRaw1 = (ELABRTransaction *)raw1;
		for (size_t i = 0; i < raw->raw.outCount; i++) {
			REQUIRE(elaRaw1->outputs[i].raw.amount == raw->outputs[i].raw.amount);
			REQUIRE(elaRaw1->outputs[i].raw.scriptLen == raw->outputs[i].raw.scriptLen);
			result = memcmp(elaRaw1->outputs[i].raw.script, raw->outputs[i].raw.script, raw->outputs[i].raw.scriptLen);
			REQUIRE(result == 0);
		}

	}

	SECTION("transaction ELABRTransaction convertToRaw test") {
		ELABRTransaction *raw = ELABRTransactionNew();
		uint8_t t[21] = {18, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24, 60, 75, 6, 182, 28, 34};
		//uint8_t *s = new uint8_t[21];
		CMBlock coinBytes(21);
		memcpy(coinBytes, t, 21);
		PayloadCoinBase payload(coinBytes);
		ByteStream stream;
		payload.Serialize(stream);

		CMBlock mb(stream.length());
		uint8_t *tmp = stream.getBuf();
		memcpy(mb, tmp, mb.GetSize());
		delete[]tmp;

		raw->payloadData = mb;

		raw->attributeData.clear();
		for (int i = 0; i < 10; i++) {
			CMBlock mb(21);
			memcpy(mb, coinBytes, 21);
			Attribute attrib(Attribute::Usage::Script, mb);
			ByteStream byteStream;
			attrib.Serialize(byteStream);
			byteStream.setPosition(0);
			CMBlock mb1(byteStream.length());
			uint8_t *tmp = byteStream.getBuf();
			memcpy(mb1, tmp, byteStream.length());
			delete[]tmp;
			raw->attributeData.push_back(mb1);
		}
		uint8_t s1[20] = {18, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24, 60, 75, 6, 182,
		                  28};
		for (int i = 0; i < 10; i++) {
			CMBlock script(21);
			memcpy(script, coinBytes, 21);
			CMBlock script1(20);
			memcpy(script1, s1, 20);
			Program program(script, script1);
			ByteStream byteStream;
			program.Serialize(byteStream);
			byteStream.setPosition(0);
			CMBlock mb(byteStream.length());
			uint8_t *tmp = byteStream.getBuf();
			memcpy(mb, tmp, byteStream.length());
			delete[]tmp;
			raw->programData.push_back(mb);
		}

		Transaction transaction((BRTransaction *) raw);
		ELABRTransaction *raw1 = (ELABRTransaction *) transaction.convertToRaw();

		REQUIRE(raw1->raw.inCount == raw->raw.inCount);
		REQUIRE(raw1->raw.outCount == raw->raw.outCount);
		REQUIRE(raw1->raw.blockHeight == raw->raw.blockHeight);
		REQUIRE(raw1->raw.lockTime == raw->raw.lockTime);
		REQUIRE(raw1->raw.timestamp == raw->raw.timestamp);
		REQUIRE(raw1->raw.version == raw->raw.version);
		int result = UInt256Eq(&raw1->raw.txHash, &raw->raw.txHash);
		REQUIRE(result == 1);
		for (size_t i = 0; i < raw->raw.inCount; i++) {
			result = UInt256Eq(&raw1->raw.inputs[i].txHash, &raw->raw.inputs[i].txHash);
			REQUIRE(result == 1);
			REQUIRE(raw1->raw.inputs[i].amount == raw->raw.inputs[i].amount);
			REQUIRE(raw1->raw.inputs[i].index == raw->raw.inputs[i].index);
			REQUIRE(raw1->raw.inputs[i].sequence == raw->raw.inputs[i].sequence);
			REQUIRE(raw1->raw.inputs[i].scriptLen == raw->raw.inputs[i].scriptLen);
			result = memcmp(raw1->raw.inputs[i].script, raw->raw.inputs[i].script, raw->raw.inputs[i].scriptLen);
			REQUIRE(result == 0);
		}
		for (size_t i = 0; i < raw->raw.outCount; i++) {
			REQUIRE(raw1->raw.outputs[i].amount == raw->raw.outputs[i].amount);
			REQUIRE(raw1->raw.outputs[i].scriptLen == raw->raw.outputs[i].scriptLen);
			result = memcmp(raw1->raw.outputs[i].script, raw->raw.outputs[i].script, raw->raw.outputs[i].scriptLen);
			REQUIRE(result == 0);
		}

		REQUIRE(raw1->payloadData.GetSize() == raw->payloadData.GetSize());
		result = memcmp(raw1->payloadData, raw->payloadData, raw->payloadData);
		REQUIRE(result == 0);

		REQUIRE(raw1->attributeData.size() == raw->attributeData.size());
		for (size_t i = 0; i < raw->attributeData.size(); i++) {
			REQUIRE(raw1->attributeData[i].GetSize() == raw->attributeData[i].GetSize());
			result = memcmp(raw1->attributeData[i], raw->attributeData[i], raw->attributeData[i].GetSize());
			REQUIRE(result == 0);
		}

		REQUIRE(raw1->programData.size() == raw->programData.size());
		for (size_t i = 0; i < raw->programData.size(); i++) {
			REQUIRE(raw1->programData[i].GetSize() == raw->programData[i].GetSize());
			result = memcmp(raw1->programData[i], raw->programData[i], raw->programData[i].GetSize());
			REQUIRE(result == 0);
		}

		ELABRTransactionFree((ELABRTransaction *) raw1);
	}

}