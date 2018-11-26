// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <Core/BRTransaction.h>
#include "SDK/Transaction/TransactionOutput.h"
#include "catch.hpp"
#include "SDK/Transaction/Transaction.h"
#include "BRTransaction.h"
#include "SDK/Base/Address.h"
#include "Payload/PayloadCoinBase.h"
#include <SDK/Common/Utils.h>
#include "Log.h"
#include "TestHelper.h"

using namespace Elastos::ElaWallet;

static Transaction *createELATransaction() {
	Transaction *tx = new Transaction();

	tx->setVersion(rand());
	for (size_t i = 0; i < 20; ++i) {
		TransactionInput input;
		input.setTransactionHash(getRandUInt256());
		input.setIndex((uint16_t) rand());
		input.setSequence(rand());
		tx->addInput(input);
	}

	for (size_t i = 0; i < 20; ++i) {
		TransactionOutput output;
		output.setAssetId(getRandUInt256());
		output.setProgramHash(getRandUInt168());
		output.setOutputLock(rand());
		output.setAddressSignType(rand());
		output.setAmount(rand());
		CMBlock script = getRandCMBlock(25);
		output.SetScript(script, output.getAddressSignType());
		tx->addOutput(output);
	}
	tx->setLockTime(rand());
	tx->setBlockHeight(rand());
	tx->setTimestamp(rand());

	tx->setTransactionType(Transaction::TransferAsset);
	tx->initPayloadFromType(tx->getTransactionType());

	tx->setPayloadVersion(rand());
	tx->setFee(rand());

	for (size_t i = 0; i < 20; ++i) {
		CMBlock data = getRandCMBlock(25);
		tx->addAttribute(Attribute(Attribute::Script, data));
	}

	for (size_t i = 0; i < 20; ++i) {
		CMBlock code = getRandCMBlock(25);
		CMBlock parameter = getRandCMBlock(25);
		tx->addProgram(Program(code, parameter));
	}

	tx->setRemark(getRandString(40));

	return tx;
}

