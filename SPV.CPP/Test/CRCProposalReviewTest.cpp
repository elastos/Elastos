// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Payload/CRCProposalReview.h>

using namespace Elastos::ElaWallet;

static void initCRCProposalReview(CRCProposalReview &review) {
	review.SetProposalHash(getRanduint256());
	uint8_t result = getRandUInt8() % 3;
	review.SetVoteResult(CRCProposalReview::VoteResult(result));
	review.SetOpinionHash(getRanduint256());
	review.SetDID(Address("icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY"));
	review.SetSignature(getRandBytes(90));
}

static void verifyProposalReview(CRCProposalReview &p1, CRCProposalReview &p2) {
	REQUIRE(p2.GetProposalHash() == p1.GetProposalHash());
	REQUIRE(p2.GetVoteResult() == p1.GetVoteResult());
	REQUIRE(p2.GetOpinionHash() == p1.GetOpinionHash());
	REQUIRE(p2.GetDID() == p1.GetDID());
	REQUIRE(p2.GetSignature() == p1.GetSignature());
}

TEST_CASE("CRCProposalReview test", "[CRCProposalReview]") {
	SECTION("Serialize and Deserialize test") {
		CRCProposalReview p1;
		initCRCProposalReview(p1);

		ByteStream byteStream;
		p1.Serialize(byteStream, 0);

		REQUIRE(byteStream.GetBytes().size() == p1.EstimateSize(0));

		CRCProposalReview p2;
		REQUIRE(p2.Deserialize(byteStream, 0));

		verifyProposalReview(p1, p2);
	}

	SECTION("ToJson FromJson test") {
		CRCProposalReview p1;
		initCRCProposalReview(p1);

		nlohmann::json j = p1.ToJson(0);

		CRCProposalReview p2;
		p2.FromJson(j, 0);

		verifyProposalReview(p1, p2);
	}
}