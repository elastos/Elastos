// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <Core/BRMerkleBlock.h>
#include <SDK/Common/Utils.h>

#include "catch.hpp"
#include "SDK/Plugin/Block/IdAuxPow.h"
#include "Log.h"
#include "TestHelper.h"

using namespace Elastos::ElaWallet;

TEST_CASE("IdAuxPow test", "[IdAuxPow]") {
	srand((unsigned int)time(nullptr));

	SECTION("Serialize and deserialize") {
		IdAuxPow idAuxPow;

		std::vector<UInt256> hashes;
		hashes.resize(20);
		for (size_t i = 0; i < hashes.size(); ++i) {
			hashes[i] = getRandUInt256();
		}

		idAuxPow.setIdAuxMerkleBranch(hashes);
		idAuxPow.setIdAuxMerkleIndex(rand());

		ELATransaction *tx = ELATransactionNew();
		tx->raw.txHash = getRandUInt256();
		tx->raw.version = rand();
		for (size_t i = 0; i < 10; ++i) {
			CMBlock script = getRandCMBlock(25);
			CMBlock signature = getRandCMBlock(35);
			UInt256 txHash = getRandUInt256();
			BRTransactionAddInput(&tx->raw, txHash, rand(), rand(),
								  script, script.GetSize(), signature, signature.GetSize(), rand());
		}

		tx->raw.lockTime = rand();
		tx->raw.blockHeight = rand();
		tx->raw.timestamp = rand();
		tx->type = ELATransaction::Type::TransferAsset;//ELATransaction::Type(rand() % ELATransaction::Type::TypeMaxCount);
		tx->payloadVersion = rand() % sizeof(tx->payloadVersion);
		tx->fee = rand();
		tx->payload = ELAPayloadNew(tx->type);

		for (size_t i = 0; i < 10; ++i) {
			TransactionOutputPtr output(new TransactionOutput());
			ELATxOutput *o = (ELATxOutput *)output->getRaw();
			CMBlock script = getRandCMBlock(25);
			BRTxOutputSetScript(&o->raw, script, script.GetSize());
			o->raw.amount = rand();
			o->assetId = getRandUInt256();
			o->outputLock = rand();
			o->programHash = getRandUInt168();
			tx->outputs.push_back(output);
		}

		for (size_t i = 0; i < 10; ++i) {
			CMBlock script = getRandCMBlock(25);
			AttributePtr attr(new Attribute(Attribute::Script, script));
			tx->attributes.push_back(attr);
		}

		for (size_t i = 0; i < 10; ++i) {
			CMBlock code = getRandCMBlock(35);
			CMBlock parameter = getRandCMBlock(45);
			ProgramPtr program(new Program(code, parameter));
			tx->programs.push_back(program);
		}

		Transaction txn(tx);

		idAuxPow.setIdAuxBlockTx(txn);

		ELAMerkleBlock *block = ELAMerkleBlockNew();
		block->raw.blockHash = getRandUInt256();
		block->raw.version = rand();
		block->raw.prevBlock = getRandUInt256();
		block->raw.merkleRoot = getRandUInt256();
		block->raw.timestamp = rand();
		block->raw.target = rand();
		block->raw.nonce = rand();
		block->raw.totalTx = rand();
		block->raw.hashesCount = 10;
		hashes.resize(block->raw.hashesCount);
		for (size_t i = 0; i < block->raw.hashesCount; ++i) {
			hashes[i] = getRandUInt256();
		}
		CMBlock flags = getRandCMBlock(5);
		BRMerkleBlockSetTxHashes(&block->raw, hashes.data(), hashes.size(), flags, flags.GetSize());
		idAuxPow.setMainBlockHeader(block);

		ByteStream stream;
		idAuxPow.Serialize(stream);

//		Log::getLogger()->info("idAuxPow serialization = {}", Utils::encodeHex(stream.getBuffer()));

		// verify
		IdAuxPow idAuxPowVerify;
		stream.setPosition(0);
		REQUIRE(idAuxPowVerify.Deserialize(stream));

		hashes = idAuxPow.getIdAuxMerkleBranch();
		const std::vector<UInt256> hashesV = idAuxPowVerify.getIdAuxMerkleBranch();
		REQUIRE(hashes.size() == hashesV.size());
		for (size_t i = 0; i < hashes.size(); ++i) {
			REQUIRE(UInt256Eq(&hashes[i], &hashesV[i]));
		}

		ELAMerkleBlock *b1 = idAuxPow.getMainBlockHeader();
		ELAMerkleBlock *b2 = idAuxPowVerify.getMainBlockHeader();

		REQUIRE(b1->auxPow.getParBlockHeader()->nonce == b2->auxPow.getParBlockHeader()->nonce);
		REQUIRE(b1->auxPow.getParBlockHeader()->target == b2->auxPow.getParBlockHeader()->target);
	}

	SECTION("to and from json") {

	}

}