//fixme [refactor] complete me later
//static void verifyELATransaction(const Transaction *tx1, const Transaction *tx, bool checkAll = true) {
//	REQUIRE(tx1->getOutputs().size() == tx->getOutputs().size());
//	REQUIRE(UInt256Eq(&tx1->getHash(), &tx->getHash()));
//	if (checkAll)
//		REQUIRE(tx1->getVersion() == tx->getVersion());
//	REQUIRE(tx1->getInputs().size() == tx->getInputs().size());
//	for (size_t i = 0; i < tx1->getInputs().size(); ++i) {
//		REQUIRE(UInt256Eq(&tx1->getInputs()[i].getTransctionHash(), &tx->getInputs()[i].getTransctionHash()));
//		REQUIRE(tx1->->getInputs()[i].getIndex() == tx->getInputs()[i].getIndex());
//		REQUIRE(tx1->getInputs()[i].getSequence() == tx->getInputs()[i].getSequence());
//	}
//
//	REQUIRE(tx1->getOutputs().size() == tx->getOutputs().size());
//	for (size_t i = 0; i < tx->outputs.size(); ++i) {
//		REQUIRE(UInt256Eq(&tx->outputs[i]->getAssetId(), &tx1->outputs[i]->getAssetId()));
//		REQUIRE(tx->outputs[i]->getAddressSignType() == tx1->outputs[i]->getAddressSignType());
//		REQUIRE(UInt168Eq(&tx->outputs[i]->getProgramHash(), &tx1->outputs[i]->getProgramHash()));
//		REQUIRE(tx->outputs[i]->getOutputLock() == tx1->outputs[i]->getOutputLock());
//		if (checkAll) {
//			REQUIRE(tx->outputs[i]->getScript().GetSize() == tx1->outputs[i]->getScript().GetSize());
//			REQUIRE(!memcmp(tx->outputs[i]->getScript(), tx1->outputs[i]->getScript(),
//							tx->outputs[i]->getScript().GetSize()));
//		}
//		REQUIRE(tx->outputs[i]->getAmount() == tx1->outputs[i]->getAmount());
//		if (checkAll) {
//			REQUIRE(tx->outputs[i]->getAddress() == tx1->outputs[i]->getAddress());
//		}
//	}
//
//	REQUIRE(tx1->raw.lockTime == tx->raw.lockTime);
//	if (checkAll) {
//		REQUIRE(tx1->raw.blockHeight == tx->raw.blockHeight);
//		REQUIRE(tx1->raw.timestamp == tx->raw.timestamp);
//	}
//	REQUIRE(tx1->type == tx->type);
//	REQUIRE(tx1->_payloadVersion == tx->_payloadVersion);
//	if (checkAll) {
//		REQUIRE(tx1->fee == tx->fee);
//		REQUIRE(tx1->Remark == tx->Remark);
//	}
//	REQUIRE(tx1->attributes.size() == tx->attributes.size());
//	for (size_t i = 0; i < tx1->attributes.size(); ++i) {
//		REQUIRE(tx1->attributes[i]->GetUsage() == tx->attributes[i]->GetUsage());
//		const CMBlock &data = tx->attributes[i]->GetData();
//		const CMBlock &data1 = tx1->attributes[i]->GetData();
//		REQUIRE(data1.GetSize() == data.GetSize());
//		REQUIRE(!memcmp(data1, data, data.GetSize()));
//	}
//
//	REQUIRE(tx1->programs.size() == tx->programs.size());
//	for (size_t i = 0; i < tx->programs.size(); ++i) {
//		const CMBlock &code = tx->programs[i]->getCode();
//		const CMBlock &code1 = tx1->programs[i]->getCode();
//		REQUIRE(code1.GetSize() == code.GetSize());
//		REQUIRE(!memcmp(code1, code, code.GetSize()));
//		const CMBlock &parameter = tx->programs[i]->getParameter();
//		const CMBlock &parameter1 = tx1->programs[i]->getParameter();
//		REQUIRE(parameter1.GetSize() == parameter.GetSize());
//		REQUIRE(!memcmp(parameter1, parameter, parameter.GetSize()));
//	}
//}
//
//TEST_CASE("Transaction Serialize and Deserialize", "[Transaction]") {
//	srand(time(nullptr));
//
//	SECTION("transaction Serialize test") {
//		Transaction *tx = createELATransaction();
//
//		Transaction txn(tx);
//		tx->raw.txHash = txn.getHash();
//
//		ByteStream stream;
//		txn.Serialize(stream);
//
//		Transaction txn1;
//		stream.setPosition(0);
//		REQUIRE(txn1.Deserialize(stream));
//
//		Transaction *tx1 = (Transaction *) txn1.getRaw();
//
//		verifyELATransaction(tx1, tx, false);
//
//		Transaction txn2 = txn;
//		Transaction *tx2 = (Transaction *) txn2.getRaw();
//
//		verifyELATransaction(tx2, tx, true);
//	}
//
//}
//
//TEST_CASE("Convert to and from json", "[Transaction]") {
//	srand(time(nullptr));
//
//	SECTION("to and from json") {
//		Transaction tx;
//		tx.isRegistered() = true;
//		Transaction *ela = (Transaction *) tx.getRaw();
//
//		ela->raw.txHash = getRandUInt256();
//		ela->raw.version = rand();
//		for (size_t i = 0; i < 3; ++i) {
//			CMBlock script = getRandCMBlock(25);
//			CMBlock signature = getRandCMBlock(28);
//			BRTransactionAddInput(&ela->raw, getRandUInt256(), i, rand(),
//								  script, script.GetSize(), signature, signature.GetSize(), rand());
//		}
//
//		ela->raw.lockTime = rand();
//		ela->raw.blockHeight = rand();
//		ela->raw.timestamp = rand();
//		ela->type = Transaction::Type::TransferAsset;// Transaction::Type(rand() % Transaction::Type::TypeMaxCount);
//		ela->_payloadVersion = rand() % sizeof(ela->_payloadVersion);
//		ela->fee = rand();
//		delete ela->payload;
//		ela->payload = ELAPayloadNew(ela->type);
//
//		for (size_t i = 0; i < 4; ++i) {
//			TransactionOutput *output = new TransactionOutput();
//			CMBlock script = getRandCMBlock(25);
//			output->SetScript(script, output->getAddressSignType());
//			output->setAmount(rand());
//			output->setAssetId(getRandUInt256());
//			output->setOutputLock(rand());
//			output->setProgramHash(getRandUInt168());
//			ela->outputs.push_back(output);
//		}
//
//		nlohmann::json txJson = tx.toJson();
//
//		/* verify transaction */
//		Transaction txn;
//		txn.fromJson(txJson);
//
//		// TODO [heropan] complete me later
//	}
//}
//
//TEST_CASE("public function test", "[Transaction]") {
//	srand(time(nullptr));
//
//	SECTION("removeDuplicatePrograms test") {
//		SECTION("situation 1") {
//			Transaction tx;
//			for (size_t i = 0; i < 20; ++i) {
//				CMBlock code = getRandCMBlock(25);
//				CMBlock parameter = getRandCMBlock(25);
//				Program *program = new Program(code, parameter);
//				tx.addProgram(program);
//			}
//
//			REQUIRE(tx.getPrograms().size() == 20);
//
//			tx.addProgram(new Program(*tx.getPrograms()[0]));
//			tx.addProgram(new Program(*tx.getPrograms()[1]));
//			tx.addProgram(new Program(*tx.getPrograms()[1]));
//			tx.addProgram(new Program(*tx.getPrograms()[3]));
//			tx.addProgram(new Program(*tx.getPrograms()[5]));
//			tx.addProgram(new Program(*tx.getPrograms()[7]));
//			tx.addProgram(new Program(*tx.getPrograms()[19]));
//
//			REQUIRE(tx.getPrograms().size() == 27);
//
//			for (size_t i = 0; i < 20; ++i) {
//				CMBlock code = getRandCMBlock(25);
//				CMBlock parameter = getRandCMBlock(25);
//				Program *program = new Program(code, parameter);
//				tx.addProgram(program);
//			}
//
//			REQUIRE(tx.getPrograms().size() == 47);
//
//			tx.removeDuplicatePrograms();
//
//			REQUIRE(tx.getPrograms().size() == 40);
//		}
//
//		SECTION("situation 2") {
//			Transaction tx;
//			for (size_t i = 0; i < 20; ++i) {
//				CMBlock code = getRandCMBlock(25);
//				CMBlock parameter = getRandCMBlock(25);
//				Program *program = new Program(code, parameter);
//				tx.addProgram(program);
//			}
//
//			REQUIRE(tx.getPrograms().size() == 20);
//
//			for (size_t i = 0; i < 20; ++i) {
//				tx.addProgram(new Program(*tx.getPrograms()[i]));
//			}
//
//			REQUIRE(tx.getPrograms().size() == 40);
//
//			tx.removeDuplicatePrograms();
//
//			REQUIRE(tx.getPrograms().size() == 20);
//		}
//
//		SECTION("situation 3") {
//			Transaction tx;
//			for (size_t i = 0; i < 20; ++i) {
//				CMBlock code = getRandCMBlock(25);
//				CMBlock parameter = getRandCMBlock(25);
//				Program *program = new Program(code, parameter);
//				tx.addProgram(program);
//			}
//
//			REQUIRE(tx.getPrograms().size() == 20);
//
//			tx.removeDuplicatePrograms();
//
//			REQUIRE(tx.getPrograms().size() == 20);
//		}
//
//	}
//}
