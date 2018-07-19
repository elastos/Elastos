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
#include "TestHelper.h"

using namespace Elastos::ElaWallet;

static ELATransaction *createELATransaction() {
	ELATransaction *tx = ELATransactionNew();

	tx->raw.outCount = 0;
	tx->raw.outputs = nullptr;
	tx->raw.version = rand();
	for (size_t i = 0; i < 20; ++i) {
		UInt256 txHash = getRandUInt256();
		uint32_t index = (uint16_t)rand();
		uint64_t amount = rand();
		CMBlock script = getRandCMBlock(25);
		CMBlock signature = getRandCMBlock(50);
		uint32_t squence = rand();
		BRTransactionAddInput(&tx->raw, txHash, index, amount,
							  script, script.GetSize(), signature, signature.GetSize(),
							  squence);
	}

	for (size_t i = 0; i < 20; ++i) {
		ELATxOutput *o = ELATxOutputNew();
		o->assetId = getRandUInt256();
		o->programHash = getRandUInt168();
		o->outputLock = rand();
		o->signType = rand();
		uint64_t amount = rand();
		CMBlock script = getRandCMBlock(25);
		ELATxOutputSetScript(o, script, script.GetSize());
		TransactionOutput *output = new TransactionOutput(o);
		tx->outputs.push_back(output);
	}
	tx->raw.lockTime = rand();
	tx->raw.blockHeight = rand();
	tx->raw.timestamp = rand();

	tx->type = ELATransaction::TransferAsset;
	delete tx->payload;
	tx->payload = ELAPayloadNew(tx->type);

	tx->payloadVersion = rand();
	tx->fee = rand();

	for (size_t i = 0; i < 20; ++i) {
		CMBlock data = getRandCMBlock(25);
		Attribute *attr = new Attribute(Attribute::Script, data);
		tx->attributes.push_back(attr);
	}

	for (size_t i = 0; i < 20; ++i) {
		CMBlock code = getRandCMBlock(25);
		CMBlock parameter = getRandCMBlock(25);
		Program *program = new Program(code, parameter);
		tx->programs.push_back(program);
	}

	tx->Remark = getRandString(40);

	return tx;
}

static void verifyELATransaction(const ELATransaction *tx1, const ELATransaction *tx, bool checkAll = true) {
	REQUIRE(tx1->raw.outCount == tx->raw.outCount);
	REQUIRE(tx1->raw.outputs == nullptr);
	REQUIRE(UInt256Eq(&tx1->raw.txHash, &tx->raw.txHash));
	if (checkAll)
		REQUIRE(tx1->raw.version == tx->raw.version);
	REQUIRE(tx1->raw.inCount == tx->raw.inCount);
	for (size_t i = 0; i < tx1->raw.inCount; ++i) {
		REQUIRE(UInt256Eq(&tx1->raw.inputs[i].txHash, &tx->raw.inputs[i].txHash));
		if (checkAll) {
			REQUIRE(!strcmp(tx1->raw.inputs[i].address, tx->raw.inputs[i].address));
			REQUIRE(tx1->raw.inputs[i].amount == tx->raw.inputs[i].amount);
		}
		REQUIRE(tx1->raw.inputs[i].index == tx->raw.inputs[i].index);
		REQUIRE(tx1->raw.inputs[i].sequence == tx->raw.inputs[i].sequence);
		if (checkAll) {
			REQUIRE(tx1->raw.inputs[i].scriptLen == tx->raw.inputs[i].scriptLen);
			REQUIRE(!memcmp(tx1->raw.inputs[i].script, tx->raw.inputs[i].script, tx->raw.inputs[i].scriptLen));
			REQUIRE(tx1->raw.inputs[i].sigLen == tx->raw.inputs[i].sigLen);
			REQUIRE(!memcmp(tx1->raw.inputs[i].signature, tx->raw.inputs[i].signature, tx->raw.inputs[i].sigLen));
		}
	}

	REQUIRE(tx1->outputs.size() == tx->outputs.size());
	for (size_t i = 0; i < tx->outputs.size(); ++i) {
		ELATxOutput *o, *o1;
		o = (ELATxOutput *)tx->outputs[i]->getRaw();
		o1 = (ELATxOutput *)tx1->outputs[i]->getRaw();
		REQUIRE(UInt256Eq(&o1->assetId, &o->assetId));
		REQUIRE(o1->signType == o->signType);
		REQUIRE(UInt168Eq(&o1->programHash, &o->programHash));
		REQUIRE(o1->outputLock == o->outputLock);
		if (checkAll) {
			REQUIRE(o1->raw.scriptLen == o->raw.scriptLen);
			REQUIRE(!memcmp(o1->raw.script, o->raw.script, o->raw.scriptLen));
		}
		REQUIRE(o1->raw.amount == o->raw.amount);
		if (checkAll) {
			REQUIRE(!strcmp(o1->raw.address, o->raw.address));
		}
	}

	REQUIRE(tx1->raw.lockTime == tx->raw.lockTime);
	if (checkAll) {
		REQUIRE(tx1->raw.blockHeight == tx->raw.blockHeight);
		REQUIRE(tx1->raw.timestamp == tx->raw.timestamp);
	}
	REQUIRE(tx1->type == tx->type);
	REQUIRE(tx1->payloadVersion == tx->payloadVersion);
	if (checkAll) {
		REQUIRE(tx1->fee == tx->fee);
		REQUIRE(tx1->Remark == tx->Remark);
	}
	REQUIRE(tx1->attributes.size() == tx->attributes.size());
	for (size_t i = 0; i < tx1->attributes.size(); ++i) {
		REQUIRE(tx1->attributes[i]->GetUsage() == tx->attributes[i]->GetUsage());
		const CMBlock &data = tx->attributes[i]->GetData();
		const CMBlock &data1 = tx1->attributes[i]->GetData();
		REQUIRE(data1.GetSize() == data.GetSize());
		REQUIRE(!memcmp(data1, data, data.GetSize()));
	}

	REQUIRE(tx1->programs.size() == tx->programs.size());
	for (size_t i = 0; i < tx->programs.size(); ++i) {
		const CMBlock &code = tx->programs[i]->getCode();
		const CMBlock &code1 = tx1->programs[i]->getCode();
		REQUIRE(code1.GetSize() == code.GetSize());
		REQUIRE(!memcmp(code1, code, code.GetSize()));
		const CMBlock &parameter = tx->programs[i]->getParameter();
		const CMBlock &parameter1 = tx1->programs[i]->getParameter();
		REQUIRE(parameter1.GetSize() == parameter.GetSize());
		REQUIRE(!memcmp(parameter1, parameter, parameter.GetSize()));
	}
}

