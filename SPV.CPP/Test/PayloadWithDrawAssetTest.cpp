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

		p1.setBlockHeight(rand());
		p1.setGenesisBlockAddress(getRandString(100));
		std::vector<UInt256> hashes;
		for (size_t i = 0; i < 20; ++i) {
			hashes.push_back(getRandUInt256());
		}
		p1.setSideChainTransacitonHash(hashes);
		p1.Serialize(stream);

		stream.setPosition(0);
		REQUIRE(p2.Deserialize(stream));

		REQUIRE(p1.getBlockHeight() == p2.getBlockHeight());
		REQUIRE(p1.getGenesisBlockAddress() == p2.getGenesisBlockAddress());
		std::vector<UInt256> pbHashes = p2.getSideChainTransacitonHash();
		REQUIRE(hashes.size() == pbHashes.size());
		for (size_t i = 0; i < pbHashes.size(); ++i) {
			REQUIRE(UInt256Eq(&hashes[i], &pbHashes[i]));
		}
	}

	SECTION("to json and from json") {
		PayloadWithDrawAsset p1, p2;

		p1.setBlockHeight(rand());
		p1.setGenesisBlockAddress(getRandString(100));
		std::vector<UInt256> hashes;
		for (size_t i = 0; i < 20; ++i) {
			hashes.push_back(getRandUInt256());
		}
		p1.setSideChainTransacitonHash(hashes);

		nlohmann::json j = p1.toJson();

		p2.fromJson(j);

		REQUIRE(p1.getBlockHeight() == p2.getBlockHeight());
		REQUIRE(p1.getGenesisBlockAddress() == p2.getGenesisBlockAddress());
		std::vector<UInt256> pbHashes = p2.getSideChainTransacitonHash();
		REQUIRE(hashes.size() == pbHashes.size());
		for (size_t i = 0; i < pbHashes.size(); ++i) {
			REQUIRE(UInt256Eq(&hashes[i], &pbHashes[i]));
		}
	}

}