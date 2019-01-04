// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"
#include <SDK/Common/Log.h>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadVote Test", "[PayloadVote]") {

	SECTION("Serialize and deserialize") {
		std::vector<CMBlock> candidates;
		for (size_t i = 0; i < 10; ++i) {
			candidates.push_back(getRandCMBlock(33));
		}

		PayloadVote p1(PayloadVote::Type::Delegate, candidates), p2;

		ByteStream stream;
		p1.Serialize(stream, 0);

		stream.setPosition(0);
		REQUIRE(p2.Deserialize(stream, 0));

		REQUIRE(p1.GetVoteType() == p2.GetVoteType());
		REQUIRE(p1.GetCandidates().size() == p2.GetCandidates().size());
		for (size_t i = 0; i < p1.GetCandidates().size(); ++i) {
			REQUIRE((p1.GetCandidates()[i] == p2.GetCandidates()[i]));
		}
	}

	SECTION("to json and from json") {
		std::vector<CMBlock> candidates;
		for (size_t i = 0; i < 10; ++i) {
			candidates.push_back(getRandCMBlock(33));
		}

		PayloadVote p1(PayloadVote::Type::CRC, candidates), p2;

		nlohmann::json p1Json = p1.toJson();
		REQUIRE_NOTHROW(p2.fromJson(p1Json));

		REQUIRE(p1.GetVoteType() == p2.GetVoteType());
		REQUIRE(p1.GetCandidates().size() == p2.GetCandidates().size());
		for (size_t i = 0; i < p1.GetCandidates().size(); ++i) {
			REQUIRE((p1.GetCandidates()[i] == p2.GetCandidates()[i]));
		}
	}

	SECTION("operator= test") {
		std::vector<CMBlock> candidates;
		for (size_t i = 0; i < 10; ++i) {
			candidates.push_back(getRandCMBlock(33));
		}

		PayloadVote p1(PayloadVote::Type::Delegate, candidates), p2;

		p2 = p1;

		REQUIRE(p1.GetVoteType() == p2.GetVoteType());
		REQUIRE(p1.GetCandidates().size() == p2.GetCandidates().size());
		for (size_t i = 0; i < p1.GetCandidates().size(); ++i) {
			REQUIRE((p1.GetCandidates()[i] == p2.GetCandidates()[i]));
		}

		PayloadPtr ptr1(new PayloadVote(PayloadVote::Type::CRC, candidates));
		PayloadPtr ptr2(new PayloadVote());

		*ptr2 = *ptr1;

		PayloadVote *pv1 = dynamic_cast<PayloadVote *>(ptr1.get());
		PayloadVote *pv2 = dynamic_cast<PayloadVote *>(ptr2.get());

		REQUIRE(pv1->GetVoteType() == pv2->GetVoteType());
		REQUIRE(pv1->GetCandidates().size() == pv2->GetCandidates().size());
		for (size_t i = 0; i < pv1->GetCandidates().size(); ++i) {
			REQUIRE((pv1->GetCandidates()[i] == pv2->GetCandidates()[i]));
		}

	}

	SECTION("copy construct test") {
		std::vector<CMBlock> candidates;
		for (size_t i = 0; i < 10; ++i) {
			candidates.push_back(getRandCMBlock(33));
		}

		PayloadVote p1(PayloadVote::Type::Delegate, candidates);

		PayloadVote p2(p1);

		REQUIRE(p1.GetVoteType() == p2.GetVoteType());
		REQUIRE(p1.GetCandidates().size() == p2.GetCandidates().size());
		for (size_t i = 0; i < p1.GetCandidates().size(); ++i) {
			REQUIRE((p1.GetCandidates()[i] == p2.GetCandidates()[i]));
		}
	}

}
