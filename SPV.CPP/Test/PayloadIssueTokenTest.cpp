// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "Payload/PayloadIssueToken.h"

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadIssueToken test", "[PayloadIssueToken]") {
	uint8_t script[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24, 60, 75, 8, 182, 57, 98};
	CMBlock data;
	data.SetMemFixed(script, sizeof(script));

	PayloadIssueToken issueToken(data);

	CMBlock data1 = issueToken.getData();
	size_t diff = data1.GetSize() - data.GetSize();
	REQUIRE(diff > 0);

	int result = memcmp(data, &data1[diff], data.GetSize());
	REQUIRE(result == 0);

	ByteStream byteStream;
	issueToken.Serialize(byteStream);
	REQUIRE(byteStream.length() > 0);
	byteStream.setPosition(0);

	PayloadIssueToken issueToken1;
	issueToken1.Deserialize(byteStream);

	CMBlock data2 = issueToken1.getData();
	REQUIRE(data2.GetSize() == data1.GetSize());
	result = memcmp(data2, data1, data1.GetSize());
	REQUIRE(result == 0);
}