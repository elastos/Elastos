// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"
#include <Plugin/Transaction/Payload/SideChainPow.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("SideChainPow Test", "[SideChainPow]") {
	Log::registerMultiLogger();

	SECTION("Serialize and deserialize") {
		SideChainPow p1, p2;

		p1.SetSideBlockHash(getRanduint256());
		p1.SetSideGenesisHash(getRanduint256());
		p1.SetBlockHeight(getRandUInt32());
		p1.SetSignedData(getRandBytes(100));

		ByteStream stream;
		p1.Serialize(stream, 0);

		REQUIRE(p2.Deserialize(stream, 0));

		REQUIRE(p1.GetSideBlockHash() == p2.GetSideBlockHash());
		REQUIRE(p1.GetSideGenesisHash() == p2.GetSideGenesisHash());
		REQUIRE(p1.GetBlockHeight() == p2.GetBlockHeight());
		REQUIRE((p1.GetSignedData() == p2.GetSignedData()));
	}

	SECTION("to json and from json") {
		SideChainPow p1, p2;

		p1.SetSideBlockHash(getRanduint256());
		p1.SetSideGenesisHash(getRanduint256());
		p1.SetBlockHeight(getRandUInt32());
		p1.SetSignedData(getRandBytes(100));

		nlohmann::json p1Json = p1.ToJson(0);

		p2.FromJson(p1Json, 0);

		REQUIRE(p1.GetSideBlockHash() == p2.GetSideBlockHash());
		REQUIRE(p1.GetSideGenesisHash() == p2.GetSideGenesisHash());
		REQUIRE(p1.GetBlockHeight() == p2.GetBlockHeight());
		REQUIRE((p1.GetSignedData() == p2.GetSignedData()));
	}

}
