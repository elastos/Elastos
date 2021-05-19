// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Block/IDAuxPow.h>
#include <Common/Log.h>
#include <Common/Utils.h>

#include <bitcoin/BRMerkleBlock.h>

using namespace Elastos::ElaWallet;

TEST_CASE("IdAuxPow test", "[IdAuxPow]") {
	srand((unsigned int)time(nullptr));
	Log::registerMultiLogger();

	SECTION("Serialize and deserialize") {
		IDAuxPow ap1;

		std::vector<uint256> hashes;
		hashes.resize(20);
		for (size_t i = 0; i < hashes.size(); ++i) {
			hashes[i] = getRanduint256();
		}

		ap1.SetIdAuxMerkleBranch(hashes);
		ap1.SetIdAuxMerkleIndex(getRandUInt32());

		Transaction tx;
		initTransaction(tx, Transaction::TxVersion::V09);

		ap1.SetIdAuxBlockTx(tx);

		ELAMerkleBlock *block = ELAMerkleBlockNew();
		block->raw.blockHash = getRandUInt256();
		block->raw.version = getRandUInt32();
		block->raw.prevBlock = getRandUInt256();
		block->raw.merkleRoot = getRandUInt256();
		block->raw.timestamp = getRandUInt32();
		block->raw.target = getRandUInt32();
		block->raw.nonce = getRandUInt32();
		block->raw.totalTx = getRandUInt32();
		block->raw.hashesCount = 10;
		hashes.resize(block->raw.hashesCount);

		std::vector<UInt256> Hashes(block->raw.hashesCount);
		for (size_t i = 0; i < block->raw.hashesCount; ++i) {
			Hashes[i] = getRandUInt256();
		}
		bytes_t flags = getRandBytes(5);
		BRMerkleBlockSetTxHashes(&block->raw, Hashes.data(), Hashes.size(), flags.data(), flags.size());
		ap1.SetMainBlockHeader(block);

		ByteStream stream;
		ap1.Serialize(stream);


//		 verify
		IDAuxPow ap2;
		REQUIRE(ap2.Deserialize(stream));

		const std::vector<uint256> &hashes1 = ap1.GetIdAuxMerkleBranch();
		const std::vector<uint256> &hashes2 = ap2.GetIdAuxMerkleBranch();
		REQUIRE(hashes1.size() == hashes2.size());
		for (size_t i = 0; i < hashes1.size(); ++i) {
			REQUIRE(hashes1[i] == hashes2[i]);
		}

		ELAMerkleBlock *b1 = ap1.GetMainBlockHeader();
		ELAMerkleBlock *b2 = ap2.GetMainBlockHeader();

		REQUIRE(b1->auxPow.GetParBlockHeader()->nonce == b2->auxPow.GetParBlockHeader()->nonce);
		REQUIRE(b1->auxPow.GetParBlockHeader()->target == b2->auxPow.GetParBlockHeader()->target);

		const Transaction &tx1 = ap1.GetIdAuxBlockTx();
		const Transaction &tx2 = ap2.GetIdAuxBlockTx();

		verifyTransaction(tx1, tx2, false);
	}

}
