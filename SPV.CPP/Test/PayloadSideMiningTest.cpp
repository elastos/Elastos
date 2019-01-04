// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"
//#include "BRInt.h"
#include <SDK/Plugin/Transaction/Payload/PayloadSideMining.h>

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadSideMining Test", "[PayloadSideMining]") {

	SECTION("Serialize and deserialize") {
		PayloadSideMining p1, p2;

		p1.SetSideBlockHash(getRandUInt256());
		p1.SetSideGenesisHash(getRandUInt256());
		p1.SetBlockHeight(getRandUInt32());
		p1.SetSignedData(getRandCMBlock(100));

		ByteStream stream;
		p1.Serialize(stream, 0);

		stream.setPosition(0);
		REQUIRE(p2.Deserialize(stream, 0));

		REQUIRE(UInt256Eq(&p1.GetSideBlockHash(), &p2.GetSideBlockHash()));
		REQUIRE(UInt256Eq(&p1.GetSideGenesisHash(), &p2.GetSideGenesisHash()));
		REQUIRE(p1.GetBlockHeight() == p2.GetBlockHeight());
		REQUIRE((p1.GetSignedData() == p2.GetSignedData()));
	}

	SECTION("to json and from json") {
		PayloadSideMining p1, p2;

		p1.SetSideBlockHash(getRandUInt256());
		p1.SetSideGenesisHash(getRandUInt256());
		p1.SetBlockHeight(getRandUInt32());
		p1.SetSignedData(getRandCMBlock(100));

		nlohmann::json p1Json = p1.toJson();

		p2.fromJson(p1Json);

		REQUIRE(UInt256Eq(&p1.GetSideBlockHash(), &p2.GetSideBlockHash()));
		REQUIRE(UInt256Eq(&p1.GetSideGenesisHash(), &p2.GetSideGenesisHash()));
		REQUIRE(p1.GetBlockHeight() == p2.GetBlockHeight());
		REQUIRE((p1.GetSignedData() == p2.GetSignedData()));
	}

}