TEST_CASE("Transaction Serialize and Deserialize", "[Transaction]") {
	srand(time(nullptr));

	SECTION("transaction Serialize test") {
		ELATransaction *tx = createELATransaction();

		Transaction txn(tx);
		tx->raw.txHash = txn.getHash();

		ByteStream stream;
		txn.Serialize(stream);

		Transaction txn1;
		stream.setPosition(0);
		REQUIRE(txn1.Deserialize(stream));

		ELATransaction *tx1 = (ELATransaction *)txn1.getRaw();

		verifyELATransaction(tx1, tx, false);

		Transaction txn2 = txn;
		ELATransaction *tx2 = (ELATransaction *)txn2.getRaw();

		verifyELATransaction(tx2, tx, true);
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
		for (size_t i = 0; i < 3; ++i) {
			CMBlock script = getRandCMBlock(25);
			CMBlock signature = getRandCMBlock(28);
			BRTransactionAddInput(&ela->raw, getRandUInt256(), i, rand(),
					script, script.GetSize(), signature, signature.GetSize(), rand());
		}

		ela->raw.lockTime = rand();
		ela->raw.blockHeight = rand();
		ela->raw.timestamp = rand();
		ela->type = ELATransaction::Type::TransferAsset;// ELATransaction::Type(rand() % ELATransaction::Type::TypeMaxCount);
		ela->payloadVersion = rand() % sizeof(ela->payloadVersion);
		ela->fee = rand();
		delete ela->payload;
		ela->payload = ELAPayloadNew(ela->type);

		for (size_t i = 0; i < 4; ++i) {
			TransactionOutput *output = new TransactionOutput();
			ELATxOutput *o = (ELATxOutput *)output->getRaw();
			CMBlock script = getRandCMBlock(25);
			ELATxOutputSetScript(o, script, script.GetSize());
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

TEST_CASE("public function test", "[Transaction]") {
	srand(time(nullptr));

	SECTION("removeDuplicatePrograms test") {
		SECTION("situation 1") {
			Transaction tx;
			for (size_t i = 0; i < 20; ++i) {
				CMBlock code = getRandCMBlock(25);
				CMBlock parameter = getRandCMBlock(25);
				Program *program = new Program(code, parameter);
				tx.addProgram(program);
			}

			REQUIRE(tx.getPrograms().size() == 20);

			tx.addProgram(new Program(*tx.getPrograms()[0]));
			tx.addProgram(new Program(*tx.getPrograms()[1]));
			tx.addProgram(new Program(*tx.getPrograms()[1]));
			tx.addProgram(new Program(*tx.getPrograms()[3]));
			tx.addProgram(new Program(*tx.getPrograms()[5]));
			tx.addProgram(new Program(*tx.getPrograms()[7]));
			tx.addProgram(new Program(*tx.getPrograms()[19]));

			REQUIRE(tx.getPrograms().size() == 27);

			for (size_t i = 0; i < 20; ++i) {
				CMBlock code = getRandCMBlock(25);
				CMBlock parameter = getRandCMBlock(25);
				Program *program = new Program(code, parameter);
				tx.addProgram(program);
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
				Program *program = new Program(code, parameter);
				tx.addProgram(program);
			}

			REQUIRE(tx.getPrograms().size() == 20);

			for (size_t i = 0; i < 20; ++i) {
				tx.addProgram(new Program(*tx.getPrograms()[i]));
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
				Program *program = new Program(code, parameter);
				tx.addProgram(program);
			}

			REQUIRE(tx.getPrograms().size() == 20);

			tx.removeDuplicatePrograms();

			REQUIRE(tx.getPrograms().size() == 20);
		}

	}
}
