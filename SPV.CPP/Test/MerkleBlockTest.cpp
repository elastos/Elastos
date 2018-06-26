// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <Core/BRMerkleBlock.h>
#include "BRMerkleBlock.h"
#include "Utils.h"
#include "catch.hpp"
#include "Log.h"
#include "SDK/Plugin/Block/MerkleBlock.h"
#include "TestHelper.h"

using namespace Elastos::ElaWallet;

TEST_CASE("MerkleBlock construct test", "[MerkleBlock]") {

	srand(time(nullptr));

	SECTION("serialize and deserialize") {
		ELAMerkleBlock *block = ELAMerkleBlockNew();

		block->raw.height = (uint32_t)rand();
		block->raw.timestamp = (uint32_t)rand();
		block->raw.version = (uint32_t)rand();

		block->raw.flagsLen = 10;
		block->raw.flags = (uint8_t *)malloc(block->raw.flagsLen);
		for (size_t i = 0; i < block->raw.flagsLen; ++i) {
			block->raw.flags[i] = (uint8_t)rand();
		}

		block->raw.hashesCount = 10;
		block->raw.hashes = (UInt256 *) malloc(sizeof(UInt256) * block->raw.hashesCount);
		for (size_t i = 0; i < block->raw.hashesCount; ++i) {
			block->raw.hashes[i] = getRandUInt256();
		}
		block->raw.merkleRoot = getRandUInt256();
		block->raw.nonce = (uint32_t)rand();
		block->raw.prevBlock = getRandUInt256();
		block->raw.target = (uint32_t)rand();
		block->raw.totalTx = (uint32_t)rand();

		std::vector<UInt256> hashes;
		for (size_t i = 0; i < 10; ++i) {
			hashes.push_back(getRandUInt256());
		}
		block->auxPow.setAuxMerkleBranch(hashes);

		hashes.clear();
		for (size_t i = 0; i < 10; ++i) {
			hashes.push_back(getRandUInt256());
		}
		block->auxPow.setCoinBaseMerkle(hashes);
		block->auxPow.setAuxMerkleIndex(rand());

		BRTransaction *tx = BRTransactionNew();
		tx->txHash = getRandUInt256();
		tx->version = (uint32_t)rand();
		for (size_t i = 0; i < 10; ++i) {
			CMBlock script = getRandCMBlock(25);
			CMBlock signature = getRandCMBlock(35);
			BRTransactionAddInput(tx, getRandUInt256(), (uint32_t)rand(), (uint64_t)rand(), script, script.GetSize(), signature, signature.GetSize(), (uint32_t)rand());
		}
		for (size_t i = 0; i < 10; ++i) {
			CMBlock script = getRandCMBlock(25);
			BRTransactionAddOutput(tx, rand(), script, script.GetSize());
		}
		tx->lockTime = rand();
		tx->blockHeight = rand();
		tx->timestamp = rand();
		block->auxPow.setBTCTransaction(tx);

		MerkleBlock mbOrig(block);
		ByteStream stream;
		mbOrig.Serialize(stream);

		MerkleBlock mb;
		stream.setPosition(0);
		mb.Deserialize(stream);
		ELAMerkleBlock *newBlock = (ELAMerkleBlock *)mb.getRaw();

		mbOrig.getBlockHash();
		mb.getBlockHash();

		REQUIRE(nullptr != mb.getRaw());

		REQUIRE(0 == memcmp(&newBlock->raw.blockHash, &block->raw.blockHash, sizeof(UInt256)));
		REQUIRE(newBlock->raw.height == block->raw.height);
		REQUIRE(newBlock->raw.timestamp == block->raw.timestamp);
		REQUIRE(newBlock->raw.version == block->raw.version);
		REQUIRE(newBlock->raw.flagsLen == block->raw.flagsLen);
		for (size_t i = 0; i < block->raw.flagsLen; ++i) {
			REQUIRE(newBlock->raw.flags[i] == block->raw.flags[i]);
		}
		REQUIRE(newBlock->raw.hashesCount == block->raw.hashesCount);
		for (size_t i = 0; i < block->raw.hashesCount; ++i) {
			REQUIRE(0 == memcmp(&newBlock->raw.hashes[i], &block->raw.hashes[i], sizeof(UInt256)));
		}
		REQUIRE(0 == memcmp(&newBlock->raw.hashes[1], &block->raw.hashes[1], sizeof(UInt256)));
		REQUIRE(0 == memcmp(&newBlock->raw.hashes[2], &block->raw.hashes[2], sizeof(UInt256)));
		REQUIRE(0 == memcmp(&newBlock->raw.merkleRoot, &block->raw.merkleRoot, sizeof(UInt256)));
		REQUIRE(newBlock->raw.nonce == block->raw.nonce);
		REQUIRE(0 == memcmp(&newBlock->raw.prevBlock, &block->raw.prevBlock, sizeof(UInt256)));
		REQUIRE(newBlock->raw.target == block->raw.target);
		REQUIRE(newBlock->raw.totalTx == block->raw.totalTx);
	}

}

