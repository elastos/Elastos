// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Interface/IMerkleBlock.h>
#include <SDK/Plugin/Registry.h>

#include "catch.hpp"
#include "TestHelper.h"

using namespace Elastos::ElaWallet;

TEST_CASE("MerkleBlock construct test", "[MerkleBlock]") {

	srand(time(nullptr));

	SECTION("serialize and deserialize") {
		MerkleBlockPtr merkleBlock = Registry::Instance()->CreateMerkleBlock("ELA");
		REQUIRE(merkleBlock != nullptr);
		setMerkleBlockValues(static_cast<MerkleBlock *>(merkleBlock.get()));

		ByteStream stream;
		merkleBlock->Serialize(stream);

		MerkleBlock mb;
		stream.setPosition(0);
		mb.Deserialize(stream);

		verifyELAMerkleBlock(static_cast<const MerkleBlock &>(*merkleBlock), mb);
	}
}

TEST_CASE("Json convert", "[json]") {

	MerkleBlock mb;

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

		mb.setVersion(11111);
		mb.setPrevBlockHash(Utils::UInt256FromString(
				"00000000000000000f9cfece8494800d3dcbf9583232825da640c8703bcd27e7", true));
		mb.setRootBlockHash(Utils::UInt256FromString(
				"000000000000000001630546cde8482cc183708f076a5e4d6f51cd24518e8f85", true));
		mb.setTimestamp(22222);
		mb.setTarget( 33333);
		mb.setNonce( 44444);
		mb.setTransactionCount(55555);

		std::vector<UInt256> hashList;
		for (int i = 0; i < hashes.size(); ++i) {
			hashList.push_back(Utils::UInt256FromString(hashes[i], true));
		}
		mb.setHashes(hashList);

		std::vector<uint8_t> flagList;
		for (int i = 0; i < sizeof(flags) / sizeof(flags[0]); ++i) {
			flagList.push_back(flags[i]);
		}
		mb.setFlags(flagList);

		mb.setHeight(1000);

		std::string blockHash = Utils::UInt256ToString(mb.getHash(), true);

		nlohmann::json j = mb.toJson();

		REQUIRE(blockHash == j["BlockHash"].get<std::string>());
		REQUIRE(mb.getVersion() == j["Version"].get<uint32_t>());
		REQUIRE(Utils::UInt256ToString(mb.getPrevBlockHash(), true) == j["PrevBlock"].get<std::string>());
		REQUIRE(Utils::UInt256ToString(mb.getRootBlockHash(), true) == j["MerkleRoot"].get<std::string>());
		REQUIRE(mb.getTimestamp() == j["Timestamp"].get<uint32_t>());
		REQUIRE(mb.getTarget() == j["Target"].get<uint32_t>());
		REQUIRE(mb.getNonce() == j["Nonce"].get<uint32_t>());

		std::vector<std::string> jhashes = j["Hashes"].get<std::vector<std::string>>();
		REQUIRE(jhashes.size() == mb.getHashes().size());
		for (int i = 0; i < mb.getHashes().size(); ++i) {
			REQUIRE(jhashes[i] == hashes[i]);
		}

		std::vector<uint8_t> jflags = j["Flags"].get<std::vector<uint8_t>>();
		REQUIRE(mb.getFlags().size() == jflags.size());
		for (int i = 0; i < mb.getFlags().size(); ++i) {
			REQUIRE(jflags[i] == mb.getFlags()[i]);
		}
		REQUIRE(mb.getHeight() == j["Height"].get<uint32_t>());
	}

	SECTION("Convert from json") {
		nlohmann::json j = {
				{"BlockHash",  "000000000000000002df2dd9d4fe0578392e519610e341dd09025469f101cfa1"},
				{"Flags",      {1, 2, 3, 4, 5, 6}},
				{"Hashes",     {
								"000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943",
								   "0000000000a33112f86f3f7b0aa590cb4949b84c2d9c673e9e303257b3be9000",
									  "0000000000376bb71314321c45de3015fe958543afcbada242a3b1b072498e38",
										 "0000000000001c93ebe0a7c33426e8edb9755505537ef9303a023f80be29d32d",
											"0000000000ef8b05da54711e2106907737741ac0278d59f358303c71d500f3c4",
											   "0000000000005d105473c916cd9d16334f017368afea6bcee71629e0fcf2f4f5",
									   "00000000000008653c7e5c00c703c5a9d53b318837bb1b3586a3d060ce6fff2e"
							   }},
				{"Height",     1000},
				{"MerkleRoot", "000000000000000001630546cde8482cc183708f076a5e4d6f51cd24518e8f85"},
				{"Nonce",      44444},
				{"PrevBlock",  "00000000000000000f9cfece8494800d3dcbf9583232825da640c8703bcd27e7"},
				{"Target",     33333},
				{"Timestamp",  22222},
				{"TotalTx",    55555},
				{"Version",    11111}
		};

		mb.fromJson(j);

		REQUIRE(0 == memcmp(mb.getHash().u8, Utils::UInt256FromString(j["BlockHash"].get<std::string>(), true).u8,
							sizeof(UInt256)));
		REQUIRE(j["Version"].get<uint32_t>() == mb.getVersion());
		REQUIRE(0 == memcmp(mb.getPrevBlockHash().u8, Utils::UInt256FromString(j["PrevBlock"].get<std::string>(), true).u8,
							sizeof(UInt256)));
		REQUIRE(0 == memcmp(mb.getRootBlockHash().u8, Utils::UInt256FromString(j["MerkleRoot"].get<std::string>(), true).u8,
							sizeof(UInt256)));
		REQUIRE(mb.getTimestamp() == j["Timestamp"].get<uint32_t>());
		REQUIRE(mb.getTarget() == j["Target"].get<uint32_t>());
		REQUIRE(mb.getNonce() == j["Nonce"].get<uint32_t>());
		REQUIRE(mb.getTransactionCount() == j["TotalTx"].get<uint32_t>());
		std::vector<std::string> jhashes = j["Hashes"].get<std::vector<std::string>>();
		REQUIRE(mb.getHashes().size() == jhashes.size());
		for (int i = 0; i < jhashes.size(); ++i) {
			REQUIRE(Utils::UInt256ToString(mb.getHashes()[i], true) == jhashes[i]);
		}
		std::vector<uint8_t> jflags = j["Flags"].get<std::vector<uint8_t>>();
		REQUIRE(jflags.size() == mb.getFlags().size());
		for (int i = 0; i < jflags.size(); ++i) {
			REQUIRE(jflags[i] == mb.getFlags()[i]);
		}
	}

}
