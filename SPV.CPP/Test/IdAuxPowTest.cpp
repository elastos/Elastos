// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"

#include <SDK/Plugin/Block/IdAuxPow.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

#include <Core/BRMerkleBlock.h>

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

		Transaction tx;
		initTransaction(tx);

		idAuxPow.setIdAuxBlockTx(tx);
//
//		ELAMerkleBlock *block = ELAMerkleBlockNew();
//		block->raw.blockHash = getRandUInt256();
//		block->raw.version = rand();
//		block->raw.prevBlock = getRandUInt256();
//		block->raw.merkleRoot = getRandUInt256();
//		block->raw.timestamp = rand();
//		block->raw.target = rand();
//		block->raw.nonce = rand();
//		block->raw.totalTx = rand();
//		block->raw.hashesCount = 10;
//		hashes.resize(block->raw.hashesCount);
//		for (size_t i = 0; i < block->raw.hashesCount; ++i) {
//			hashes[i] = getRandUInt256();
//		}
//		CMBlock flags = getRandCMBlock(5);
//		BRMerkleBlockSetTxHashes(&block->raw, hashes.data(), hashes.size(), flags, flags.GetSize());
//		idAuxPow.setMainBlockHeader(block);
//
//		ByteStream stream;
//		idAuxPow.Serialize(stream);

//		SPDLOG_DEBUG(Log::getLogger(),"idAuxPow serialization = {}", Utils::encodeHex(stream.getBuffer()));

		// verify
//		IdAuxPow idAuxPowVerify;
//		stream.setPosition(0);
//		REQUIRE(idAuxPowVerify.Deserialize(stream));
//
//		hashes = idAuxPow.getIdAuxMerkleBranch();
//		const std::vector<UInt256> hashesV = idAuxPowVerify.getIdAuxMerkleBranch();
//		REQUIRE(hashes.size() == hashesV.size());
//		for (size_t i = 0; i < hashes.size(); ++i) {
//			REQUIRE(UInt256Eq(&hashes[i], &hashesV[i]));
//		}
//
//		ELAMerkleBlock *b1 = idAuxPow.getMainBlockHeader();
//		ELAMerkleBlock *b2 = idAuxPowVerify.getMainBlockHeader();
//
//		REQUIRE(b1->auxPow.getParBlockHeader()->nonce == b2->auxPow.getParBlockHeader()->nonce);
//		REQUIRE(b1->auxPow.getParBlockHeader()->target == b2->auxPow.getParBlockHeader()->target);
	}

	SECTION("to and from json") {

	}

}
