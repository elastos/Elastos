// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "Payload/PayloadIssueToken.h"

using namespace Elastos::SDK;

TEST_CASE("PayloadIssueToken test", "[PayloadIssueToken]") {
	uint8_t script[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24, 60, 75, 8, 182, 57, 98};
	ByteData data(script, sizeof(script));

	PayloadIssueToken issueToken(data);

	ByteData data1 = issueToken.getData();
	size_t diff = data1.length - data.length;
	REQUIRE(diff > 0);

	int result = memcmp(data.data, &data1.data[diff], data.length);
	REQUIRE(result == 0);

	ByteStream byteStream;
	issueToken.Serialize(byteStream);
	REQUIRE(byteStream.length() > 0);
	byteStream.setPosition(0);

	PayloadIssueToken issueToken1;
	issueToken1.Deserialize(byteStream);

	ByteData data2 = issueToken1.getData();
	REQUIRE(data2.length == data1.length);
	result = memcmp(data2.data, data1.data, data1.length);
	REQUIRE(result == 0);
}