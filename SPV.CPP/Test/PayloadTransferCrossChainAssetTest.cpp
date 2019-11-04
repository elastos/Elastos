// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"
#include <Plugin/Transaction/Payload/TransferCrossChainAsset.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

static void verifyPayload(const TransferCrossChainAsset &p1, const TransferCrossChainAsset &p2) {
	const std::vector<TransferInfo> &info1 = p1.Info();
	const std::vector<TransferInfo> &info2 = p2.Info();

	REQUIRE(info1.size() == info2.size());
	for (size_t i = 0; i < info1.size(); ++i) {
		REQUIRE(info1[i].CrossChainAddress() == info2[i].CrossChainAddress());
		REQUIRE(info1[i].OutputIndex() == info2[i].OutputIndex());
		REQUIRE(info1[i].CrossChainAmount() == info2[i].CrossChainAmount());
	}
}

TEST_CASE("TransferCrossChainAsset Test", "[TransferCrossChainAsset]") {
	Log::registerMultiLogger();

	SECTION("Default construct test") {
		TransferCrossChainAsset p1, p2;

		ByteStream stream1, stream2;
		p1.Serialize(stream1, 0);
		p2.Serialize(stream2, 0);

		bytes_t buffer1 = stream1.GetBytes();
		bytes_t buffer2 = stream2.GetBytes();

		REQUIRE((buffer1 == buffer2));

		verifyPayload(p1, p2);
	}

	SECTION("Serialize and deserialize test") {
		std::vector<TransferInfo> infos;
		for (int i = 0; i < 10; ++i) {
			std::string addr = getRandString(34);
			uint16_t index = getRandUInt16();
			BigInt amount;
			amount.setUint64(getRandUInt64());

			TransferInfo info(addr, index, amount);
			infos.push_back(info);
		}
		TransferCrossChainAsset p1(infos), p2;

		ByteStream stream;
		p1.Serialize(stream, 0);
		bytes_t data1 = stream.GetBytes();

		p2.Deserialize(stream, 0);
		bytes_t data2 = p2.GetData(0);

		REQUIRE((data1 == data2));

		verifyPayload(p1, p2);
	}

	SECTION("to json and from json") {

		std::vector<std::string> crossChainAddress;
		std::vector<uint64_t> crossChainIndex;
		std::vector<uint64_t> crossChainAmount;
		std::vector<TransferInfo> infos;
		for (int i = 0; i < 10; ++i) {
			std::string addr = getRandString(34);
			uint16_t index = getRandUInt16();
			BigInt amount;
			amount.setUint64(getRandUInt64());

			TransferInfo info(addr, index, amount);
			infos.push_back(info);
		}
		TransferCrossChainAsset p1(infos), p2;

		nlohmann::json jsonData = p1.ToJson(0);

		p2.FromJson(jsonData, 0);

		verifyPayload(p1, p2);
	}
}
