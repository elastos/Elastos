// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "Utils.h"
#include "Key.h"

using namespace Elastos::SDK;

TEST_CASE("ProgramHash and AddressHash Test", "") {
	UInt168 targetHash = Utils::UInt168FromString("213a3b4511636bf45a582a02b2ee0a0d3c9c52dfe1");
	std::string publicKeyHex = "022c9652d3ad5cc065aa9147dc2ad022f80001e8ed233de20f352950d351d472b7";
	size_t pubLen = 0;
	uint8_t *publicKey = Utils::decodeHexCreate(&pubLen, (char *)publicKeyHex.c_str(), publicKeyHex.size());
	BRKey *brKey = new BRKey;
	memset(brKey, 0, sizeof(BRKey));
	brKey->compressed = 1;
	memcpy(brKey->pubKey, publicKey, pubLen);

	Key key(brKey);
	std::string redeedStr = key.keyToRedeemScript(ELA_STANDARD);
	size_t scriptLen = 0;
	uint8_t *script = Utils::decodeHexCreate(&scriptLen, (char *)(redeedStr.c_str()), redeedStr.size());
	CMBlock redeedScript(scriptLen);
	memcpy(redeedScript, script, scriptLen);

	UInt168 hash = Utils::codeToProgramHash(redeedScript);
	int r = UInt168Eq(&hash, &targetHash);
	REQUIRE(r == 1);

	std::string addr = Utils::UInt168ToAddress(hash);

	UInt168 addrHash = UINT168_ZERO;
	REQUIRE(true == Utils::UInt168FromAddress(addrHash, addr));
	r = UInt168Eq(&addrHash, &targetHash);
	REQUIRE(r == 1);
}
