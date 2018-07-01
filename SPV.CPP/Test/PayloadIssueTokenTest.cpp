// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "Payload/PayloadIssueToken.h"
#include "TestHelper.h"

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadIssueToken test", "[PayloadIssueToken]") {
	CMBlock merkleProof = getRandCMBlock(50);
	CMBlock mainChainTx = getRandCMBlock(100);

	PayloadIssueToken issueToken(merkleProof, mainChainTx);
	ByteStream stream;
	issueToken.Serialize(stream);
	stream.setPosition(0);
	PayloadIssueToken issueToken1;
	REQUIRE(issueToken1.Deserialize(stream));

	CMBlock data = issueToken.getData();
	CMBlock data1 = issueToken1.getData();

	REQUIRE(data.GetSize() == data1.GetSize());
	REQUIRE(0 == memcmp(data, data1, data.GetSize()));
}