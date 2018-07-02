// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BRInt.h"
#include "Payload/PayloadWithDrawAsset.h"
#include "TestHelper.h"

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadWithDrawAsset Test", "[PayloadWithDrawAsset]") {

	srand(time(nullptr));

	SECTION("Serialize and deserialize test") {
		PayloadWithDrawAsset pa, pb;
		ByteStream stream;

		pa.setBlockHeight(rand());
		pa.setGenesisBlockAddress(getRandString(100));
		std::vector<UInt256> hashes;
		for (size_t i = 0; i < 20; ++i) {
			hashes.push_back(getRandUInt256());
		}
		pa.setSideChainTransacitonHash(hashes);
		pa.Serialize(stream);

		stream.setPosition(0);
		REQUIRE(pb.Deserialize(stream));

		REQUIRE(pa.getBlockHeight() == pb.getBlockHeight());
		REQUIRE(pa.getGenesisBlockAddress() == pb.getGenesisBlockAddress());
		std::vector<UInt256> pbHashes = pb.getSideChainTransacitonHash();
		REQUIRE(hashes.size() == pbHashes.size());
		for (size_t i = 0; i < pbHashes.size(); ++i) {
			REQUIRE(UInt256Eq(&hashes[i], &pbHashes[i]));
		}
	}

	SECTION("To and from json") {
		PayloadWithDrawAsset pa, pb;

		pa.setBlockHeight(rand());
		pa.setGenesisBlockAddress(getRandString(100));
		std::vector<UInt256> hashes;
		for (size_t i = 0; i < 20; ++i) {
			hashes.push_back(getRandUInt256());
		}
		pa.setSideChainTransacitonHash(hashes);

		nlohmann::json j = pa.toJson();

		pb.fromJson(j);

		REQUIRE(pa.getBlockHeight() == pb.getBlockHeight());
		REQUIRE(pa.getGenesisBlockAddress() == pb.getGenesisBlockAddress());
		std::vector<UInt256> pbHashes = pb.getSideChainTransacitonHash();
		REQUIRE(hashes.size() == pbHashes.size());
		for (size_t i = 0; i < pbHashes.size(); ++i) {
			REQUIRE(UInt256Eq(&hashes[i], &pbHashes[i]));
		}
	}

}