// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"

#include <SDK/Plugin/Transaction/Payload/PayloadRechargeToSideChain.h>

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadRechargeToSideChain test", "[PayloadRechargeToSideChain]") {
	CMBlock merkleProof = getRandCMBlock(50);
	CMBlock mainChainTx = getRandCMBlock(100);

	PayloadRechargeToSideChain p1(merkleProof, mainChainTx);
	ByteStream stream;
	p1.Serialize(stream, 0);
	stream.setPosition(0);
	PayloadRechargeToSideChain p2;
	REQUIRE(p2.Deserialize(stream, 0));

	CMBlock data1 = p1.getData(0);
	CMBlock data2 = p2.getData(0);

	REQUIRE(data1.GetSize() == data2.GetSize());
	REQUIRE(0 == memcmp(data1, data2, data1.GetSize()));
}