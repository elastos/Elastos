// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"
#include <Plugin/Transaction/Payload/WithdrawFromSideChain.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("WithdrawFromSideChain Test", "[WithdrawFromSideChain]") {
	Log::registerMultiLogger();

	srand(time(nullptr));

	SECTION("Serialize and deserialize test") {
		WithdrawFromSideChain p1, p2;
		ByteStream stream;

		p1.SetBlockHeight(rand());
		p1.SetGenesisBlockAddress(getRandString(100));
		std::vector<uint256> hashes;
		for (size_t i = 0; i < 20; ++i) {
			hashes.push_back(getRanduint256());
		}
		p1.SetSideChainTransacitonHash(hashes);
		p1.Serialize(stream, 0);

		REQUIRE(p2.Deserialize(stream, 0));

		REQUIRE(p1.GetBlockHeight() == p2.GetBlockHeight());
		REQUIRE(p1.GetGenesisBlockAddress() == p2.GetGenesisBlockAddress());
		std::vector<uint256> pbHashes = p2.GetSideChainTransacitonHash();
		REQUIRE(hashes.size() == pbHashes.size());
		for (size_t i = 0; i < pbHashes.size(); ++i) {
			REQUIRE(hashes[i] == pbHashes[i]);
		}
	}

	SECTION("to json and from json") {
		WithdrawFromSideChain p1, p2;

		p1.SetBlockHeight(rand());
		p1.SetGenesisBlockAddress(getRandString(100));
		std::vector<uint256> hashes;
		for (size_t i = 0; i < 20; ++i) {
			hashes.push_back(getRanduint256());
		}
		p1.SetSideChainTransacitonHash(hashes);

		nlohmann::json j = p1.ToJson(0);

		p2.FromJson(j, 0);

		REQUIRE(p1.GetBlockHeight() == p2.GetBlockHeight());
		REQUIRE(p1.GetGenesisBlockAddress() == p2.GetGenesisBlockAddress());
		const std::vector<uint256> &pbHashes = p2.GetSideChainTransacitonHash();
		REQUIRE(hashes.size() == pbHashes.size());
		for (size_t i = 0; i < pbHashes.size(); ++i) {
			REQUIRE(hashes[i] == pbHashes[i]);
		}
	}

}