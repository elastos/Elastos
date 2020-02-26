// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Payload/CRInfo.h>

using namespace Elastos::ElaWallet;

TEST_CASE("CRInfo test", "[CRInfo]") {

	SECTION("Serialize and deserialize") {
		CRInfo crInfo1;
		crInfo1.SetCode(getRandBytes(34));
		crInfo1.SetCID(uint168(PrefixIDChain, getRandBytes(20)));
		crInfo1.SetDID(uint168(PrefixIDChain, getRandBytes(20)));
		crInfo1.SetNickName(getRandString(10));
		crInfo1.SetLocation(getRandUInt64());
		crInfo1.SetUrl(getRandString(13));
		crInfo1.SetSignature(getRandBytes(36));

		ByteStream stream;

		crInfo1.Serialize(stream, CRInfoDIDVersion);

		CRInfo crInfo2;
		REQUIRE(crInfo2.Deserialize(stream, CRInfoDIDVersion));
		REQUIRE(crInfo2.GetCode() == crInfo1.GetCode());
		REQUIRE(crInfo2.GetCID() == crInfo1.GetCID());
		REQUIRE(crInfo2.GetDID() == crInfo1.GetDID());
		REQUIRE(crInfo2.GetNickName() == crInfo1.GetNickName());
		REQUIRE(crInfo2.GetUrl() == crInfo1.GetUrl());
		REQUIRE(crInfo2.GetSignature() == crInfo1.GetSignature());
	}

	SECTION("to json and from json") {
		CRInfo crInfo1;
		;
		crInfo1.SetCode(getRandBytes(34));
		crInfo1.SetDID(uint168(PrefixIDChain, getRandBytes(20)));
		crInfo1.SetCID(uint168(PrefixIDChain, getRandBytes(20)));
		crInfo1.SetNickName(getRandString(10));
		crInfo1.SetLocation(getRandUInt64());
		crInfo1.SetUrl(getRandString(13));
		crInfo1.SetSignature(getRandBytes(36));

		CRInfo crInfo2;
		crInfo2.FromJson(crInfo1.ToJson(0), 0);

		REQUIRE(crInfo2.GetCode() == crInfo1.GetCode());
		REQUIRE(crInfo2.GetCID() == crInfo1.GetCID());
		REQUIRE(crInfo2.GetDID() == crInfo1.GetDID());
		REQUIRE(crInfo2.GetNickName() == crInfo1.GetNickName());
		REQUIRE(crInfo2.GetUrl() == crInfo1.GetUrl());
		REQUIRE(crInfo2.GetSignature() == crInfo1.GetSignature());
	}
}