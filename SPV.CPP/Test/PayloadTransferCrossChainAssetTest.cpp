// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"
#include <Core/BRInt.h>
#include <SDK/Plugin/Transaction/Payload/PayloadTransferCrossChainAsset.h>

using namespace Elastos::ElaWallet;

static void verifyPayload(const PayloadTransferCrossChainAsset &p1, const PayloadTransferCrossChainAsset &p2) {
	const std::vector<std::string> &addresses1 = p1.getCrossChainAddress();
	const std::vector<std::string> &addresses2 = p2.getCrossChainAddress();
	REQUIRE(addresses1.size() == addresses2.size());
	for (size_t i = 0; i < addresses1.size(); ++i) {
		REQUIRE(addresses1[i] == addresses2[i]);
	}

	const std::vector<uint64_t> &indexes1 = p1.getOutputIndex();
	const std::vector<uint64_t> &indexes2 = p2.getOutputIndex();
	REQUIRE(indexes1.size() == indexes2.size());
	for (size_t i = 0; i < indexes1.size(); ++i) {
		REQUIRE(indexes1[i] == indexes2[i]);
	}

	const std::vector<uint64_t> &amounts1 = p1.getCrossChainAmout();
	const std::vector<uint64_t> &amounts2 = p2.getCrossChainAmout();
	REQUIRE(amounts1.size() == amounts2.size());
	for (size_t i = 0; i < amounts1.size(); ++i) {
		REQUIRE(amounts1[i] == amounts2[i]);
	}
}

TEST_CASE("PayloadTransferCrossChainAsset Test", "[PayloadTransferCrossChainAsset]") {

	SECTION("Default construct test") {
		PayloadTransferCrossChainAsset p1, p2;

		ByteStream stream1, stream2;
		p1.Serialize(stream1);
		p2.Serialize(stream2);

		CMBlock buffer1 = stream1.getBuffer();
		CMBlock buffer2 = stream2.getBuffer();

		REQUIRE((buffer1 == buffer2));

		verifyPayload(p1, p2);
	}

	SECTION("Serialize and deserialize test") {
		PayloadTransferCrossChainAsset p1, p2;

		std::vector<std::string> crossChainAddress;
		std::vector<uint64_t> crossChainIndex;
		std::vector<uint64_t> crossChainAmount;
		for (int i = 0; i < 10; ++i) {
			crossChainAddress.push_back(getRandString(34));
			crossChainIndex.push_back(getRandUInt64());
			crossChainAmount.push_back(getRandUInt64());
		}
		p1.setCrossChainData(crossChainAddress, crossChainIndex, crossChainAmount);

		ByteStream stream;
		p1.Serialize(stream);
		CMBlock data1 = stream.getBuffer();

		stream.setPosition(0);
		p2.Deserialize(stream);
		CMBlock data2 = p2.getData();

		REQUIRE((data1 == data2));

		verifyPayload(p1, p2);
	}

	SECTION("to json and from json") {
		PayloadTransferCrossChainAsset p1, p2;

		std::vector<std::string> crossChainAddress;
		std::vector<uint64_t> crossChainIndex;
		std::vector<uint64_t> crossChainAmount;
		for (int i = 0; i < 10; ++i) {
			crossChainAddress.push_back(getRandString(34));
			crossChainIndex.push_back(getRandUInt64());
			crossChainAmount.push_back(getRandUInt64());
		}

		p1.setCrossChainData(crossChainAddress, crossChainIndex, crossChainAmount);

		nlohmann::json jsonData = p1.toJson();

		p2.fromJson(jsonData);

		verifyPayload(p1, p2);
	}
}
