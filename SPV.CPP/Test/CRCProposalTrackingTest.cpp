// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Payload/CRCProposalTracking.h>

using namespace Elastos::ElaWallet;

static void initCRCProposalTracking(CRCProposalTracking &tracking) {
	CRCProposalTracking::CRCProposalTrackingType type = CRCProposalTracking::CRCProposalTrackingType(getRandUInt8() % 6);
	tracking.SetType(type);
	tracking.SetProposalHash(getRanduint256());
	tracking.SetDocumentHash(getRanduint256());
	tracking.SetStage(getRandUInt8());
	tracking.SetAppropriation(getRandUInt64());
	tracking.SetLeaderPubKey(getRandBytes(33));
	tracking.SetNewLeaderPubKey(getRandBytes(33));
	tracking.SetLeaderSign(getRandBytes(64));
	tracking.SetNewLeaderSign(getRandBytes(64));
	tracking.SetSecretaryGeneralSign(getRandBytes(64));
}

TEST_CASE("CRCProposalTracking test", "[CRCProposalTracking]") {
	SECTION("Serialize and Deserialize test") {
		CRCProposalTracking tracking1;
		initCRCProposalTracking(tracking1);

		ByteStream byteStream;
		tracking1.Serialize(byteStream, 0);

		REQUIRE(byteStream.GetBytes().size() == tracking1.EstimateSize(0));

		CRCProposalTracking tracking2;
		REQUIRE(tracking2.Deserialize(byteStream, 0) == true);

		REQUIRE(tracking1.GetType() == tracking2.GetType());
		REQUIRE(tracking1.GetProposalHash() == tracking2.GetProposalHash());
		REQUIRE(tracking1.GetDocumentHash() == tracking2.GetDocumentHash());
		REQUIRE(tracking1.GetStage() == tracking2.GetStage());
		REQUIRE(tracking1.GetAppropriation() == tracking2.GetAppropriation());
		REQUIRE(tracking1.GetLeaderPubKey() == tracking2.GetLeaderPubKey());
		REQUIRE(tracking1.GetNewLeaderPubKey() == tracking2.GetNewLeaderPubKey());
		REQUIRE(tracking1.GetLeaderSign() == tracking2.GetLeaderSign());
		REQUIRE(tracking1.GetNewLeaderSign() == tracking2.GetNewLeaderSign());
		REQUIRE(tracking1.GetSecretaryGeneralSign() == tracking2.GetSecretaryGeneralSign());
	}

	SECTION("ToJson FromJson test") {
		CRCProposalTracking tracking1;
		initCRCProposalTracking(tracking1);

		nlohmann::json j = tracking1.ToJson(0);
		REQUIRE(j.empty() == false);

		CRCProposalTracking tracking2;
		tracking2.FromJson(j, 0);

		REQUIRE(tracking1.GetType() == tracking2.GetType());
		REQUIRE(tracking1.GetProposalHash() == tracking2.GetProposalHash());
		REQUIRE(tracking1.GetDocumentHash() == tracking2.GetDocumentHash());
		REQUIRE(tracking1.GetStage() == tracking2.GetStage());
		REQUIRE(tracking1.GetAppropriation() == tracking2.GetAppropriation());
		REQUIRE(tracking1.GetLeaderPubKey() == tracking2.GetLeaderPubKey());
		REQUIRE(tracking1.GetNewLeaderPubKey() == tracking2.GetNewLeaderPubKey());
		REQUIRE(tracking1.GetLeaderSign() == tracking2.GetLeaderSign());
		REQUIRE(tracking1.GetNewLeaderSign() == tracking2.GetNewLeaderSign());
		REQUIRE(tracking1.GetSecretaryGeneralSign() == tracking2.GetSecretaryGeneralSign());
	}

}

