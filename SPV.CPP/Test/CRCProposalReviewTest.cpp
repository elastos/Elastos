// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"

#include <SDK/Plugin/Transaction/Payload/CRCProposalReview.h>

using namespace Elastos::ElaWallet;

static void initCRCProposalView(CRCProposalReview &review) {
	review.SetProposalHash(getRanduint256());
	uint8_t result = getRandUInt8() % 3;
	review.SetResult(CRCProposalReview::VoteResult (result));

	review.SetCRDID(getRandUInt168());

	review.SetSignature(getRandBytes(90));
}

TEST_CASE("CRCProposalReview test", "[CRCProposalReview]") {
	SECTION("Serialize and Deserialize test") {
		CRCProposalReview review1;
		initCRCProposalView(review1);

		ByteStream byteStream;
		review1.Serialize(byteStream, 0);

		REQUIRE(byteStream.GetBytes().size() == review1.EstimateSize(0));

		CRCProposalReview review2;
		REQUIRE(review2.Deserialize(byteStream, 0) == true);

		REQUIRE(review2.GetProposalHash() == review1.GetProposalHash());
		REQUIRE(review2.GetResult() == review1.GetResult());
		REQUIRE(review2.GetCRDID() == review1.GetCRDID());
		REQUIRE(review2.GetSignature() == review1.GetSignature());
	}

	SECTION("ToJson FromJson test") {
		CRCProposalReview review1;
		initCRCProposalView(review1);

		nlohmann::json j = review1.ToJson(0);
		REQUIRE(j.empty() == false);

		CRCProposalReview review2;
		review2.FromJson(j, 0);

		REQUIRE(review2.GetProposalHash() == review1.GetProposalHash());
		REQUIRE(review2.GetResult() == review1.GetResult());
		REQUIRE(review2.GetCRDID() == review1.GetCRDID());
		REQUIRE(review2.GetSignature() == review1.GetSignature());
	}
}