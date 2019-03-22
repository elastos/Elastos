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
		std::vector<PayloadVote::VoteContent> voteContent;
		for (size_t i = 0; i < 10; ++i) {
			std::vector<bytes_t> candidates;
			for (size_t c = 0; c < 10; c++) {
				candidates.push_back(getRandBytes(33));
			}
			voteContent.emplace_back(PayloadVote::Type(i % PayloadVote::Type::Max), candidates);
		}
		PayloadVote p1(voteContent), p2;

		ByteStream stream;
		p1.Serialize(stream);

		REQUIRE(p2.Deserialize(stream));

		const std::vector<PayloadVote::VoteContent> &vc1 = p1.GetVoteContent();
		const std::vector<PayloadVote::VoteContent> &vc2 = p2.GetVoteContent();
		REQUIRE(vc1.size() == vc2.size());
		for (size_t i = 0; i < vc1.size(); ++i) {
			REQUIRE(vc1[i].type == vc2[i].type);
			REQUIRE((vc1[i].candidates.size() == vc2[i].candidates.size()));
			for (size_t c = 0; c < vc1[i].candidates.size(); c++) {
				REQUIRE((vc1[i].candidates[c] == vc2[i].candidates[c]));
			}
		}
	}

	SECTION("to json and from json") {
		std::vector<PayloadVote::VoteContent> voteContent;
		for (size_t i = 0; i < 10; ++i) {
			std::vector<bytes_t> candidates;
			for (size_t c = 0; c < 10; c++) {
				candidates.push_back(getRandBytes(33));
			}
			voteContent.emplace_back(PayloadVote::Type(i % PayloadVote::Type::Max), candidates);
		}
		PayloadVote p1(voteContent), p2;

		nlohmann::json p1Json = p1.ToJson();
		REQUIRE_NOTHROW(p2.FromJson(p1Json));

		const std::vector<PayloadVote::VoteContent> &vc1 = p1.GetVoteContent();
		const std::vector<PayloadVote::VoteContent> &vc2 = p2.GetVoteContent();
		REQUIRE(vc1.size() == vc2.size());
		for (size_t i = 0; i < vc1.size(); ++i) {
			REQUIRE(vc1[i].type == vc2[i].type);
			REQUIRE((vc1[i].candidates.size() == vc2[i].candidates.size()));
			for (size_t c = 0; c < vc1[i].candidates.size(); c++) {
				REQUIRE((vc1[i].candidates[c] == vc2[i].candidates[c]));
			}
		}
	}

	SECTION("operator= test") {
		std::vector<PayloadVote::VoteContent> voteContent;
		for (size_t i = 0; i < 10; ++i) {
			std::vector<bytes_t> candidates;
			for (size_t c = 0; c < 10; c++) {
				candidates.push_back(getRandBytes(33));
			}
			voteContent.emplace_back(PayloadVote::Type(i % PayloadVote::Type::Max), candidates);
		}
		PayloadVote p1(voteContent), p2;

		p2 = p1;

		const std::vector<PayloadVote::VoteContent> &vc1 = p1.GetVoteContent();
		const std::vector<PayloadVote::VoteContent> &vc2 = p2.GetVoteContent();
		REQUIRE(vc1.size() == vc2.size());
		for (size_t i = 0; i < vc1.size(); ++i) {
			REQUIRE(vc1[i].type == vc2[i].type);
			REQUIRE((vc1[i].candidates.size() == vc2[i].candidates.size()));
			for (size_t c = 0; c < vc1[i].candidates.size(); c++) {
				REQUIRE((vc1[i].candidates[c] == vc2[i].candidates[c]));
			}
		}
	}

	SECTION("copy construct test") {
		std::vector<PayloadVote::VoteContent> voteContent;
		for (size_t i = 0; i < 10; ++i) {
			std::vector<bytes_t> candidates;
			for (size_t c = 0; c < 10; c++) {
				candidates.push_back(getRandBytes(33));
			}
			voteContent.emplace_back(PayloadVote::Type(i % PayloadVote::Type::Max), candidates);
		}
		PayloadVote p1(voteContent);

		PayloadVote p2(p1);

		const std::vector<PayloadVote::VoteContent> &vc1 = p1.GetVoteContent();
		const std::vector<PayloadVote::VoteContent> &vc2 = p2.GetVoteContent();
		REQUIRE(vc1.size() == vc2.size());
		for (size_t i = 0; i < vc1.size(); ++i) {
			REQUIRE(vc1[i].type == vc2[i].type);
			REQUIRE((vc1[i].candidates.size() == vc2[i].candidates.size()));
			for (size_t c = 0; c < vc1[i].candidates.size(); c++) {
				REQUIRE((vc1[i].candidates[c] == vc2[i].candidates[c]));
			}
		}
	}

}
