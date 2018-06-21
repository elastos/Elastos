// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <Core/BRTransaction.h>
#include "ELATxOutput.h"
#include "SDK/Transaction/TransactionOutput.h"
#include "ELATransaction.h"
#include "catch.hpp"
#include "SDK/Transaction/Transaction.h"
#include "BRTransaction.h"
#include "Address.h"
#include "Payload/PayloadCoinBase.h"
#include "Utils.h"
#include "Log.h"

using namespace Elastos::ElaWallet;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

UInt256 getRandUInt256(void) {
	UInt256 u;
	for (size_t i = 0; i < ARRAY_SIZE(u.u32); ++i) {
		u.u32[i] = rand();
	}
	return u;
}

UInt168 getRandUInt168(void) {
	UInt168 u;
	for (size_t i = 0; i < ARRAY_SIZE(u.u8); ++i) {
		u.u8[i] = rand();
	}
	return u;
}

CMBlock getRandCMBlock(size_t size) {
	CMBlock block(size);

	for (size_t i = 0; i < size; ++i) {
		block[i] = (uint8_t)rand();
	}

	return block;
}

TEST_CASE("Transaction constructor test", "[Transaction]") {

	SECTION("Default constructor") {

		Transaction transaction;
		REQUIRE(transaction.getRaw() != nullptr);
	}
}

TEST_CASE("New empty transaction behavior", "[Transaction]") {

	Transaction transaction;

	SECTION("Input and related addresses") {
//		REQUIRE(transaction.getInputs().empty());
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

		for (int i = 0; i < 10; i++) {
			transaction.addInput(hash, i + 1, amount + i, myaddress.getPubKeyScript(), CMBlock(), sequence + i);
		}
		REQUIRE(transaction.getRaw()->inCount == 10);
		for (int i = 0; i < 10; i++) {
			BRTxInput *input = &transaction.getRaw()->inputs[i];

			REQUIRE(UInt256Eq(&hash, &input->txHash) == 1);
			REQUIRE(input->index == i + 1);
			REQUIRE(input->amount == amount + i);
			REQUIRE(input->sequence == sequence + i);
			REQUIRE(input->scriptLen == script.GetSize());
			REQUIRE(0 == memcmp(script, input->script, input->scriptLen));
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
		CMBlock script = myaddress.getPubKeyScript();
		for (int i = 0; i < 10; i++) {
			TransactionOutput *output = new TransactionOutput(8888 + i, script);
			transaction.addOutput(output);
		}

		SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction.getOutputs();
		REQUIRE(outputs.size() == 10);
		for (int i = 0; i < 10; i++) {
			TransactionOutputPtr output = outputs[i];
			CMBlock s = output->getScript();
			REQUIRE(s.GetSize() == script.GetSize());
			for (uint64_t j = 0; j < s.GetSize(); j++) {
				REQUIRE(s[j] == script[j]);
			}
			REQUIRE(output->getAmount() == 8888 + i);
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
		REQUIRE(UInt256Eq(&hash, &zero) != 1);
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
		for (int i = 0; i < 10; i++) {
			transaction.addInput(hash, i + 1, 1000 + i, myaddress.getPubKeyScript(), CMBlock(), i + 1);
		}
		size_t inputSize = transaction.getSize();
		REQUIRE(inputSize > size);

		for (int i = 0; i < 10; i++) {
			TransactionOutput *output = new TransactionOutput(8888 + i, myaddress.getPubKeyScript());
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
		CMBlock mb;
		mb.SetMemFixed(hash.u8, sizeof(hash));
		transaction.addInput(hash, 1, 1000, myaddress.getPubKeyScript(), key.compactSign(mb), 1);
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
			transaction.addInput(hash, 1, 1000, address[i].getPubKeyScript(), CMBlock(), 1);
		}
		bool r = transaction.sign(keys, 0);
		REQUIRE(r == true);

		UInt256 zero = UINT256_ZERO;
		UInt256 tempHash = transaction.getHash();
		REQUIRE(UInt256Eq(&tempHash, &zero) != 1);

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
			transaction.addInput(hash, i + 1, 10000 + i, myaddress.getPubKeyScript(), CMBlock(), i);
		}

		UInt256 outHash = uint256("0000000000000000008e5d72027ef42ca050a0776b7184c96d0d4b300fa5da9e");

		UInt168 programHash = *(UInt168 *) "\x21\xc2\xe2\x51\x72\xcb\x15\x19\x3c\xb1\xc6\xd4\x8f\x60\x7d\x42\xc1\xd2\xa2\x15\x16";

		for (int i = 0; i < 10; i++) {
			TransactionOutput *output = new TransactionOutput();
			output->setAssetId(outHash);
			output->setAmount(888);
			output->setOutputLock(i + 1);
			output->setProgramHash(programHash);
			transaction.addOutput(output);
		}
		transaction.setLockTime(1524231034);
		transaction.setTransactionType(ELATransaction::Type::CoinBase);

		ByteStream byteStream;
		transaction.Serialize(byteStream);
		byteStream.setPosition(0);

		Transaction transaction1;
		transaction1.Deserialize(byteStream);

		REQUIRE(transaction.getLockTime() == transaction1.getLockTime());

		REQUIRE(10 == transaction.getRaw()->inCount);
		for (int i = 0; i < transaction.getRaw()->inCount; i++) {
			BRTxInput *input = &transaction.getRaw()->inputs[i];
			REQUIRE(input->index == i + 1);
			REQUIRE(input->sequence == i);
			UInt256 hash = input->txHash;
			REQUIRE(UInt256Eq(&hash, &hash) == 1);
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

TEST_CASE("Convert to and from json", "[Transaction]") {
	srand(time(nullptr));

	SECTION("to and from json") {
		Transaction tx;
		tx.isRegistered() = true;
		ELATransaction *ela = (ELATransaction *)tx.getRaw();

		ela->raw.txHash = getRandUInt256();
		ela->raw.version = rand();
		ela->raw.inCount = 3;
		for (size_t i = 0; i < 3; ++i) {
			CMBlock script = getRandCMBlock(25);
			CMBlock signature = getRandCMBlock(28);
			BRTransactionAddInput(&ela->raw, getRandUInt256(), i, rand(),
					script, script.GetSize(), signature, signature.GetSize(), rand());
		}

		ela->raw.lockTime = rand();
		ela->raw.blockHeight = rand();
		ela->raw.timestamp = rand();
		ela->type = ELATransaction::Type(rand() % ELATransaction::Type::TypeMaxCount);
		ela->payloadVersion = rand() % sizeof(ela->payloadVersion);
		ela->fee = rand();
		ela->payload = ELAPayloadNew(ela->type);

		for (size_t i = 0; i < 4; ++i) {
			TransactionOutputPtr output(new TransactionOutput());
			ELATxOutput *o = (ELATxOutput *)output->getRaw();
			CMBlock script = getRandCMBlock(25);
			BRTxOutputSetScript((BRTxOutput *)o, script, script.GetSize());
			o->raw.amount = rand();
			o->assetId = getRandUInt256();
			o->outputLock = rand();
			o->programHash = getRandUInt168();
			ela->outputs.push_back(output);
		}

		nlohmann::json txJson = tx.toJson();

		/* verify transaction */
		Transaction txn;
		txn.fromJson(txJson);

		// TODO [heropan] complete me later
	}
}

