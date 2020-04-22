// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Payload/CRCProposalTracking.h>

using namespace Elastos::ElaWallet;

static void initCRCProposalTracking(CRCProposalTracking &tracking) {
	CRCProposalTracking::Type type = CRCProposalTracking::Type(getRandUInt8() % 6);
	tracking.SetProposalHash(getRanduint256());
	tracking.SetMessageHash(getRanduint256());
	tracking.SetStage(getRandUInt8());
	tracking.SetOwnerPubKey(getRandBytes(33));
	tracking.SetNewOwnerPubKey(getRandBytes(33));
	tracking.SetOwnerSign(getRandBytes(64));
	tracking.SetNewOwnerSign(getRandBytes(64));
	tracking.SetType(type);
	tracking.SetSecretaryGeneralOpinionHash(getRanduint256());
	tracking.SetSecretaryGeneralSignature(getRandBytes(64));
}

static void verifyProposalTracking(CRCProposalTracking &p1, CRCProposalTracking &p2) {
	REQUIRE(p1.GetProposalHash() == p2.GetProposalHash());
	REQUIRE(p1.GetMessageHash() == p2.GetMessageHash());
	REQUIRE(p1.GetStage() == p2.GetStage());
	REQUIRE(p1.GetOwnerPubKey() == p2.GetOwnerPubKey());
	REQUIRE(p1.GetNewOwnerPubKey() == p2.GetNewOwnerPubKey());
	REQUIRE(p1.GetOwnerSign() == p2.GetOwnerSign());
	REQUIRE(p1.GetNewOwnerSign() == p2.GetNewOwnerSign());
	REQUIRE(p1.GetType() == p2.GetType());
	REQUIRE(p1.GetSecretaryGeneralOpinionHash() == p2.GetSecretaryGeneralOpinionHash());
	REQUIRE(p1.GetSecretaryGeneralSignature() == p2.GetSecretaryGeneralSignature());
}

TEST_CASE("CRCProposalTracking test", "[CRCProposalTracking]") {
	SECTION("Serialize and Deserialize test") {
		CRCProposalTracking p1;
		initCRCProposalTracking(p1);

		ByteStream byteStream;
		p1.Serialize(byteStream, 0);

		REQUIRE(byteStream.GetBytes().size() == p1.EstimateSize(0));

		CRCProposalTracking p2;
		REQUIRE(p2.Deserialize(byteStream, 0));

		verifyProposalTracking(p1, p2);
	}

	SECTION("ToJson FromJson test") {
		CRCProposalTracking p1;
		initCRCProposalTracking(p1);

		nlohmann::json j = p1.ToJson(0);

		CRCProposalTracking p2;
		p2.FromJson(j, 0);

		verifyProposalTracking(p1, p2);
	}

}

