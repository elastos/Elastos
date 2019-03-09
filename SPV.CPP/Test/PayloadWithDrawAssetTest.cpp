// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"
#include <SDK/Plugin/Transaction/Payload/PayloadWithDrawAsset.h>

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadWithDrawAsset Test", "[PayloadWithDrawAsset]") {

	srand(time(nullptr));

	SECTION("Serialize and deserialize test") {
		PayloadWithDrawAsset p1, p2;
		ByteStream stream;

		p1.SetBlockHeight(rand());
		p1.SetGenesisBlockAddress(getRandString(100));
		std::vector<UInt256> hashes;
		for (size_t i = 0; i < 20; ++i) {
			hashes.push_back(getRandUInt256());
		}
		p1.SetSideChainTransacitonHash(hashes);
		p1.Serialize(stream, 0);

		stream.SetPosition(0);
		REQUIRE(p2.Deserialize(stream, 0));

		REQUIRE(p1.GetBlockHeight() == p2.GetBlockHeight());
		REQUIRE(p1.GetGenesisBlockAddress() == p2.GetGenesisBlockAddress());
		std::vector<UInt256> pbHashes = p2.GetSideChainTransacitonHash();
		REQUIRE(hashes.size() == pbHashes.size());
		for (size_t i = 0; i < pbHashes.size(); ++i) {
			REQUIRE(UInt256Eq(&hashes[i], &pbHashes[i]));
		}
	}

	SECTION("to json and from json") {
		PayloadWithDrawAsset p1, p2;

		p1.SetBlockHeight(rand());
		p1.SetGenesisBlockAddress(getRandString(100));
		std::vector<UInt256> hashes;
		for (size_t i = 0; i < 20; ++i) {
			hashes.push_back(getRandUInt256());
		}
		p1.SetSideChainTransacitonHash(hashes);

		nlohmann::json j = p1.ToJson(0);

		p2.FromJson(j, 0);

		REQUIRE(p1.GetBlockHeight() == p2.GetBlockHeight());
		REQUIRE(p1.GetGenesisBlockAddress() == p2.GetGenesisBlockAddress());
		std::vector<UInt256> pbHashes = p2.GetSideChainTransacitonHash();
		REQUIRE(hashes.size() == pbHashes.size());
		for (size_t i = 0; i < pbHashes.size(); ++i) {
			REQUIRE(UInt256Eq(&hashes[i], &pbHashes[i]));
		}
	}

}