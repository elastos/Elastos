// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <Core/BRMerkleBlock.h>
#include "BRMerkleBlock.h"
#include "Utils.h"
#include "catch.hpp"
#include "MerkleBlock.h"
#include "Log.h"

using namespace Elastos::ElaWallet;

TEST_CASE("MerkleBlock construct test", "[MerkleBlock]") {

	SECTION("deserialize and serialize") {
		std::string mbData = "000000000ac0575503833028ae62c2e91e67936cac40f5498f76ae38cb343386a74b61aefeb5cacf7b43c1"
					         "fed8e6a32bac7e6626548603e779b02fad199f0a0b37b343dbe790185bffff7f2000000000820000000100"
					         "0000010000000000000000000000000000000000000000000000000000000000000000000000002cfabe6d"
			                 "6d0f2e6091a91346460742c1a213878f665c6c07adf883f3973acb2cabfe77165c01000000000000000000"
			                 "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
					         "000000000000ffffff7f000000000000000000000000000000000000000000000000000000000000000018"
					         "5da84ff58cd5676504e3fcc3734d7e7cb96ee5b38daa0e3a4ed67ad3b9ea6ae690185b0000000000000000"
			                 "010100000001000000feb5cacf7b43c1fed8e6a32bac7e6626548603e779b02fad199f0a0b37b343db0100";

		CMBlock hex = Utils::decodeHex(mbData);

		std::string mbString = Utils::encodeHex(hex, hex.GetSize());

		REQUIRE(mbString == mbData);
		MerkleBlock mbOrig;
		ByteStream stream(hex, hex.GetSize(), false);
		mbOrig.Deserialize(stream);

		ByteStream newStream;
		mbOrig.Serialize(newStream);
		CMBlock buf = newStream.getBuffer();
		REQUIRE(buf.GetSize() == hex.GetSize());
		REQUIRE(0 == memcmp(hex, buf, buf.GetSize()));
	}

	SECTION("serialize and deserialize") {
		ELAMerkleBlock *block = ELAMerkleBlockNew();
		block->raw.height = 123;
		block->raw.timestamp = 456;
		block->raw.version = 10000;
		block->raw.flagsLen = 3;
		block->raw.flags = (uint8_t *)malloc(block->raw.flagsLen);
		block->raw.flags[0] = 33;
		block->raw.flags[1] = 44;
		block->raw.flags[2] = 55;
		block->raw.hashesCount = 3;
		block->raw.hashes = (UInt256 *) malloc(sizeof(UInt256) * block->raw.hashesCount);
		block->raw.hashes[0] = uint256("000000000008d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce25e");
		block->raw.hashes[1] = uint256("0000000000f7d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce24d");
		block->raw.hashes[2] = uint256("0000000000e6d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce23c");
		block->raw.merkleRoot = uint256("0000000000d5d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce22b");
		block->raw.nonce = 789;
		block->raw.prevBlock = uint256("0000000000c4d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce21a");
		block->raw.target = 111;
		block->raw.totalTx = 222;

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

		nlohmann::json j;
		to_json(j, mb);

		REQUIRE(Utils::UInt256ToString(mb.getRaw()->blockHash) == j["blockHash"].get<std::string>());
		REQUIRE(mb.getVersion() == j["version"].get<uint32_t>());
		REQUIRE(Utils::UInt256ToString(mb.getPrevBlockHash()) == j["prevBlock"].get<std::string>());
		REQUIRE(Utils::UInt256ToString(mb.getRootBlockHash()) == j["merkleRoot"].get<std::string>());
		REQUIRE(mb.getTimestamp() == j["timestamp"].get<uint32_t>());
		REQUIRE(mb.getTarget() == j["target"].get<uint32_t>());
		REQUIRE(mb.getNonce() == j["nonce"].get<uint32_t>());

		std::vector<std::string> jhashes = j["hashes"].get<std::vector<std::string>>();
		REQUIRE(jhashes.size() == hashes.size());
		for (int i = 0; i < hashes.size(); ++i) {
			REQUIRE(jhashes[i] == hashes[i]);
		}

		std::vector<uint8_t> jflags = j["flags"].get<std::vector<uint8_t>>();
		REQUIRE(mb.getRaw()->flagsLen == jflags.size());
		for (int i = 0; i < mb.getRaw()->flagsLen; ++i) {
			REQUIRE(jflags[i] == mb.getRaw()->flags[i]);
		}
		REQUIRE(mb.getHeight() == j["height"].get<uint32_t>());
	}

	SECTION("Convert from json") {
		nlohmann::json j = {
			{"blockHash", "000000000000000002df2dd9d4fe0578392e519610e341dd09025469f101cfa1"},
			{"flags", {1,2,3,4,5,6}},
			{"hashes", {
						   "000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943",
						   "0000000000a33112f86f3f7b0aa590cb4949b84c2d9c673e9e303257b3be9000",
						   "0000000000376bb71314321c45de3015fe958543afcbada242a3b1b072498e38",
						   "0000000000001c93ebe0a7c33426e8edb9755505537ef9303a023f80be29d32d",
						   "0000000000ef8b05da54711e2106907737741ac0278d59f358303c71d500f3c4",
						   "0000000000005d105473c916cd9d16334f017368afea6bcee71629e0fcf2f4f5",
						   "00000000000008653c7e5c00c703c5a9d53b318837bb1b3586a3d060ce6fff2e"
					   }},
			{"height", 1000},
			{"merkleRoot", "000000000000000001630546cde8482cc183708f076a5e4d6f51cd24518e8f85"},
			{"nonce", 44444},
			{"prevBlock", "00000000000000000f9cfece8494800d3dcbf9583232825da640c8703bcd27e7"},
			{"target", 33333},
			{"timestamp", 22222},
			{"totalTx", 55555},
			{"version", 11111}
		};

		from_json(j, mb);

		REQUIRE(0 == memcmp(mb.getBlockHash().u8, Utils::UInt256FromString(j["blockHash"].get<std::string>()).u8, sizeof(UInt256)));
		REQUIRE(j["version"].get<uint32_t>() == mb.getVersion());
		REQUIRE(0 == memcmp(mb.getPrevBlockHash().u8, Utils::UInt256FromString(j["prevBlock"].get<std::string>()).u8, sizeof(UInt256)));
		REQUIRE(0 == memcmp(mb.getRootBlockHash().u8, Utils::UInt256FromString(j["merkleRoot"].get<std::string>()).u8, sizeof(UInt256)));
		REQUIRE(mb.getTimestamp() == j["timestamp"].get<uint32_t>());
		REQUIRE(mb.getTarget() == j["target"].get<uint32_t>());
		REQUIRE(mb.getNonce() == j["nonce"].get<uint32_t>());
		REQUIRE(mb.getTransactionCount() == j["totalTx"].get<uint32_t>());
		std::vector<std::string> jhashes = j["hashes"].get<std::vector<std::string>>();
		REQUIRE(mb.getRaw()->hashesCount == jhashes.size());
		for (int i = 0; i < jhashes.size(); ++i) {
			REQUIRE(Utils::UInt256ToString(mb.getRaw()->hashes[i]) == jhashes[i]);
		}
		std::vector<uint8_t> jflags = j["flags"].get<std::vector<uint8_t>>();
		REQUIRE(jflags.size() == mb.getRaw()->flagsLen);
		for (int i = 0; i < jflags.size(); ++i) {
			REQUIRE(jflags[i] == mb.getRaw()->flags[i]);
		}
	}

}
