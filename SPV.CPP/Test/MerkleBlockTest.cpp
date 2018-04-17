// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "BRMerkleBlock.h"
#include "Utils.h"
#include "catch.hpp"
#include "MerkleBlock.h"
#include "Log.h"

using namespace Elastos::SDK;

TEST_CASE("MerkleBlock construct test", "[MerkleBlock]") {

	BRMerkleBlock *merkleBlock = BRMerkleBlockNew();

	UInt256 u256Empty = {0};

	SECTION("Construct with BRMerkleBlock pointer") {
		MerkleBlock mb(merkleBlock);
		REQUIRE(nullptr != mb.getRaw());
		REQUIRE(0 == memcmp(u256Empty.u8, mb.getBlockHash().u8, sizeof(UInt256)));
		REQUIRE(false == mb.isValid(time(NULL)));
	}

	SECTION("Construct with nullptr pointer") {
		MerkleBlock mb(nullptr);
		REQUIRE(nullptr == mb.getRaw());
	}

	SECTION("Construct with serial data") {
		MerkleBlock mbTemp(merkleBlock);
		ByteData serializedData = mbTemp.serialize();
		MerkleBlock mb(serializedData, 0);

		REQUIRE(nullptr != mb.getRaw());
	}

}

TEST_CASE("Json convert", "[json]") {

	BRMerkleBlock *merkleBlock = BRMerkleBlockNew();
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

//		std::cout << j << std::endl;

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
