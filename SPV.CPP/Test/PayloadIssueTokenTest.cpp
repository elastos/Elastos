// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Payload/RechargeToSideChain.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("RechargeToSideChain test", "[RechargeToSideChain]") {
	Log::registerMultiLogger();
	bytes_t merkleProof = getRandBytes(50);
	bytes_t mainChainTx = getRandBytes(100);

	RechargeToSideChain p1(merkleProof, mainChainTx);
	ByteStream stream;
	p1.Serialize(stream, 0);
	RechargeToSideChain p2;
	REQUIRE(p2.Deserialize(stream, 0));

	bytes_t data1 = p1.GetData(0);
	bytes_t data2 = p2.GetData(0);

	REQUIRE(data1.size() == data2.size());
	REQUIRE(data1 == data2);
}