TEST_CASE("Json convert", "[json]") {

	ELAMerkleBlock *merkleBlock = ELAMerkleBlockNew();
	MerkleBlock mb(merkleBlock);

	SECTION("Convert to json") {
		std::vector<std::string> hashes = {
			"000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943",
			"0000000000a33112f86f3f7b0aa590cb4949b84c2d9c673e9e303257b3be9000",
			"0000000000376bb71314321c45de3015fe958543afcbada242a3b1b072498e38",
			"0000000000001c93ebe0a7c33426e8edb9755505537ef9303a023f80be29d32d",
			"0000000000ef8b05da54711e2106907737741ac0278d59f358303c71d500f3c4",
			"0000000000005d105473c916cd9d16334f017368afea6bcee71629e0fcf2f4f5",
			"00000000000008653c7e5c00c703c5a9d53b318837bb1b3586a3d060ce6fff2e"
		};
		uint8_t flags[] = {1, 2, 3, 4, 5, 6};

		mb.getRaw()->blockHash = Utils::UInt256FromString("000000000000000002df2dd9d4fe0578392e519610e341dd09025469f101cfa1");
		mb.getRaw()->version = 11111;
		mb.getRaw()->prevBlock = Utils::UInt256FromString("00000000000000000f9cfece8494800d3dcbf9583232825da640c8703bcd27e7");
		mb.getRaw()->merkleRoot = Utils::UInt256FromString("000000000000000001630546cde8482cc183708f076a5e4d6f51cd24518e8f85");
		mb.getRaw()->timestamp = 22222;
		mb.getRaw()->target = 33333;
		mb.getRaw()->nonce = 44444;
		mb.getRaw()->totalTx = 55555;

		mb.getRaw()->hashesCount = hashes.size();
		mb.getRaw()->hashes = (UInt256 *)malloc(sizeof(UInt256) * hashes.size());
		for (int i = 0; i < hashes.size(); ++i) {
			memcpy(&mb.getRaw()->hashes[i], Utils::UInt256FromString(hashes[i]).u8, sizeof(UInt256));
		}

		mb.getRaw()->flagsLen = sizeof(flags) / sizeof(flags[0]);
		mb.getRaw()->flags = (uint8_t *)malloc(mb.getRaw()->flagsLen);
		for (int i = 0; i < mb.getRaw()->flagsLen; ++i) {
			mb.getRaw()->flags[i] = flags[i];
		}

		mb.getRaw()->height = 1000;

		nlohmann::json j = mb.toJson();

		REQUIRE(Utils::UInt256ToString(mb.getRaw()->blockHash) == j["BlockHash"].get<std::string>());
		REQUIRE(mb.getVersion() == j["Version"].get<uint32_t>());
		REQUIRE(Utils::UInt256ToString(mb.getPrevBlockHash()) == j["PrevBlock"].get<std::string>());
		REQUIRE(Utils::UInt256ToString(mb.getRootBlockHash()) == j["MerkleRoot"].get<std::string>());
		REQUIRE(mb.getTimestamp() == j["Timestamp"].get<uint32_t>());
		REQUIRE(mb.getTarget() == j["Target"].get<uint32_t>());
		REQUIRE(mb.getNonce() == j["Nonce"].get<uint32_t>());

		std::vector<std::string> jhashes = j["Hashes"].get<std::vector<std::string>>();
		REQUIRE(jhashes.size() == hashes.size());
		for (int i = 0; i < hashes.size(); ++i) {
			REQUIRE(jhashes[i] == hashes[i]);
		}

		std::vector<uint8_t> jflags = j["Flags"].get<std::vector<uint8_t>>();
		REQUIRE(mb.getRaw()->flagsLen == jflags.size());
		for (int i = 0; i < mb.getRaw()->flagsLen; ++i) {
			REQUIRE(jflags[i] == mb.getRaw()->flags[i]);
		}
		REQUIRE(mb.getHeight() == j["Height"].get<uint32_t>());
	}

	SECTION("Convert from json") {
		nlohmann::json j = {
			{"BlockHash", "000000000000000002df2dd9d4fe0578392e519610e341dd09025469f101cfa1"},
			{"Flags", {1,2,3,4,5,6}},
			{"Hashes", {
						   "000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943",
						   "0000000000a33112f86f3f7b0aa590cb4949b84c2d9c673e9e303257b3be9000",
						   "0000000000376bb71314321c45de3015fe958543afcbada242a3b1b072498e38",
						   "0000000000001c93ebe0a7c33426e8edb9755505537ef9303a023f80be29d32d",
						   "0000000000ef8b05da54711e2106907737741ac0278d59f358303c71d500f3c4",
						   "0000000000005d105473c916cd9d16334f017368afea6bcee71629e0fcf2f4f5",
						   "00000000000008653c7e5c00c703c5a9d53b318837bb1b3586a3d060ce6fff2e"
					   }},
			{"Height", 1000},
			{"MerkleRoot", "000000000000000001630546cde8482cc183708f076a5e4d6f51cd24518e8f85"},
			{"Nonce", 44444},
			{"PrevBlock", "00000000000000000f9cfece8494800d3dcbf9583232825da640c8703bcd27e7"},
			{"Target", 33333},
			{"Timestamp", 22222},
			{"TotalTx", 55555},
			{"Version", 11111}
		};

		mb.fromJson(j);

		REQUIRE(0 == memcmp(mb.getBlockHash().u8, Utils::UInt256FromString(j["BlockHash"].get<std::string>()).u8, sizeof(UInt256)));
		REQUIRE(j["Version"].get<uint32_t>() == mb.getVersion());
		REQUIRE(0 == memcmp(mb.getPrevBlockHash().u8, Utils::UInt256FromString(j["PrevBlock"].get<std::string>()).u8, sizeof(UInt256)));
		REQUIRE(0 == memcmp(mb.getRootBlockHash().u8, Utils::UInt256FromString(j["MerkleRoot"].get<std::string>()).u8, sizeof(UInt256)));
		REQUIRE(mb.getTimestamp() == j["Timestamp"].get<uint32_t>());
		REQUIRE(mb.getTarget() == j["Target"].get<uint32_t>());
		REQUIRE(mb.getNonce() == j["Nonce"].get<uint32_t>());
		REQUIRE(mb.getTransactionCount() == j["TotalTx"].get<uint32_t>());
		std::vector<std::string> jhashes = j["Hashes"].get<std::vector<std::string>>();
		REQUIRE(mb.getRaw()->hashesCount == jhashes.size());
		for (int i = 0; i < jhashes.size(); ++i) {
			REQUIRE(Utils::UInt256ToString(mb.getRaw()->hashes[i]) == jhashes[i]);
		}
		std::vector<uint8_t> jflags = j["Flags"].get<std::vector<uint8_t>>();
		REQUIRE(jflags.size() == mb.getRaw()->flagsLen);
		for (int i = 0; i < jflags.size(); ++i) {
			REQUIRE(jflags[i] == mb.getRaw()->flags[i]);
		}
	}

}
